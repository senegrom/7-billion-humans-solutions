/*
 * emu.c -- a small simulator for 7 Billion Humans solutions.
 *
 * Goal: run a solution program against a level and report whether it wins, how
 * many commands it uses (SIZE), and how many lockstep rounds it takes (raw
 * step count -- see the caveat below). This lets us machine-check candidate
 * solutions instead of clicking through the game.
 *
 * Companion tool for owners of 7 Billion Humans: it reads none of the game's
 * files, but refuses to run unless the game is installed (see game_installed).
 *
 * Level files: two formats.
 *   v1 "row" grids -- small hand-authored regression levels (see levels/).
 *   v2 "ent" lists -- one entity per line, plus per-level metadata (command
 *      palette, random-value spec, special rules, win predicate). v2 files are
 *      generated locally by owners from their own copy of the game; none ship
 *      with this repo.
 *
 * Faithfulness: movement, item handling, sensing, multi-condition ifs, and the
 * walk-swap rule follow the game's observable behavior as documented by the
 * community (levels' goal descriptions, tips, and known-good solutions).
 * Commands whose semantics we haven't verified (calc, write, set, nearest,
 * tell, listen, foreachdir) parse but refuse to run rather than mis-simulate.
 * Randomized levels are checked over many seeded trials.
 *
 * IMPORTANT CAVEAT ON "SPEED": the round count this prints is NOT the game's
 * Speed score. The game's speed metric is not "lines executed" (e.g. Year 2
 * scores speed 1, Year 46 scores speed 0) and must be calibrated against the
 * real game. We print raw rounds only as a coarse progress signal.
 *
 * Build:  gcc -std=c11 -O2 -Wall -Wextra -o emu emu.c
 * Run:    ./emu <level.lvl> <solution.txt> [trials]
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static bool g_goal_dbg = false;

/* ------------------------------------------------------------------ model -- */

enum { MAXW = 64, MAXH = 64, MAXWORKERS = 128, MAXPROG = 4096, MAXLABELS = 256,
       MAXCUBES = 512, MAXDEV = 64 };

typedef enum { T_FLOOR, T_WALL, T_HOLE, T_SHREDDER, T_PRINTER } Terrain;

typedef struct {
    Terrain terrain;
    bool goal;      /* v1: this floor tile is a delivery target */
    bool has_cube;
    int  cube;
    int  owner;     /* worker who printed this cube (-1 = level cube) */
} Tile;

typedef enum { CB_FIXED, CB_RAND, CB_RANDU } CubeMode;
typedef struct { int x, y; CubeMode mode; int value; } CubeDef;

/* a memory slot holds nothing, a number, or a remembered tile; tiles found by
 * `nearest <type>` remember the type so a stale reference can re-resolve */
typedef enum { MV_NOTHING, MV_NUM, MV_TILE } MemKind;
typedef struct { MemKind k; int num, x, y; int ntype; /* CmpKind or -1 */ } MemVal;

enum { NMEM = 4, MAXFOREACH = 8, WORDLEN = 32 };

typedef struct {
    int  x, y;
    bool holding;
    int  held;
    int  held_src_x, held_src_y;  /* floor tile the held cube came from (-1 = printed) */
    int  held_owner;              /* printing worker of the held cube (-1 = level cube) */
    int  pc;
    bool alive;     /* still on the board (false = destroyed / fell) */
    bool done;      /* program finished (END or ran off the end); stays on board */
    bool exited;    /* left via a hole */
    int  exit_x, exit_y, exit_beat;
    long next_ms;   /* when this worker's current command completes */
    int  pend_x, pend_y; /* standing step intent left by a blocked step (-1 none):
                            if the blocker later steps toward us, we swap */
    int  tgt_x, tgt_y;   /* aligned-hole-exit target (or -1) */
    MemVal mem[NMEM];
    int  fe_idx[MAXFOREACH];      /* foreachdir loop positions */
    unsigned char fe_ord[MAXFOREACH][8];   /* per-sweep direction order */
    bool listening;               /* parked on a listenfor */
    bool heard;                   /* release flag set by a matching tell */
    int  printed, fed;            /* printer takes / shredder feeds by this worker */
    int  last_tell;               /* beat of most recent tell (-1 = never) */
    int  last_x, last_y, blocked_beats;   /* traffic-jam detection */
    int  fresh;     /* a just-acquired cube can't be taken from us until our
                       first decision about it completes (2 = acquiring,
                       1 = deciding, 0 = settled): Big Data's chains steal
                       only cubes their holder chose to keep */
} Worker;

typedef enum {
    G_CUBES_ON_GOALS, G_SHREDDED_N, G_ALL_EXITED,          /* v1 */
    G_TUT_PICKUP_DROP, G_CUBES_OFFSET, G_ROOM_CLEARED, G_ALIGNED_HOLE_EXIT,
    G_ALL_CUBES_HELD, G_UNZIP, G_SHRED_ALL, G_ALL_WORKERS_HOLDING,
    G_SORTED_ROW, G_ROWS_FILLED, G_LINE_REVERSED, G_ALL_HOLDING_MIN,
    G_CUBES_LINE_ROW, G_CUBES_DIAGONAL, G_WORKERS_EXIT_DOOR, G_PRINT_SHRED_FOREVER,
    G_ROYALE_MAX_REMAINS, G_FLOOR_COVERED, G_CHECKERBOARD, G_ALL_CUBES_VALUE,
    G_BACKUP_PAIRS, G_SHRED_MIN_PER_COL, G_SHRED_COLS_ASC, G_SHRED_MIN_ROOM,
    G_CUBES_INCREMENTED, G_ROW_SUMS_RIGHT, G_PRINTED_PER_WORKER, G_PRINTED_LABELED,
    G_DECRYPT_LEFT_EXIT, G_EMAIL_SORT, G_MULT_TABLE, G_FASHION_UNIQUE,
    G_ROMANCE_FOREVER, G_CHAIN_GREET, G_TRAINING_DAY, G_ALTERNATE_SHRED,
    G_PRINTSHRED_QUIET, G_IDENTIFY_LINE, G_MODE_COUNTS, G_ALL_VALUES_PRESENT,
    G_CUBES_AVG, G_FLOWER_SUMS, G_SHRED_MAX_PER_GROUP, G_NEIGHBOR_COUNTS,
    G_MAX_NEIGHBORS, G_GLORY_DIVE, G_DISTANCES_FROM_DOOR, G_SORTED_GRID,
    G_DEFRAG, G_GOODBYE,
    G_UNKNOWN
} GoalKind;

enum {                                        /* enforced special rules */
    R_NOWALK = 1, R_UNIQUE_SHRED = 2, R_LABELS_EXPLODE = 4,
    R_LABELS_EXPLODE_NONZERO = 8, R_ONE_SHREDDER = 16, R_SPEAK_ORDER = 32
};

typedef struct {
    char    name[64];
    char    goal_raw[96];
    int     w, h;
    Terrain terr[MAXH][MAXW];
    bool    goalpad[MAXH][MAXW];
    CubeDef cubes[MAXCUBES];
    int     ncubes;
    int     sx[MAXWORKERS], sy[MAXWORKERS];
    int     nworkers;
    int     door_x, door_y;  /* the level's door tile (-1 = none) */
    GoalKind win;
    int     goal_a, goal_b;
    unsigned rules;
    int     randmax;         /* max random cube value (default 99) */
    bool    has_random;      /* any random cube or printer present */
    bool    has_palette;
    char    palette[24][16];
    int     npalette;
} Level;

/* --------------------------------------------------------------- program -- */

typedef enum {
    OP_STEP, OP_PICKUP, OP_DROP, OP_GIVETO, OP_TAKEFROM, OP_END,
    OP_JUMP, OP_IF, OP_ELSE, OP_ENDIF, OP_LABEL, OP_NOP,
    OP_ASSIGN, OP_FOREACH, OP_ENDFOR, OP_WRITE, OP_TELL, OP_LISTEN,
    OP_UNSUPPORTED
} Op;

typedef enum { D_N, D_S, D_E, D_W, D_NE, D_NW, D_SE, D_SW, D_HERE, D_COUNT } Dir;
static const int DX[9] = { 0, 0, 1, -1, 1, -1, 1, -1, 0 };
static const int DY[9] = { -1, 1, 0, 0, -1, -1, 1, 1, 0 };

typedef enum { C_WALL, C_DATACUBE, C_HOLE, C_NOTHING, C_SHREDDER, C_PRINTER, C_PERSON,
               C_SOMETHING } CmpKind;
typedef enum { O_EQ, O_NE, O_LT, O_GT, O_LE, O_GE } CmpOp;

/* an operand of a condition/calc/set/write: number, tile dir, mem slot, myitem */
typedef struct { int kind; /* 0 num, 1 dir, 2 mem, 3 myitem */ int num; Dir dir; int mem; } Operand;

/* one term of an if-condition: <operand> <op> <type|operand> */
typedef struct {
    Operand lhs;
    CmpOp op;
    bool rhs_is_type; CmpKind rhs_type;
    Operand rhs;
    int  conn;           /* connector before this term: 0 none, 1 and, 2 or */
} Cond;

typedef struct {
    Op   op;
    Dir  dirs[8];
    int  ndirs;
    int  mem_target;     /* step/pickup/giveto/takefrom memN target (-1 = dirs) */
    int  target;
    Cond conds[8];
    int  nconds;
    /* OP_ASSIGN / OP_FOREACH / OP_WRITE */
    int  slot;           /* destination mem slot */
    int  akind;          /* assign: 0 nearest, 1 set, 2 calc */
    CmpKind near_type;
    Operand op1, op2;
    int  calcop;         /* '+', '-', '*', '/' */
    int  fe_slot;        /* which fe_idx[] this foreach uses */
    /* OP_TELL / OP_LISTEN */
    char word[WORDLEN];
    int  tt_kind;        /* tell target: 0 none, 1 everyone, 2 dir, 3 mem */
    Dir  tt_dir; int tt_mem;
    char raw[128];
} Instr;

typedef struct {
    Instr instr[MAXPROG];
    int   n;
    char  labels[MAXLABELS][32];
    int   label_line[MAXLABELS];
    int   nlabels;
} Program;

/* ------------------------------------------------------------- utilities -- */

static void die(const char *msg) { fprintf(stderr, "error: %s\n", msg); exit(2); }

static char *rstrip(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t')) s[--n]=0;
    return s;
}
static char *lstrip(char *s) { while (*s==' '||*s=='\t') s++; return s; }

/* ------------------------------------------------------- game-ownership -- */
/*
 * This tool is only useful to people who own 7 Billion Humans, so it declines
 * to run unless the game is present. It reads NOTHING from the game -- this is
 * purely an ownership gate. Detection (any one is enough):
 *   - $SEVENBH_GAME points at an existing path (manual override for odd installs)
 *   - a save profile exists (created once the game has been launched)
 *   - a Steam install is found, including on non-default library drives
 *     (parsed from libraryfolders.vdf) or via the app manifest.
 */
#define SEVENBH_APPID "792100"

static bool path_exists(const char *p) {
    struct stat st;
    return p && *p && stat(p, &st) == 0;
}

static bool steam_lib_has_game(const char *root) {
    char buf[1200];
    snprintf(buf, sizeof buf, "%s/steamapps/common/7 Billion Humans/7 Billion Humans.exe", root);
    if (path_exists(buf)) return true;
    snprintf(buf, sizeof buf, "%s/steamapps/appmanifest_" SEVENBH_APPID ".acf", root);
    return path_exists(buf);
}

/* Minimal libraryfolders.vdf scan: check every `"path" "<root>"` entry. */
static bool scan_libraryfolders(const char *vdf) {
    FILE *f = fopen(vdf, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0 || n > (1 << 20)) { fclose(f); return false; }
    char *data = malloc((size_t)n + 1);
    if (!data) { fclose(f); return false; }
    size_t got = fread(data, 1, (size_t)n, f);
    data[got] = 0;
    fclose(f);

    bool found = false;
    for (char *p = data; (p = strstr(p, "\"path\"")) != NULL; ) {
        p += 6;
        while (*p && *p != '"') p++;
        if (!*p) break;
        p++;
        char root[1024];
        size_t i = 0;
        while (*p && *p != '"' && i < sizeof root - 1) {
            if (*p == '\\' && p[1] == '\\') { root[i++] = '\\'; p += 2; }
            else root[i++] = *p++;
        }
        root[i] = 0;
        if (steam_lib_has_game(root)) { found = true; break; }
    }
    free(data);
    return found;
}

static bool game_installed(void) {
    if (path_exists(getenv("SEVENBH_GAME"))) return true;

    char buf[1200];
    const char *appdata = getenv("APPDATA");                 /* Windows */
    if (appdata) {
        snprintf(buf, sizeof buf, "%s/7 Billion Humans", appdata);
        if (path_exists(buf)) return true;
    }
    const char *home = getenv("HOME");                       /* macOS / Linux */
    if (home) {
        const char *saves[] = {
            "%s/Library/Application Support/7 Billion Humans",
            "%s/.local/share/7 Billion Humans",
        };
        for (size_t i = 0; i < 2; i++) {
            snprintf(buf, sizeof buf, saves[i], home);
            if (path_exists(buf)) return true;
        }
    }

    const char *pf[] = { getenv("ProgramFiles(x86)"), getenv("ProgramW6432"), getenv("ProgramFiles") };
    for (size_t i = 0; i < 3; i++) {
        if (!pf[i]) continue;
        snprintf(buf, sizeof buf, "%s/Steam", pf[i]);
        if (steam_lib_has_game(buf)) return true;
        snprintf(buf, sizeof buf, "%s/Steam/steamapps/libraryfolders.vdf", pf[i]);
        if (scan_libraryfolders(buf)) return true;
        snprintf(buf, sizeof buf, "%s/Steam/config/libraryfolders.vdf", pf[i]);
        if (scan_libraryfolders(buf)) return true;
    }
    if (home) {
        const char *roots[] = { "%s/.steam/steam", "%s/.local/share/Steam",
                                "%s/Library/Application Support/Steam" };
        for (size_t i = 0; i < 3; i++) {
            snprintf(buf, sizeof buf, roots[i], home);
            if (steam_lib_has_game(buf)) return true;
            char vdf[1300];
            snprintf(vdf, sizeof vdf, "%s/steamapps/libraryfolders.vdf", buf);
            if (scan_libraryfolders(vdf)) return true;
        }
    }
    return false;
}

static void require_game(void) {
    if (game_installed()) return;
    fprintf(stderr,
        "7 Billion Humans was not detected on this machine.\n"
        "This emulator is a companion tool for people who own the game.\n"
        "If you own it but it wasn't found, point at your install:\n"
        "  SEVENBH_GAME=\"/path/to/7 Billion Humans/7 Billion Humans.exe\"\n");
    exit(4);
}

static int dir_from(const char *t) {
    if (!strcmp(t,"n"))  return D_N;
    if (!strcmp(t,"s"))  return D_S;
    if (!strcmp(t,"e"))  return D_E;
    if (!strcmp(t,"w"))  return D_W;
    if (!strcmp(t,"ne")) return D_NE;
    if (!strcmp(t,"nw")) return D_NW;
    if (!strcmp(t,"se")) return D_SE;
    if (!strcmp(t,"sw")) return D_SW;
    if (!strcmp(t,"c"))  return D_HERE;
    return -1;
}

/* ---------------------------------------------------------- level loader -- */

static void add_cube(Level *L, int x, int y, CubeMode m, int v) {
    if (L->ncubes >= MAXCUBES) die("too many cubes");
    L->cubes[L->ncubes++] = (CubeDef){ x, y, m, v };
}

static GoalKind goal_from(const char *g, int *a, int *b) {
    char kw[48] = {0};
    int n = sscanf(g, "%47s %d %d", kw, a, b);
    if (n < 2) *a = 0;
    if (n < 3) *b = 0;
    if (!strcmp(kw,"cubes_on_goals"))      return G_CUBES_ON_GOALS;
    if (!strcmp(kw,"shredded"))            return G_SHREDDED_N;
    if (!strcmp(kw,"shredded_count"))      return G_SHREDDED_N;
    if (!strcmp(kw,"all_exited"))          return G_ALL_EXITED;
    if (!strcmp(kw,"tutorial_pickup_drop"))return G_TUT_PICKUP_DROP;
    if (!strcmp(kw,"cubes_offset"))        return G_CUBES_OFFSET;
    if (!strcmp(kw,"room_cleared"))        return G_ROOM_CLEARED;
    if (!strcmp(kw,"aligned_hole_exit"))   return G_ALIGNED_HOLE_EXIT;
    if (!strcmp(kw,"all_cubes_held"))      return G_ALL_CUBES_HELD;
    if (!strcmp(kw,"unzip"))               return G_UNZIP;
    if (!strcmp(kw,"shred_all")) { *a = (strstr(g,"alive_all")!=NULL); return G_SHRED_ALL; }
    if (!strcmp(kw,"all_workers_holding")) return G_ALL_WORKERS_HOLDING;
    if (!strcmp(kw,"sorted_row_holdable")) return G_SORTED_ROW;
    if (!strcmp(kw,"rows_gaps_filled"))    return G_ROWS_FILLED;
    if (!strcmp(kw,"line_reversed"))       return G_LINE_REVERSED;
    if (!strcmp(kw,"all_holding_min"))     return G_ALL_HOLDING_MIN;
    if (!strcmp(kw,"cubes_line_row"))      return G_CUBES_LINE_ROW;
    if (!strcmp(kw,"cubes_diagonal"))      return G_CUBES_DIAGONAL;
    if (!strcmp(kw,"workers_reach_door"))  return G_WORKERS_EXIT_DOOR;
    if (!strcmp(kw,"print_shred_forever")) { if (!*a) *a = 2; return G_PRINT_SHRED_FOREVER; }
    if (!strcmp(kw,"royale_max_remains"))  return G_ROYALE_MAX_REMAINS;
    if (!strcmp(kw,"floor_covered"))       return G_FLOOR_COVERED;
    if (!strcmp(kw,"checkerboard"))        return G_CHECKERBOARD;
    if (!strcmp(kw,"all_cubes_value"))     return G_ALL_CUBES_VALUE;
    if (!strcmp(kw,"backup_pairs"))        return G_BACKUP_PAIRS;
    if (!strcmp(kw,"shred_min_per_col"))   return G_SHRED_MIN_PER_COL;
    if (!strcmp(kw,"shred_cols_ascending"))return G_SHRED_COLS_ASC;
    if (!strcmp(kw,"shred_min_room"))      return G_SHRED_MIN_ROOM;
    if (!strcmp(kw,"cubes_incremented"))   { if (!*a) *a = 1; return G_CUBES_INCREMENTED; }
    if (!strcmp(kw,"row_sums_right"))      return G_ROW_SUMS_RIGHT;
    if (!strcmp(kw,"printed_per_worker"))  { if (!*a) *a = 5; return G_PRINTED_PER_WORKER; }
    if (!strcmp(kw,"printed_labeled_1to5"))return G_PRINTED_LABELED;
    if (!strcmp(kw,"decrypt_left_exit"))   return G_DECRYPT_LEFT_EXIT;
    if (!strcmp(kw,"email_sort"))          return G_EMAIL_SORT;
    if (!strcmp(kw,"mult_table"))          return G_MULT_TABLE;
    if (!strcmp(kw,"fashion_unique"))      return G_FASHION_UNIQUE;
    if (!strcmp(kw,"romance_forever"))     { if (!*a) *a = 6; return G_ROMANCE_FOREVER; }
    if (!strcmp(kw,"chain_greet"))         return G_CHAIN_GREET;
    if (!strcmp(kw,"training_day"))        return G_TRAINING_DAY;
    if (!strcmp(kw,"alternate_shred"))     return G_ALTERNATE_SHRED;
    if (!strcmp(kw,"printshred_quiet"))    { if (!*a) *a = 5; return G_PRINTSHRED_QUIET; }
    if (!strcmp(kw,"identify_line"))       return G_IDENTIFY_LINE;
    if (!strcmp(kw,"mode_counts"))         return G_MODE_COUNTS;
    if (!strcmp(kw,"all_values_present"))  return G_ALL_VALUES_PRESENT;
    if (!strcmp(kw,"cubes_avg"))           return G_CUBES_AVG;
    if (!strcmp(kw,"flower_sums"))         return G_FLOWER_SUMS;
    if (!strcmp(kw,"shred_max_per_group")) return G_SHRED_MAX_PER_GROUP;
    if (!strcmp(kw,"neighbor_counts"))     return G_NEIGHBOR_COUNTS;
    if (!strcmp(kw,"max_neighbors"))       { if (!*a) *a = 3; return G_MAX_NEIGHBORS; }
    if (!strcmp(kw,"glory_dive"))          return G_GLORY_DIVE;
    if (!strcmp(kw,"distances_from_door")) return G_DISTANCES_FROM_DOOR;
    if (!strcmp(kw,"sorted_grid"))         return G_SORTED_GRID;
    if (!strcmp(kw,"defrag"))              { *a = (strstr(g,"ordered")!=NULL); return G_DEFRAG; }
    if (!strcmp(kw,"goodbye_last_tells"))  return G_GOODBYE;
    return G_UNKNOWN;
}

static void load_level(const char *path, Level *L) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open level file");
    memset(L, 0, sizeof *L);
    L->win = G_CUBES_ON_GOALS;
    L->randmax = 99;
    L->door_x = L->door_y = -1;
    char line[512];
    int row = 0;
    while (fgets(line, sizeof line, f)) {
        rstrip(line);
        char *s = lstrip(line);
        if (*s == 0 || *s == '#') continue;
        if (!strncmp(s, "name ", 5)) {
            strncpy(L->name, s + 5, sizeof L->name - 1);
        } else if (!strncmp(s, "dim ", 4)) {
            if (sscanf(s + 4, "%d %d", &L->w, &L->h) != 2) die("bad dim");
            if (L->w > MAXW || L->h > MAXH) die("level too big");
        } else if (!strncmp(s, "row ", 4)) {                       /* v1 grids */
            char *r = s + 4;
            for (int x = 0; x < L->w && r[x]; x++) {
                char c = r[x];
                Terrain *t = &L->terr[row][x];
                switch (c) {
                    case '.': *t = T_FLOOR; break;
                    case '#': *t = T_WALL; break;
                    case 'O': *t = T_HOLE; break;
                    case 'S': *t = T_SHREDDER; break;
                    case 'P': *t = T_PRINTER; break;
                    case 'G': *t = T_FLOOR; L->goalpad[row][x] = true; break;
                    case '@':
                        *t = T_FLOOR;
                        L->sx[L->nworkers] = x; L->sy[L->nworkers] = row;
                        L->nworkers++;
                        break;
                    default:
                        if (isalnum((unsigned char)c)) {
                            *t = T_FLOOR;
                            add_cube(L, x, row, CB_FIXED,
                                     isdigit((unsigned char)c) ? c - '0' : toupper(c));
                        } else die("bad grid char");
                }
            }
            row++;
        } else if (!strncmp(s, "ent ", 4)) {                       /* v2 lists */
            char kind[24] = {0}, val[24] = {0};
            int x = -1, y = -1;
            int n = sscanf(s + 4, "%23s %d %d %23s", kind, &x, &y, val);
            if (n < 3 || x < 0 || y < 0 || x >= MAXW || y >= MAXH) die("bad ent");
            if (!strcmp(kind, "door")) { L->terr[y][x] = T_WALL; L->door_x = x; L->door_y = y; }
            else if (!strcmp(kind, "wall"))                           L->terr[y][x] = T_WALL;
            else if (!strcmp(kind, "hole"))                           L->terr[y][x] = T_HOLE;
            else if (!strcmp(kind, "shredder"))                       L->terr[y][x] = T_SHREDDER;
            else if (!strcmp(kind, "printer"))                        L->terr[y][x] = T_PRINTER;
            else if (!strcmp(kind, "sign"))                           ; /* decorative floor */
            else if (!strcmp(kind, "worker")) {
                if (L->nworkers >= MAXWORKERS) die("too many workers");
                L->sx[L->nworkers] = x; L->sy[L->nworkers] = y; L->nworkers++;
            } else if (!strcmp(kind, "cube")) {
                if (n < 4) die("cube needs a value");
                if (!strcmp(val, "rand"))       add_cube(L, x, y, CB_RAND, 0);
                else if (!strcmp(val, "randu")) add_cube(L, x, y, CB_RANDU, 0);
                else                            add_cube(L, x, y, CB_FIXED, atoi(val));
            } else { /* unknown entity kinds are decorative -- ignore */ }
        } else if (!strncmp(s, "randmax ", 8)) {
            L->randmax = atoi(s + 8);
        } else if (!strncmp(s, "commands ", 9)) {
            L->has_palette = true;
            char *tok = strtok(s + 9, " \t");
            while (tok && L->npalette < 24) {
                strncpy(L->palette[L->npalette++], tok, 15);
                tok = strtok(NULL, " \t");
            }
        } else if (!strncmp(s, "rule ", 5)) {
            if (!strcmp(s + 5, "nowalk"))              L->rules |= R_NOWALK;
            else if (!strcmp(s + 5, "unique_shredder_use")) L->rules |= R_UNIQUE_SHRED;
            else if (!strcmp(s + 5, "labels_explode"))         L->rules |= R_LABELS_EXPLODE;
            else if (!strcmp(s + 5, "labels_explode_nonzero")) L->rules |= R_LABELS_EXPLODE_NONZERO;
            else if (!strcmp(s + 5, "one_shredder_at_a_time")) L->rules |= R_ONE_SHREDDER;
            else if (!strcmp(s + 5, "speak_order"))            L->rules |= R_SPEAK_ORDER;
            /* other rules recorded in the file are not yet enforced */
        } else if (!strncmp(s, "goal ", 5)) {
            strncpy(L->goal_raw, s + 5, sizeof L->goal_raw - 1);
            L->win = goal_from(s + 5, &L->goal_a, &L->goal_b);
        }
        /* year/idx/par_.../flag152 lines: metadata, no effect on simulation */
    }
    fclose(f);
    if (L->w == 0 || L->h == 0) die("level missing dim");
    for (int i = 0; i < L->ncubes; i++)
        if (L->cubes[i].mode == CB_RANDU) L->has_random = true;   /* -1 = blank, not random */
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++)
            if (L->terr[y][x] == T_PRINTER) L->has_random = true;
}

/* -------------------------------------------------------------- parser --- */

static int find_or_add_label(Program *P, const char *name) {
    for (int i = 0; i < P->nlabels; i++)
        if (!strcmp(P->labels[i], name)) return i;
    if (P->nlabels >= MAXLABELS) die("too many labels");
    strncpy(P->labels[P->nlabels], name, 31);
    P->label_line[P->nlabels] = -1;
    return P->nlabels++;
}

static void parse_dirs(Instr *ins, char *arg) {
    ins->ndirs = 0;
    char *tok = strtok(arg, ", \t");
    while (tok && ins->ndirs < 8) {
        int d = dir_from(tok);
        if (d < 0) { ins->op = OP_UNSUPPORTED; return; }
        ins->dirs[ins->ndirs++] = (Dir)d;
        tok = strtok(NULL, ", \t");
    }
}

static int type_from(const char *t, CmpKind *out) {
    if (!strcmp(t,"wall"))      { *out = C_WALL; return 1; }
    if (!strcmp(t,"datacube")||!strcmp(t,"cube")) { *out = C_DATACUBE; return 1; }
    if (!strcmp(t,"hole"))      { *out = C_HOLE; return 1; }
    if (!strcmp(t,"nothing"))   { *out = C_NOTHING; return 1; }
    if (!strcmp(t,"something")) { *out = C_SOMETHING; return 1; }
    if (!strcmp(t,"shredder"))  { *out = C_SHREDDER; return 1; }
    if (!strcmp(t,"printer"))   { *out = C_PRINTER; return 1; }
    if (!strcmp(t,"person")||!strcmp(t,"worker")) { *out = C_PERSON; return 1; }
    return 0;
}

static int cmpop_from(const char *t, CmpOp *out) {
    if (!strcmp(t,"==")) { *out = O_EQ; return 1; }
    if (!strcmp(t,"!=")) { *out = O_NE; return 1; }
    if (!strcmp(t,"<"))  { *out = O_LT; return 1; }
    if (!strcmp(t,">"))  { *out = O_GT; return 1; }
    if (!strcmp(t,"<=")) { *out = O_LE; return 1; }
    if (!strcmp(t,">=")) { *out = O_GE; return 1; }
    return 0;
}

static int mem_from(const char *t) {         /* "mem1".."mem4" -> 0..3 */
    if (!strncmp(t, "mem", 3) && t[3] >= '1' && t[3] <= '0' + NMEM && !t[4])
        return t[3] - '1';
    return -1;
}

/* number | dir | memN | myitem */
static bool operand_from(const char *t, Operand *o) {
    memset(o, 0, sizeof *o);
    int m, d;
    if (!strcmp(t, "myitem"))            { o->kind = 3; return true; }
    if ((m = mem_from(t)) >= 0)          { o->kind = 2; o->mem = m; return true; }
    if ((d = dir_from(t)) >= 0)          { o->kind = 1; o->dir = (Dir)d; return true; }
    if (isdigit((unsigned char)t[0]) || (t[0]=='-'&&isdigit((unsigned char)t[1])))
                                         { o->kind = 0; o->num = atoi(t); return true; }
    return false;
}

/* Parse a full if-condition: term (and|or term)*, evaluated left to right. */
static bool parse_cond(Instr *ins, char *text) {
    char *tok[64]; int nt = 0;
    for (char *t = strtok(text, " \t"); t && nt < 64; t = strtok(NULL, " \t")) tok[nt++] = t;
    int i = 0;
    ins->nconds = 0;
    int conn = 0;
    while (i < nt) {
        if (i + 3 > nt || ins->nconds >= 8) return false;
        Cond *c = &ins->conds[ins->nconds];
        memset(c, 0, sizeof *c);
        c->conn = conn;
        if (!operand_from(tok[i], &c->lhs)) return false;
        if (!cmpop_from(tok[i+1], &c->op)) return false;
        const char *r = tok[i+2];
        CmpKind ck;
        if (type_from(r, &ck)) { c->rhs_is_type = true; c->rhs_type = ck; }
        else if (!operand_from(r, &c->rhs)) return false;
        ins->nconds++;
        i += 3;
        if (i == nt) break;
        if (!strcmp(tok[i], "and"))      conn = 1;
        else if (!strcmp(tok[i], "or"))  conn = 2;
        else return false;
        i++;
    }
    return ins->nconds > 0;
}

/* Turn one logical source line into an Instr. */
static void parse_line(Program *P, char *src) {
    Instr *ins = &P->instr[P->n];
    memset(ins, 0, sizeof *ins);
    ins->op = OP_NOP;
    ins->mem_target = -1;
    char *s = lstrip(rstrip(src));
    snprintf(ins->raw, sizeof ins->raw, "%s", s);

    if (*s == 0) { P->n++; return; }
    if (s[0]=='-' && s[1]=='-') { P->n++; return; }      /* -- header/comment -- */

    size_t len = strlen(s);
    if (len >= 2 && s[len-1]==':' && !strchr(s, ' ')) {  /* label: -- or a bare
        keyword with the game's trailing colon (e.g. "else:") */
        s[len-1] = 0;
        if      (!strcmp(s, "else"))  ins->op = OP_ELSE;
        else if (!strcmp(s, "endif")) ins->op = OP_ENDIF;
        else {
            int li = find_or_add_label(P, s);
            P->label_line[li] = P->n;
            ins->op = OP_LABEL;
        }
        P->n++;
        return;
    }

    char verb[32] = {0};
    char *sp = strpbrk(s, " \t");
    char *arg = NULL;
    if (sp) { size_t vl = (size_t)(sp - s); if (vl > 31) vl = 31; memcpy(verb, s, vl); arg = lstrip(sp); }
    else strncpy(verb, s, 31);

    if (arg) { char *colon = strrchr(arg, ':'); if (colon && colon[1]==0) *colon = 0; }
    for (char *p = verb; *p; p++) *p = (char)tolower((unsigned char)*p);

    int slot;
    if (!strcmp(verb,"comment"))       ins->op = OP_NOP;
    else if (!strcmp(verb,"endblock")) ins->op = OP_NOP;   /* prefilled-program guard */
    else if (!strncmp(verb,"define",6))ins->op = OP_NOP;   /* DEFINE COMMENT doodles */
    else if (!strcmp(verb,"step") || !strcmp(verb,"pickup")
          || !strcmp(verb,"giveto") || !strcmp(verb,"takefrom")) {
        ins->op = !strcmp(verb,"step") ? OP_STEP : !strcmp(verb,"pickup") ? OP_PICKUP
                : !strcmp(verb,"giveto") ? OP_GIVETO : OP_TAKEFROM;
        if (arg) {
            int m = mem_from(arg);
            if (m >= 0) ins->mem_target = m;
            else parse_dirs(ins, arg);
        }
    }
    else if (!strcmp(verb,"drop"))     ins->op = OP_DROP;
    else if (!strcmp(verb,"end"))      ins->op = OP_END;
    else if (!strcmp(verb,"endfor"))   ins->op = OP_ENDFOR;
    else if (!strcmp(verb,"jump"))     { ins->op = OP_JUMP; ins->target = arg ? find_or_add_label(P, arg) : -1; }
    else if (!strcmp(verb,"else"))     ins->op = OP_ELSE;
    else if (!strcmp(verb,"endif"))    ins->op = OP_ENDIF;
    else if (!strcmp(verb,"if")) {
        /* an unparseable condition still nests (endif must balance); it only
         * errors if the worker actually reaches it */
        ins->op = OP_IF;
        if (!arg || !parse_cond(ins, arg)) ins->nconds = 0;
    }
    else if (!strcmp(verb,"write")) {
        ins->op = OP_WRITE;
        if (!arg || !operand_from(arg, &ins->op1)) ins->op = OP_UNSUPPORTED;
    }
    else if (!strcmp(verb,"tell")) {
        /* tell <target> <word>; target = everyone | dir | memN | [blank] */
        ins->op = OP_TELL;
        char t1[32] = {0}, t2[32] = {0};
        if (arg && sscanf(arg, "%31s %31s", t1, t2) >= 1) {
            int d, m;
            if (!strcmp(t1, "everyone"))      ins->tt_kind = 1;
            else if ((d = dir_from(t1)) >= 0) { ins->tt_kind = 2; ins->tt_dir = (Dir)d; }
            else if ((m = mem_from(t1)) >= 0) { ins->tt_kind = 3; ins->tt_mem = m; }
            else                              ins->tt_kind = 0;   /* [blank] / unknown */
            snprintf(ins->word, sizeof ins->word, "%s", t2[0] ? t2 : t1);
        } else ins->op = OP_UNSUPPORTED;
    }
    else if (!strcmp(verb,"listenfor") || !strcmp(verb,"listen")) {
        ins->op = OP_LISTEN;
        if (arg) snprintf(ins->word, sizeof ins->word, "%s", arg);
        else ins->op = OP_UNSUPPORTED;
    }
    else if ((slot = mem_from(verb)) >= 0 && arg && arg[0] == '=') {
        /* memN = nearest <type> | set <operand> | calc <a> <op> <b>
         *      | foreachdir <dirlist>:  (block, closed by endfor) */
        char *rhs = lstrip(arg + 1);
        char kw[24] = {0};
        sscanf(rhs, "%23s", kw);
        char *rest = lstrip(rhs + strlen(kw));
        ins->slot = slot;
        if (!strcmp(kw, "nearest")) {
            ins->op = OP_ASSIGN; ins->akind = 0;
            char ty[24] = {0};
            CmpKind ck;
            if (sscanf(rest, "%23s", ty) == 1 && type_from(ty, &ck)) ins->near_type = ck;
            else ins->op = OP_UNSUPPORTED;
        } else if (!strcmp(kw, "set")) {
            ins->op = OP_ASSIGN; ins->akind = 1;
            char ov[24] = {0};
            if (sscanf(rest, "%23s", ov) == 1 && operand_from(ov, &ins->op1)) ;
            else if (!strncmp(rest, "nothing", 7)) ins->akind = 4;   /* clear slot */
            else {
                /* multi-tile form "set sw,n": remember one of them at random */
                ins->akind = 3;
                parse_dirs(ins, rest);
                if (ins->op != OP_ASSIGN || ins->ndirs == 0) ins->op = OP_UNSUPPORTED;
            }
        } else if (!strcmp(kw, "calc")) {
            ins->op = OP_ASSIGN; ins->akind = 2;
            char a[24] = {0}, o[8] = {0}, b[24] = {0};
            if (sscanf(rest, "%23s %7s %23s", a, o, b) == 3
                && operand_from(a, &ins->op1) && operand_from(b, &ins->op2)
                && (o[0]=='+'||o[0]=='-'||o[0]=='*'||o[0]=='/') && !o[1])
                ins->calcop = o[0];
            else ins->op = OP_UNSUPPORTED;
        } else if (!strcmp(kw, "foreachdir")) {
            ins->op = OP_FOREACH;
            parse_dirs(ins, rest);           /* comma-separated direction list */
            if (ins->op != OP_FOREACH || ins->ndirs == 0) ins->op = OP_UNSUPPORTED;
        } else ins->op = OP_UNSUPPORTED;
    }
    else ins->op = OP_UNSUPPORTED;

    P->n++;
}

static void link_program(Program *P) {
    int stack[256], sp = 0, nfe = 0;
    for (int i = 0; i < P->n; i++) {
        Op op = P->instr[i].op;
        if (op == OP_IF) { stack[sp++] = i; }
        else if (op == OP_ELSE) {
            if (!sp) die("else without if");
            P->instr[stack[sp-1]].target = i;
            stack[sp-1] = i;
        } else if (op == OP_ENDIF) {
            if (!sp) die("endif without if");
            P->instr[stack[--sp]].target = i;
        } else if (op == OP_FOREACH) {
            if (nfe >= MAXFOREACH) die("too many foreachdir loops");
            P->instr[i].fe_slot = nfe++;
            stack[sp++] = i;
        } else if (op == OP_ENDFOR) {
            if (!sp || P->instr[stack[sp-1]].op != OP_FOREACH) die("endfor without foreachdir");
            P->instr[stack[--sp]].target = i;   /* foreach -> its endfor */
        }
    }
    if (sp) die("unclosed if/foreachdir");
    /* endfor -> its foreach: recover by rescanning pairs */
    int fst[256], fsp = 0;
    for (int i = 0; i < P->n; i++) {
        if (P->instr[i].op == OP_FOREACH) fst[fsp++] = i;
        else if (P->instr[i].op == OP_ENDFOR) P->instr[i].target = fst[--fsp];
    }
    for (int i = 0; i < P->n; i++)
        if (P->instr[i].op == OP_JUMP) {
            int li = P->instr[i].target;
            if (li < 0 || P->label_line[li] < 0) die("jump to unknown label");
            P->instr[i].target = P->label_line[li];
        }
}

/* Load, joining multi-line if-conditions (a condition ends at the ':'). */
static void load_program(const char *path, Program *P) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open solution file");
    memset(P, 0, sizeof *P);
    char line[512], joined[1024];
    bool in_defines = false;   /* trailing DEFINE COMMENT blocks (doodle data) */
    while (fgets(line, sizeof line, f)) {
        if (P->n >= MAXPROG) die("program too long");
        rstrip(line);
        char *s = lstrip(line);
        if (!strncmp(s, "DEFINE ", 7)) in_defines = true;
        if (in_defines) {
            Instr *ins = &P->instr[P->n];
            memset(ins, 0, sizeof *ins);
            ins->op = OP_NOP;
            ins->mem_target = -1;
            P->n++;
            continue;
        }
        if (!strncmp(s, "if ", 3) || !strcmp(s, "if")) {
            snprintf(joined, sizeof joined, "%s", s);
            size_t jl = strlen(joined);
            while (jl == 0 || joined[jl-1] != ':') {
                if (!fgets(line, sizeof line, f)) break;
                rstrip(line);
                char *cont = lstrip(line);
                if (jl + 1 + strlen(cont) + 1 >= sizeof joined) die("if-condition too long");
                joined[jl] = ' ';
                strcpy(joined + jl + 1, cont);
                jl = strlen(joined);
            }
            parse_line(P, joined);
        } else {
            parse_line(P, line);
        }
    }
    fclose(f);
    link_program(P);
}

/* SIZE metric: every command except blanks, headers, bare labels, comments --
 * and else/endif, which are part of the if block in the game's editor (Year 05
 * confirms: if+2*step+2*jump scores 5 with endif free). Note: some repo files
 * do not reproduce their recorded score exactly, so treat close calls with
 * in-game verification. */
static int program_size(const Program *P) {
    int n = 0;
    for (int i = 0; i < P->n; i++) {
        Op op = P->instr[i].op;
        if (op==OP_NOP || op==OP_LABEL || op==OP_ELSE || op==OP_ENDIF) continue;
        n++;
    }
    return n;
}

/* Verify the program only uses commands available in this level. */
static const char *op_palette_name(Op op) {
    switch (op) {
        case OP_STEP: return "step";       case OP_PICKUP: return "pickup";
        case OP_DROP: return "drop";       case OP_GIVETO: return "giveto";
        case OP_TAKEFROM: return "takefrom"; case OP_END: return "end";
        case OP_JUMP: return "jump";       case OP_IF: return "if";
        case OP_WRITE: return "write";     case OP_TELL: return "tell";
        case OP_LISTEN: return "listen";   case OP_FOREACH: return "foreachdir";
        default: return NULL;   /* else/endif/endfor/labels ride along */
    }
}

/* OP_ASSIGN maps to nearest/set/calc depending on akind */
static const char *assign_palette_name(const Instr *ins) {
    return ins->akind == 0 ? "nearest" : ins->akind == 1 ? "set" : "calc";
}

static void check_palette(const Level *L, const Program *P) {
    if (!L->has_palette) return;
    for (int i = 0; i < P->n; i++) {
        const char *name = P->instr[i].op == OP_ASSIGN
            ? assign_palette_name(&P->instr[i])
            : op_palette_name(P->instr[i].op);
        if (!name) continue;
        bool ok = false;
        for (int j = 0; j < L->npalette; j++)
            if (!strcmp(L->palette[j], name)) { ok = true; break; }
        /* set and calc are facets of the same assignment block in the game's
         * editor (levels with only "set" accept calc arithmetic) */
        if (!ok && (!strcmp(name, "calc") || !strcmp(name, "set")))
            for (int j = 0; j < L->npalette; j++)
                if (!strcmp(L->palette[j], "set") || !strcmp(L->palette[j], "calc")) { ok = true; break; }
        if (!ok) {
            fprintf(stderr, "error: command '%s' is not available in this level\n", name);
            exit(3);
        }
    }
}

/* --------------------------------------------------------------- runtime -- */

enum { MAXSHREV = 1024, MAXTELLEV = 512 };
typedef struct { int value, src_x, src_y, shr_x, shr_y, worker; } ShredEv;
typedef struct { int worker, x; char word[WORDLEN]; } TellEv;

typedef struct {
    Level  *L;
    Tile    grid[MAXH][MAXW];
    Worker  w[MAXWORKERS];
    int     nw;
    int     shredded;
    long    pickups, drops;
    bool    failed;              /* a special rule was violated */
    int     shred_used[MAXH][MAXW];
    bool    label_tile[MAXH][MAXW];   /* labels_explode rules */
    /* per-trial snapshot of the initial cube placement */
    int     icx[MAXCUBES], icy[MAXCUBES], icv[MAXCUBES];
    int     nic;
    ShredEv shrev[MAXSHREV]; int nshrev;
    TellEv  tellev[MAXTELLEV]; int ntellev;
    int     glory_x, glory_y;    /* G_GLORY_DIVE target hole */
    bool    reach[MAXH][MAXW];   /* floor reachable from worker spawns (the room;
                                    unlisted tiles outside the walls read as
                                    floor in sparse levels and must not count) */
    int     dist_door[MAXH][MAXW];   /* G_DISTANCES_FROM_DOOR (-1 unreachable) */
    int     ic_group[MAXCUBES];      /* G_FLOWER/SHRED_MAX groups (8-connected) */
    int     ngroups;
    bool    door_exit;           /* the door acts as a walk-in exit */
    int     feeds_this_beat;
    int     beat;
    long    now_ms, win_ms;      /* event clock; when the win state was reached */
    long    st_steps, st_bumps, st_items, st_waits;   /* speed-model counters */
    unsigned rng;
} Sim;

/* per-command durations in milliseconds, calibrated against recorded
 * community speeds: step 333, item ops 250; printers and shredders carry
 * their machine animation; assignments (set/calc/nearest) and writes take
 * time too. EMU_MS_* env vars override for calibration runs. */
static int MS_STEP = 333, MS_ITEM = 250, MS_PRINTER = 1200, MS_SHRED = 750,
           MS_TELL = 1000, MS_IF = 333, MS_ASSIGN = 333, MS_WRITE = 1200;

static unsigned rnd(Sim *S) { S->rng = S->rng * 1664525u + 1013904223u; return S->rng >> 8; }

static void sim_reset(Sim *S, Level *L, unsigned seed) {
    memset(S, 0, sizeof *S);
    S->L = L;
    S->rng = seed * 2654435761u + 12345u;
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++)
            S->grid[y][x] = (Tile){ L->terr[y][x], L->goalpad[y][x], false, 0, -1 };

    /* distinct-value pool for CB_RANDU cubes */
    int pool[10000]; int pn = L->randmax + 1;
    if (pn > 10000) pn = 10000;
    for (int i = 0; i < pn; i++) pool[i] = i;
    for (int i = pn - 1; i > 0; i--) { int j = (int)(rnd(S) % (unsigned)(i+1)); int t = pool[i]; pool[i] = pool[j]; pool[j] = t; }
    int pi = 0;

    S->nic = 0;
    for (int i = 0; i < L->ncubes; i++) {
        CubeDef *c = &L->cubes[i];
        int v = c->value;
        /* mode -1 = a BLANK cube (no number printed; reads as 0) -- levels
         * needing real random numbers use mode -2 (distinct draws) */
        if (c->mode == CB_RAND)  v = 0;
        if (c->mode == CB_RAND && L->win == G_LINE_REVERSED) {
            /* blank cubes get hidden serials so "reversed" is meaningful
             * (the game tracks cube identity; equal blanks would make the
             * check degenerate) */
            v = 1;
            for (int j = 0; j < L->ncubes; j++)
                if (L->cubes[j].x < c->x) v++;
        }
        if (c->mode == CB_RANDU) { v = pool[pi]; pi = (pi + 1) % pn; }
        S->grid[c->y][c->x].has_cube = true;
        S->grid[c->y][c->x].cube = v;
        if (((L->rules & R_LABELS_EXPLODE) && c->mode == CB_FIXED)
         || ((L->rules & R_LABELS_EXPLODE_NONZERO) && c->mode == CB_FIXED && c->value != 0))
            S->label_tile[c->y][c->x] = true;
        S->icx[S->nic] = c->x; S->icy[S->nic] = c->y; S->icv[S->nic] = v; S->nic++;
    }

    S->door_exit = (L->win == G_WORKERS_EXIT_DOOR);

    /* the room = floor reachable from any worker spawn */
    {
        static int q[MAXW*MAXH];
        int head = 0, tail = 0;
        for (int i = 0; i < L->nworkers; i++)
            if (!S->reach[L->sy[i]][L->sx[i]]) {
                S->reach[L->sy[i]][L->sx[i]] = true;
                q[tail++] = L->sy[i] * MAXW + L->sx[i];
            }
        while (head < tail) {
            int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
            for (int d = 0; d < 8; d++) {
                int nx = cx + DX[d], ny = cy + DY[d];
                if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
                if (S->reach[ny][nx] || L->terr[ny][nx] != T_FLOOR) continue;
                S->reach[ny][nx] = true;
                q[tail++] = ny * MAXW + nx;
            }
        }
    }

    if (L->win == G_DISTANCES_FROM_DOOR && L->door_x >= 0) {
        /* walking distance (8-dir over floor) from the boss's door */
        for (int y = 0; y < L->h; y++)
            for (int x = 0; x < L->w; x++) S->dist_door[y][x] = -1;
        static int q[MAXW*MAXH];
        int head = 0, tail = 0;
        S->dist_door[L->door_y][L->door_x] = 0;
        q[tail++] = L->door_y * MAXW + L->door_x;
        while (head < tail) {
            int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
            for (int d = 0; d < 8; d++) {
                int nx = cx + DX[d], ny = cy + DY[d];
                if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
                if (S->dist_door[ny][nx] >= 0 || L->terr[ny][nx] != T_FLOOR) continue;
                S->dist_door[ny][nx] = S->dist_door[cy][cx] + 1;
                q[tail++] = ny * MAXW + nx;
            }
        }
    }

    S->ngroups = 0;
    if (L->win == G_FLOWER_SUMS || L->win == G_SHRED_MAX_PER_GROUP) {
        /* 8-connected components of the initial cubes */
        for (int i = 0; i < S->nic; i++) S->ic_group[i] = -1;
        for (int i = 0; i < S->nic; i++) {
            if (S->ic_group[i] >= 0) continue;
            int stack[MAXCUBES], sp = 0;
            S->ic_group[i] = S->ngroups;
            stack[sp++] = i;
            while (sp) {
                int a = stack[--sp];
                for (int j = 0; j < S->nic; j++)
                    if (S->ic_group[j] < 0
                        && abs(S->icx[j]-S->icx[a]) <= 1 && abs(S->icy[j]-S->icy[a]) <= 1) {
                        S->ic_group[j] = S->ngroups;
                        stack[sp++] = j;
                    }
            }
            S->ngroups++;
        }
    }

    S->glory_x = S->glory_y = -1;
    if (L->win == G_GLORY_DIVE) {
        /* the special hole is the one whose WALKING distance (8-dir, around
         * walls and other holes) matches every cube's label */
        static int gd[MAXH][MAXW];
        static int q[MAXW*MAXH];
        for (int hy = 0; hy < L->h && S->glory_x < 0; hy++)
            for (int hx = 0; hx < L->w && S->glory_x < 0; hx++) {
                if (L->terr[hy][hx] != T_HOLE) continue;
                for (int y = 0; y < L->h; y++)
                    for (int x = 0; x < L->w; x++) gd[y][x] = -1;
                int head = 0, tail = 0;
                gd[hy][hx] = 0;
                q[tail++] = hy * MAXW + hx;
                while (head < tail) {
                    int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
                    for (int d = 0; d < 8; d++) {
                        int nx = cx + DX[d], ny = cy + DY[d];
                        if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
                        if (gd[ny][nx] >= 0 || L->terr[ny][nx] != T_FLOOR) continue;
                        gd[ny][nx] = gd[cy][cx] + 1;
                        q[tail++] = ny * MAXW + nx;
                    }
                }
                bool ok = true;
                for (int i = 0; i < S->nic && ok; i++)
                    if (gd[S->icy[i]][S->icx[i]] != S->icv[i]) ok = false;
                if (ok) { S->glory_x = hx; S->glory_y = hy; }
            }
    }

    S->nw = L->nworkers;
    for (int i = 0; i < L->nworkers; i++) {
        Worker *w = &S->w[i];
        memset(w, 0, sizeof *w);
        w->x = L->sx[i]; w->y = L->sy[i];
        w->alive = true;
        w->exit_x = w->exit_y = -1;
        w->pend_x = w->pend_y = -1;
        w->tgt_x = w->tgt_y = -1;
        w->held_src_x = w->held_src_y = -1;
        w->held_owner = -1;
        w->last_tell = -1;
        for (int m = 0; m < NMEM; m++) w->mem[m].k = MV_NOTHING;
        if (L->win == G_ALIGNED_HOLE_EXIT) {
            /* the safe hole lies straight through the worker's adjacent cube */
            int dx = 0;
            if (S->grid[w->y][w->x+1].has_cube) dx = 1;
            else if (S->grid[w->y][w->x-1].has_cube) dx = -1;
            if (dx) {
                for (int x = w->x + dx; x >= 0 && x < L->w; x += dx) {
                    if (L->terr[w->y][x] == T_WALL) break;
                    if (L->terr[w->y][x] == T_HOLE) { w->tgt_x = x; w->tgt_y = w->y; break; }
                }
            }
        }
    }
}

static bool g_nothing_ignores_workers = false;   /* experimental sense variant */

/* Does the tile CONTAIN the queried thing? A tile is a set of contents: it can
 * hold a worker AND a floor cube at once (a worker standing on a cube matches
 * both "== worker" and "== datacube"). `self` is excluded so that querying your
 * own tile ('c') sees the cube beneath your feet, not yourself. A cube held up
 * in the air by a worker does NOT count as a datacube (in-game tip). */
static bool tile_contains(Sim *S, int x, int y, const Worker *self, CmpKind what) {
    Level *L = S->L;
    bool oob = (x < 0 || y < 0 || x >= L->w || y >= L->h);
    if (what == C_WALL) return oob || S->grid[y][x].terrain == T_WALL;
    if (oob) return false;
    Terrain t = S->grid[y][x].terrain;
    switch (what) {
        case C_HOLE:     return t == T_HOLE;
        case C_SHREDDER: return t == T_SHREDDER;
        case C_PRINTER:  return t == T_PRINTER;
        case C_DATACUBE: return t == T_FLOOR && S->grid[y][x].has_cube;
        case C_PERSON:
            for (int i = 0; i < S->nw; i++)
                if (&S->w[i] != self && S->w[i].alive && S->w[i].x == x && S->w[i].y == y)
                    return true;
            return false;
        case C_NOTHING: {
            if (t != T_FLOOR || S->grid[y][x].has_cube) return false;
            if (!g_nothing_ignores_workers)
                for (int i = 0; i < S->nw; i++)
                    if (&S->w[i] != self && S->w[i].alive && S->w[i].x == x && S->w[i].y == y)
                        return false;
            return true;
        }
        case C_SOMETHING:
            return !tile_contains(S, x, y, self, C_NOTHING);
        default: return false;
    }
}

/* Numeric value visible on a tile: a floor cube (even under a worker). A cube
 * held aloft by the worker standing there is visible ONLY in a comparison
 * against myitem ("your workers are smart enough to know you want to compare
 * their items" -- Number Royale tip); plain reads see floor cubes only (the
 * Neighborly Sweeper tip). */
static bool value_at2(Sim *S, int x, int y, const Worker *self, bool see_held, int *out) {
    if (x < 0 || y < 0 || x >= S->L->w || y >= S->L->h) return false;
    if (S->grid[y][x].has_cube) { *out = S->grid[y][x].cube; return true; }
    if (!see_held) return false;
    for (int i = 0; i < S->nw; i++)
        if (&S->w[i] != self && S->w[i].alive && S->w[i].x == x && S->w[i].y == y) {
            if (S->w[i].holding) { *out = S->w[i].held; return true; }
            return false;
        }
    return false;
}


static bool num_cmp(CmpOp op, int a, int b) {
    switch (op) {
        case O_EQ: return a == b;  case O_NE: return a != b;
        case O_LT: return a < b;   case O_GT: return a > b;
        case O_LE: return a <= b;  case O_GE: return a >= b;
    }
    return false;
}

/* numeric value of an operand; false when there is no value to read.
 * Pointing a DIRECTION at a neighbor reads their held item too ("compare
 * their items" -- Number Royale tip); a remembered TILE reads only what lies
 * on the floor of that square (Neighborly Sweeper tip). */
static bool operand_value(Sim *S, Worker *w, const Operand *o, int *out) {
    switch (o->kind) {
        case 0: *out = o->num; return true;
        case 1: return value_at2(S, w->x + DX[o->dir], w->y + DY[o->dir], w, true, out);
        case 3: if (w->holding) { *out = w->held; return true; } return false;
        case 2: {
            MemVal *m = &w->mem[o->mem];
            if (m->k == MV_NUM)  { *out = m->num; return true; }
            if (m->k == MV_TILE) return value_at2(S, m->x, m->y, w, false, out);
            return false;
        }
    }
    return false;
}

static bool cond_true(Sim *S, Cond *c, Worker *w) {
    if (c->rhs_is_type) {
        /* type comparison: does the referenced tile contain the thing? */
        bool eq;
        if (c->lhs.kind == 1)
            eq = tile_contains(S, w->x + DX[c->lhs.dir], w->y + DY[c->lhs.dir], w, c->rhs_type);
        else if (c->lhs.kind == 2) {
            MemVal *m = &w->mem[c->lhs.mem];
            if (m->k == MV_TILE)         eq = tile_contains(S, m->x, m->y, w, c->rhs_type);
            else if (m->k == MV_NOTHING) eq = (c->rhs_type == C_NOTHING);
            else                         eq = false;   /* a number is no tile */
        }
        else if (c->lhs.kind == 3) {
            /* "myitem == datacube/something" = am I holding? "== nothing" = empty */
            if (c->rhs_type == C_DATACUBE || c->rhs_type == C_SOMETHING) eq = w->holding;
            else if (c->rhs_type == C_NOTHING)                           eq = !w->holding;
            else                                                         eq = false;
        }
        else return false;               /* number vs type: not modeled */
        return (c->op == O_NE) ? !eq : eq;
    }
    /* mem vs mem with two remembered tiles compares identity, not contents
     * ("if mem1 != mem2" = are these the same remembered thing?) */
    if (c->lhs.kind == 2 && c->rhs.kind == 2
        && (c->op == O_EQ || c->op == O_NE)) {
        MemVal *ma = &w->mem[c->lhs.mem], *mb = &w->mem[c->rhs.mem];
        if (ma->k == MV_TILE && mb->k == MV_TILE) {
            bool eq = (ma->x == mb->x && ma->y == mb->y);
            return (c->op == O_NE) ? !eq : eq;
        }
        if (ma->k == MV_NOTHING || mb->k == MV_NOTHING) {
            bool eq = (ma->k == mb->k);
            return (c->op == O_NE) ? !eq : eq;
        }
    }
    int a, b;
    bool ha = operand_value(S, w, &c->lhs, &a);
    bool hb = operand_value(S, w, &c->rhs, &b);
    if (!ha || !hb) return c->op == O_NE && (ha != hb);   /* missing value */
    return num_cmp(c->op, a, b);
}

static bool g_rightassoc = false;   /* experimental alternative fold order */

static bool if_true(Sim *S, Instr *ins, Worker *w) {
    if (g_rightassoc) {
        /* right-associative: A op (B op (C op D)) */
        int i = ins->nconds - 1;
        bool acc = cond_true(S, &ins->conds[i], w);
        for (i--; i >= 0; i--) {
            bool v = cond_true(S, &ins->conds[i], w);
            acc = (ins->conds[i+1].conn == 1) ? (v && acc) : (v || acc);
        }
        return acc;
    }
    bool acc = false;
    for (int i = 0; i < ins->nconds; i++) {
        bool v = cond_true(S, &ins->conds[i], w);
        if (i == 0)                acc = v;
        else if (ins->conds[i].conn == 1) acc = acc && v;   /* and */
        else                              acc = acc || v;   /* or  */
    }
    return acc;
}

static int worker_at(Sim *S, int x, int y, int self) {
    for (int i = 0; i < S->nw; i++)
        if (i != self && S->w[i].alive && S->w[i].x == x && S->w[i].y == y) return i;
    return -1;
}

/* movement blocking only: a worker whose program has ended sits down and no
 * longer blocks the aisle (12 deliveries through one shredder ring require it) */
static int blocking_worker_at(Sim *S, int x, int y, int self) {
    for (int i = 0; i < S->nw; i++)
        if (i != self && S->w[i].alive && !S->w[i].done
            && S->w[i].x == x && S->w[i].y == y) return i;
    return -1;
}

/* seated (done) workers are solid for explicit dir-steps -- only routed
 * walks slip past them (Neural Pathways deliveries vs Reverse Line stops) */
static bool done_worker_at(Sim *S, int x, int y) {
    for (int i = 0; i < S->nw; i++)
        if (S->w[i].alive && S->w[i].done && S->w[i].x == x && S->w[i].y == y)
            return true;
    return false;
}

static bool walkable(Sim *S, int x, int y) {
    if (x < 0 || y < 0 || x >= S->L->w || y >= S->L->h) return false;
    if (S->door_exit && x == S->L->door_x && y == S->L->door_y) return true;
    Terrain t = S->grid[y][x].terrain;
    return t == T_FLOOR || t == T_HOLE;
}

/* BFS (8-dir) over floor tiles from (sx,sy). Returns the first-step direction
 * toward the goal, -2 if already at goal, or -1 if unreachable.
 * adjacent_ok: standing on any tile Chebyshev-adjacent to (tx,ty) counts as
 * arrived (for using a remembered thing); otherwise the goal is the tile
 * itself (which may be a hole -- diving is allowed as the final step).
 * block_workers: treat other alive workers' tiles as obstacles. */
static int path_step(Sim *S, const Worker *self, int tx, int ty,
                     bool adjacent_ok, bool block_workers) {
    Level *L = S->L;
    #define ISGOAL(X,Y) (adjacent_ok ? (abs((X)-tx)<=1 && abs((Y)-ty)<=1) \
                                     : ((X)==tx && (Y)==ty))
    if (ISGOAL(self->x, self->y)) return -2;
    static int q[MAXW*MAXH], from[MAXH][MAXW];
    int head = 0, tail = 0;
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++) from[y][x] = -9;
    from[self->y][self->x] = -2;
    q[tail++] = self->y * MAXW + self->x;
    int goal = -1;
    while (head < tail && goal < 0) {
        int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
        for (int d = 0; d < 8 && goal < 0; d++) {
            int nx = cx + DX[d], ny = cy + DY[d];
            if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
            if (from[ny][nx] != -9) continue;
            bool target_tile = (!adjacent_ok && nx == tx && ny == ty);
            Terrain t = S->grid[ny][nx].terrain;
            bool pass = (t == T_FLOOR) || (t == T_HOLE && target_tile);
            if (!pass) continue;
            if (block_workers && blocking_worker_at(S, nx, ny, (int)(self - S->w)) >= 0) continue;
            from[ny][nx] = d;
            if (ISGOAL(nx, ny)) { goal = ny * MAXW + nx; break; }
            if (t != T_HOLE) q[tail++] = ny * MAXW + nx;
        }
    }
    #undef ISGOAL
    if (goal < 0) return -1;
    /* walk back to find the first step */
    int cx = goal % MAXW, cy = goal / MAXW;
    for (;;) {
        int d = from[cy][cx];
        int px = cx - DX[d], py = cy - DY[d];
        if (px == self->x && py == self->y) return d;
        cx = px; cy = py;
    }
}

/* one movement step toward a target, avoiding occupied tiles when possible */
static int route_step(Sim *S, const Worker *self, int tx, int ty, bool adjacent_ok) {
    int d = path_step(S, self, tx, ty, adjacent_ok, false);
    if (d == -2 || d == -1) return d;
    int nx = self->x + DX[d], ny = self->y + DY[d];
    if (blocking_worker_at(S, nx, ny, (int)(self - S->w)) < 0) return d;
    int d2 = path_step(S, self, tx, ty, adjacent_ok, true);
    return (d2 >= 0) ? d2 : d;   /* fall back to the blocked route (bump/wait) */
}

/* does this tile hold a `nearest`-findable thing? Unlike IF-sensing, nearest
 * finds cube ENTITIES -- including one held aloft by a worker standing there
 * (the crowd chases the carrier of the last cube instead of besieging the
 * shredder) */
static bool nearest_matches(Sim *S, const Worker *self, CmpKind type, int x, int y) {
    if (tile_contains(S, x, y, self, type)) return true;
    if (type == C_DATACUBE)
        for (int i = 0; i < S->nw; i++)
            if (&S->w[i] != self && S->w[i].alive && S->w[i].holding
                && S->w[i].x == x && S->w[i].y == y) return true;
    return false;
}

/* nearest thing of a type, by BFS distance; false if none. The caller's own
 * tile counts (Seek and Destroy remembers the cube underfoot that way). */
static bool find_nearest(Sim *S, Worker *w, CmpKind type, int *ox, int *oy) {
    Level *L = S->L;
    if (nearest_matches(S, w, type, w->x, w->y)) { *ox = w->x; *oy = w->y; return true; }
    static int q[MAXW*MAXH];
    static bool seen[MAXH][MAXW];
    memset(seen, 0, sizeof seen);
    int head = 0, tail = 0;
    seen[w->y][w->x] = true;
    q[tail++] = w->y * MAXW + w->x;
    while (head < tail) {
        int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
        for (int d = 0; d < 8; d++) {
            int nx = cx + DX[d], ny = cy + DY[d];
            if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h || seen[ny][nx]) continue;
            seen[ny][nx] = true;
            if (nearest_matches(S, w, type, nx, ny)) { *ox = nx; *oy = ny; return true; }
            if (S->grid[ny][nx].terrain == T_FLOOR) q[tail++] = ny * MAXW + nx;
        }
    }
    return false;
}

/* ------------------------------------------------------------ win checks -- */

static int floor_cube_count(Sim *S) {
    int n = 0;
    for (int y = 0; y < S->L->h; y++)
        for (int x = 0; x < S->L->w; x++)
            if (S->grid[y][x].has_cube) n++;
    return n;
}

static bool level_won(Sim *S) {
    Level *L = S->L;
    if (S->failed) return false;
    switch (L->win) {
        case G_CUBES_ON_GOALS:
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].goal && !S->grid[y][x].has_cube) return false;
            return true;
        case G_SHREDDED_N:
            return S->shredded >= L->goal_a;
        case G_ALL_EXITED:
            for (int i = 0; i < S->nw; i++) if (!S->w[i].exited) return false;
            return true;
        case G_TUT_PICKUP_DROP:
            return S->pickups >= S->nic && S->drops >= S->nic
                && floor_cube_count(S) == S->nic;
        case G_CUBES_OFFSET: {
            for (int i = 0; i < S->nic; i++) {
                int x = S->icx[i] + L->goal_a, y = S->icy[i] + L->goal_b;
                if (x<0||y<0||x>=L->w||y>=L->h || !S->grid[y][x].has_cube) return false;
            }
            return floor_cube_count(S) == S->nic;
        }
        case G_ROOM_CLEARED: {
            if (floor_cube_count(S)) return false;
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].exited || S->w[i].holding) return false;
            return true;
        }
        case G_ALIGNED_HOLE_EXIT:
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->exited) return false;
                if (w->tgt_x >= 0 && (w->exit_x != w->tgt_x || w->exit_y != w->tgt_y)) return false;
            }
            return true;
        case G_ALL_CUBES_HELD: {
            if (floor_cube_count(S)) return false;
            int held = 0;
            for (int i = 0; i < S->nw; i++) if (S->w[i].alive && S->w[i].holding) held++;
            return held == S->nic;
        }
        case G_UNZIP: {
            /* leftmost initial cube moves up one, next down one, alternating */
            for (int i = 0; i < S->nic; i++) {
                int rank = 0;
                for (int j = 0; j < S->nic; j++) if (S->icx[j] < S->icx[i]) rank++;
                int ty = S->icy[i] + ((rank % 2 == 0) ? -1 : 1);
                if (!S->grid[ty][S->icx[i]].has_cube) return false;
            }
            return floor_cube_count(S) == S->nic;
        }
        case G_SHRED_ALL:
            if (S->shredded < S->nic) return false;
            if (L->goal_a)
                for (int i = 0; i < S->nw; i++) if (!S->w[i].alive) return false;
            return true;
        case G_ALL_WORKERS_HOLDING:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || !S->w[i].holding) return false;
            return true;
        case G_SORTED_ROW: {
            /* every cube (on floor or in hand), ordered by x: non-decreasing */
            int xs[MAXCUBES], vs[MAXCUBES], n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube) { xs[n] = x; vs[n] = S->grid[y][x].cube; n++; }
            for (int i = 0; i < S->nw; i++)
                if (S->w[i].alive && S->w[i].holding) { xs[n] = S->w[i].x; vs[n] = S->w[i].held; n++; }
            if (n != S->nic) return false;
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (xs[j] < xs[i] || (xs[j] == xs[i] && vs[j] < vs[i])) {
                        int t = xs[i]; xs[i] = xs[j]; xs[j] = t;
                        t = vs[i]; vs[i] = vs[j]; vs[j] = t;
                    }
            for (int i = 1; i < n; i++) if (vs[i] < vs[i-1]) return false;
            return true;
        }
        case G_ROWS_FILLED: {
            /* every row that (still) holds cubes must have no gaps in its span;
             * rows that were emptied (the supply row) impose nothing */
            bool changed = false;
            for (int y = 0; y < L->h; y++) {
                int mn = MAXW, mx = -1, cur = 0;
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube) { if (x < mn) mn = x; if (x > mx) mx = x; cur++; }
                int init = 0;
                for (int i = 0; i < S->nic; i++) if (S->icy[i] == y) init++;
                if (cur != init) changed = true;
                for (int x = mn; x <= mx; x++)
                    if (S->grid[y][x].terrain == T_FLOOR && !S->grid[y][x].has_cube) return false;
            }
            return changed;   /* the untouched initial board doesn't count */
        }
        case G_LINE_REVERSED: {
            for (int i = 0; i < S->nic; i++) {
                int rank = 0, n = S->nic;
                for (int j = 0; j < n; j++) if (S->icx[j] < S->icx[i]) rank++;
                /* the value initially at rank r must now sit at rank n-1-r's position */
                int want = -1;
                for (int j = 0; j < n; j++) {
                    int rj = 0;
                    for (int k = 0; k < n; k++) if (S->icx[k] < S->icx[j]) rj++;
                    if (rj == n - 1 - rank) { want = S->icv[j]; break; }
                }
                Tile *t = &S->grid[S->icy[i]][S->icx[i]];
                if (!t->has_cube || t->cube != want) return false;
            }
            return true;
        }
        case G_ALL_HOLDING_MIN:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || !S->w[i].holding || S->w[i].held < L->goal_a) return false;
            return true;

        case G_CUBES_LINE_ROW: {
            /* all cubes contiguous in one row, sitting on the bottommost floor */
            int y = -1, mn = MAXW, mx = -1, n = 0;
            for (int ty = 0; ty < L->h; ty++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[ty][x].has_cube) {
                        if (y < 0) y = ty;
                        if (ty != y) return false;
                        if (x < mn) mn = x;
                        if (x > mx) mx = x;
                        n++;
                    }
            if (n != S->nic || y < 0) return false;
            for (int x = mn; x <= mx; x++) {
                if (!S->grid[y][x].has_cube) return false;
                if (y + 1 < L->h && S->grid[y+1][x].terrain == T_FLOOR) return false;
            }
            return true;
        }
        case G_CUBES_DIAGONAL: {
            /* cubes form one diagonal; the workerless anchor cube stays put */
            int ax = -1, ay = -1;
            for (int i = 0; i < S->nic; i++) {
                bool worker_above = false;
                for (int j = 0; j < L->nworkers; j++)
                    if (L->sx[j] == S->icx[i] && L->sy[j] == S->icy[i] - 1) worker_above = true;
                if (!worker_above) { ax = S->icx[i]; ay = S->icy[i]; }
            }
            if (ax >= 0 && !S->grid[ay][ax].has_cube) return false;
            int xs[MAXCUBES], ys[MAXCUBES], n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube) { xs[n] = x; ys[n] = y; n++; }
            if (n != S->nic) return false;
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (xs[j] < xs[i]) {
                        int t = xs[i]; xs[i] = xs[j]; xs[j] = t;
                        t = ys[i]; ys[i] = ys[j]; ys[j] = t;
                    }
            int slope = 0;
            for (int i = 1; i < n; i++) {
                if (xs[i] != xs[i-1] + 1) return false;
                int dy = ys[i] - ys[i-1];
                if (dy != 1 && dy != -1) return false;
                if (slope == 0) slope = dy;
                else if (dy != slope) return false;
            }
            return true;
        }
        case G_WORKERS_EXIT_DOOR:
            for (int i = 0; i < S->nw; i++) if (!S->w[i].exited) return false;
            return true;
        case G_PRINT_SHRED_FOREVER:
        case G_PRINTSHRED_QUIET:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || S->w[i].printed < L->goal_a || S->w[i].fed < L->goal_a)
                    return false;
            return true;
        case G_ROYALE_MAX_REMAINS: {
            int max = 0;
            for (int i = 0; i < S->nic; i++) if (S->icv[i] > max) max = S->icv[i];
            int nmax = 0;
            for (int i = 0; i < S->nic; i++) if (S->icv[i] == max) nmax++;
            int alive = 0;
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (w->alive) {
                    if (!w->holding || w->held != max) return false;
                    alive++;
                } else if (!w->exited) return false;
            }
            return alive == nmax;
        }
        case G_FLOOR_COVERED:
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->reach[y][x] && !S->grid[y][x].has_cube) return false;
            return true;
        case G_CHECKERBOARD: {
            int par = (S->icx[0] + S->icy[0]) & 1;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->reach[y][x]) continue;
                    bool want = (((x + y) & 1) == par);
                    if (S->grid[y][x].has_cube != want) return false;
                }
            return true;
        }
        case G_ALL_CUBES_VALUE: {
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube) {
                        if (S->grid[y][x].cube != L->goal_a) return false;
                        n++;
                    }
            return n == S->nic;
        }
        case G_BACKUP_PAIRS: {
            for (int i = 0; i < L->nworkers; i++) {
                int lx = L->sx[i]-1, rx = L->sx[i]+1, y = L->sy[i];
                int lv = -1, rv = -1;
                for (int k = 0; k < S->nic; k++) {
                    if (S->icx[k] == lx && S->icy[k] == y) lv = S->icv[k];
                    if (S->icx[k] == rx && S->icy[k] == y) rv = S->icv[k];
                }
                if (lv < 0 || rv < 0) continue;      /* not a pair-flanked worker */
                int want = lv < rv ? lv : rv;
                if (!S->grid[y][lx].has_cube || S->grid[y][lx].cube != want) return false;
                if (!S->grid[y][rx].has_cube || S->grid[y][rx].cube != want) return false;
            }
            return true;
        }
        case G_SHRED_MIN_PER_COL: {
            int ncols = 0;
            for (int x = 0; x < L->w; x++) {
                int mn = -1, cnt = 0;
                for (int k = 0; k < S->nic; k++)
                    if (S->icx[k] == x) { cnt++; if (mn < 0 || S->icv[k] < mn) mn = S->icv[k]; }
                if (!cnt) continue;
                ncols++;
                int ev = 0;
                for (int e = 0; e < S->nshrev; e++)
                    if (S->shrev[e].src_x == x) {
                        ev++;
                        if (S->shrev[e].value != mn) return false;
                    }
                if (ev != 1) return false;
            }
            return S->nshrev == ncols;
        }
        case G_SHRED_COLS_ASC: {
            if (S->shredded < S->nic) return false;
            for (int x = 0; x < L->w; x++) {
                int prev = -1;
                for (int e = 0; e < S->nshrev; e++)
                    if (S->shrev[e].src_x == x) {
                        if (S->shrev[e].value < prev) return false;
                        prev = S->shrev[e].value;
                    }
            }
            return true;
        }
        case G_SHRED_MIN_ROOM: {
            if (S->nshrev != 1) return false;
            int mn = S->icv[0];
            for (int k = 1; k < S->nic; k++) if (S->icv[k] < mn) mn = S->icv[k];
            return S->shrev[0].value == mn && floor_cube_count(S) == S->nic - 1;
        }
        case G_CUBES_INCREMENTED: {
            /* every initial value must reappear incremented, cubes back on the
             * floor (position free: workers set them down where they stand) */
            int want[MAXCUBES], nwant = S->nic;
            for (int k = 0; k < S->nic; k++) want[k] = S->icv[k] + L->goal_a;
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    n++;
                    bool matched = false;
                    for (int k = 0; k < nwant; k++)
                        if (want[k] == S->grid[y][x].cube) {
                            want[k] = want[--nwant]; matched = true; break;
                        }
                    if (!matched) return false;
                }
            return n == S->nic && nwant == 0;
        }
        case G_ROW_SUMS_RIGHT: {
            bool allok = true;
            for (int y = 0; y < L->h; y++) {
                int rx = -1, sum = 0, cnt = 0;
                for (int k = 0; k < S->nic; k++)
                    if (S->icy[k] == y) {
                        cnt++;
                        if (S->icx[k] > rx) rx = S->icx[k];
                    }
                if (cnt < 2) continue;
                for (int k = 0; k < S->nic; k++)
                    if (S->icy[k] == y && S->icx[k] != rx) sum += S->icv[k];
                Tile *t = &S->grid[y][rx];
                if (g_goal_dbg)
                    fprintf(stderr, "row_sums y=%d target(%d,%d) expect=%d has=%d val=%d\n",
                            y, rx, y, sum, t->has_cube, t->has_cube ? t->cube : -999);
                if (!t->has_cube || t->cube != sum) {
                    allok = false;
                    if (!g_goal_dbg) return false;
                }
            }
            return allok;
        }
        case G_PRINTED_PER_WORKER: {
            int n = 0;
            for (int i = 0; i < S->nw; i++) {
                if (!S->w[i].alive || S->w[i].printed != L->goal_a || S->w[i].holding) return false;
                n += L->goal_a;
            }
            return floor_cube_count(S) == S->nic + n;
        }
        case G_PRINTED_LABELED: {
            for (int i = 0; i < S->nw; i++) {
                if (!S->w[i].alive || S->w[i].holding) return false;
                bool seen[6] = { false };
                int owned = 0;
                for (int y = 0; y < L->h; y++)
                    for (int x = 0; x < L->w; x++) {
                        Tile *t = &S->grid[y][x];
                        if (!t->has_cube || t->owner != i) continue;
                        owned++;
                        if (t->cube < 1 || t->cube > 5 || seen[t->cube]) return false;
                        seen[t->cube] = true;
                    }
                if (owned != 5) return false;
            }
            return true;
        }
        case G_DECRYPT_LEFT_EXIT: {
            for (int k = 0; k < S->nic; k++) {
                int x = S->icx[k] - S->icv[k], y = S->icy[k];
                if (x < 0 || !S->grid[y][x].has_cube) return false;
            }
            if (floor_cube_count(S) != S->nic) return false;
            for (int i = 0; i < S->nw; i++) if (!S->w[i].exited) return false;
            return true;
        }
        case G_EMAIL_SORT: {
            int nlabel = 0;
            for (int k = 0; k < S->nic; k++)
                if (L->cubes[k].mode == CB_FIXED) {
                    nlabel++;
                    Tile *t = &S->grid[S->icy[k]][S->icx[k]];
                    if (!t->has_cube || t->cube != S->icv[k]) return false;
                }
            if (S->shredded < S->nic - nlabel) return false;
            for (int e = 0; e < S->nshrev; e++) {
                ShredEv *ev = &S->shrev[e];
                int lx = ev->shr_x, ly = ev->shr_y - 1;   /* label above the shredder */
                int label = -1;
                for (int k = 0; k < S->nic; k++)
                    if (S->icx[k] == lx && S->icy[k] == ly && L->cubes[k].mode == CB_FIXED)
                        label = S->icv[k];
                if (label < 0 || ev->value / 10 != label) return false;
            }
            return true;
        }
        case G_MULT_TABLE: {
            for (int k = 0; k < S->nic; k++) {
                if (!(L->cubes[k].mode == CB_FIXED && S->icv[k] == 0)) continue;
                int rh = -1, ch = -1;
                for (int j = 0; j < S->nic; j++) {
                    if (L->cubes[j].mode != CB_FIXED || S->icv[j] == 0) continue;
                    if (S->icy[j] == S->icy[k]) rh = S->icv[j];
                    if (S->icx[j] == S->icx[k]) ch = S->icv[j];
                }
                if (rh < 0 || ch < 0) return false;
                Tile *t = &S->grid[S->icy[k]][S->icx[k]];
                if (!t->has_cube || t->cube != rh * ch) return false;
            }
            return true;
        }
        case G_FASHION_UNIQUE: {
            int seen[128] = { 0 };
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (w->alive) {
                    if (!w->holding || w->held < L->goal_a || w->held > L->goal_b) return false;
                    if (seen[w->held]++) return false;
                } else if (!w->exited) return false;
            }
            for (int v = L->goal_a; v <= L->goal_b; v++) if (!seen[v]) return false;
            return true;
        }
        case G_ROMANCE_FOREVER: {
            if (S->ntellev < L->goal_a) return false;
            for (int e = 1; e < L->goal_a; e++)
                if (S->tellev[e].worker == S->tellev[e-1].worker) return false;
            return S->tellev[0].x < S->tellev[1].x;
        }
        case G_CHAIN_GREET: {
            if (S->ntellev < S->nw - 1) return false;
            for (int e = 1; e < S->nw - 1; e++)
                if (S->tellev[e].x <= S->tellev[e-1].x) return false;
            return true;
        }
        case G_TRAINING_DAY:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || S->w[i].printed < 1 || S->w[i].fed < 1) return false;
            return true;
        case G_ALTERNATE_SHRED: {
            if (S->shredded < S->nic) return false;
            int mid = L->w / 2;
            for (int e = 0; e < S->nshrev; e++) {
                bool left = S->shrev[e].src_x < mid;
                if (left != (e % 2 == 0)) return false;
            }
            return true;
        }
        case G_IDENTIFY_LINE: {
            for (int k = 0; k < S->nic; k++) {
                int rank = 0;
                for (int j = 0; j < S->nic; j++) if (S->icx[j] < S->icx[k]) rank++;
                Tile *t = &S->grid[S->icy[k]][S->icx[k]];
                if (!t->has_cube || t->cube != rank + 1) return false;
            }
            return true;
        }
        case G_MODE_COUNTS: {
            /* fixed cubes (left to right) must count the random values a..b */
            int rx[16], rn = 0;
            for (int k = 0; k < S->nic; k++)
                if (L->cubes[k].mode == CB_FIXED && rn < 16) rx[rn++] = k;
            for (int i = 0; i < rn; i++)      /* sort result cubes by x */
                for (int j = i + 1; j < rn; j++)
                    if (S->icx[rx[j]] < S->icx[rx[i]]) { int t = rx[i]; rx[i] = rx[j]; rx[j] = t; }
            if (rn != L->goal_b - L->goal_a + 1) return false;
            for (int v = L->goal_a; v <= L->goal_b; v++) {
                int count = 0;
                for (int k = 0; k < S->nic; k++)
                    if (L->cubes[k].mode != CB_FIXED && S->icv[k] == v) count++;
                int k = rx[v - L->goal_a];
                Tile *t = &S->grid[S->icy[k]][S->icx[k]];
                if (!t->has_cube || t->cube != count) return false;
            }
            return true;
        }
        case G_ALL_VALUES_PRESENT: {
            bool seen[256] = { false };
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube && S->grid[y][x].cube >= 0 && S->grid[y][x].cube < 256)
                        seen[S->grid[y][x].cube] = true;
            for (int v = L->goal_a; v <= L->goal_b; v++) if (!seen[v]) return false;
            return true;
        }
        case G_CUBES_AVG: {
            long sum = 0;
            for (int k = 0; k < S->nic; k++) sum += S->icv[k];
            int avg = (int)(sum / S->nic);
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (S->grid[y][x].has_cube) {
                        if (S->grid[y][x].cube != avg) return false;
                        n++;
                    }
            return n == S->nic;
        }
        case G_FLOWER_SUMS: {
            /* a flower = 8 cubes ringing an initially-EMPTY tile; the carried
             * result cube must land in that middle showing the ring's sum */
            bool any = false;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (L->terr[y][x] != T_FLOOR) continue;
                    bool was_cube = false;
                    int ring = 0, sum = 0;
                    for (int j = 0; j < S->nic; j++) {
                        if (S->icx[j] == x && S->icy[j] == y) was_cube = true;
                        else if (abs(S->icx[j]-x) <= 1 && abs(S->icy[j]-y) <= 1
                                 && L->cubes[j].mode != CB_FIXED) {
                            ring++;
                            sum += S->icv[j];
                        }
                    }
                    if (was_cube || ring != 8) continue;
                    any = true;
                    Tile *t = &S->grid[y][x];
                    if (!t->has_cube || t->cube != sum) return false;
                }
            return any;
        }
        case G_SHRED_MAX_PER_GROUP: {
            if (S->nshrev != S->ngroups) return false;
            for (int g = 0; g < S->ngroups; g++) {
                int mx = -1;
                for (int k = 0; k < S->nic; k++)
                    if (S->ic_group[k] == g && S->icv[k] > mx) mx = S->icv[k];
                int ev = 0;
                for (int e = 0; e < S->nshrev; e++) {
                    for (int k = 0; k < S->nic; k++)
                        if (S->ic_group[k] == g
                            && S->icx[k] == S->shrev[e].src_x && S->icy[k] == S->shrev[e].src_y) {
                            ev++;
                            if (S->shrev[e].value != mx) return false;
                        }
                }
                if (ev != 1) return false;
            }
            return true;
        }
        case G_NEIGHBOR_COUNTS: {
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    n++;
                    int nb = 0;
                    for (int d = 0; d < 8; d++) {
                        int nx = x + DX[d], ny = y + DY[d];
                        if (nx>=0&&ny>=0&&nx<L->w&&ny<L->h&&S->grid[ny][nx].has_cube) nb++;
                    }
                    if (S->grid[y][x].cube != nb) return false;
                }
            return n == S->nic;
        }
        case G_MAX_NEIGHBORS: {
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    n++;
                    int nb = 0;
                    for (int d = 0; d < 8; d++) {
                        int nx = x + DX[d], ny = y + DY[d];
                        if (nx>=0&&ny>=0&&nx<L->w&&ny<L->h&&S->grid[ny][nx].has_cube) nb++;
                    }
                    if (nb > L->goal_a) return false;
                }
            return n == S->nic;
        }
        case G_GLORY_DIVE:
            if (S->glory_x < 0) return false;
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->exited || w->exit_x != S->glory_x || w->exit_y != S->glory_y) return false;
            }
            return true;
        case G_DISTANCES_FROM_DOOR: {
            int n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    n++;
                    if (S->grid[y][x].cube != S->dist_door[y][x]) return false;
                }
            return n == S->nic;
        }
        case G_SORTED_GRID: {
            /* initial positions in row-major order must hold ascending values */
            int idx[MAXCUBES];
            for (int k = 0; k < S->nic; k++) idx[k] = k;
            for (int i = 0; i < S->nic; i++)
                for (int j = i + 1; j < S->nic; j++)
                    if (S->icy[idx[j]] < S->icy[idx[i]]
                        || (S->icy[idx[j]] == S->icy[idx[i]] && S->icx[idx[j]] < S->icx[idx[i]])) {
                        int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
                    }
            int prev = -1;
            for (int i = 0; i < S->nic; i++) {
                Tile *t = &S->grid[S->icy[idx[i]]][S->icx[idx[i]]];
                if (!t->has_cube || t->cube < prev) return false;
                prev = t->cube;
            }
            return true;
        }
        case G_DEFRAG: {
            /* cubes fill the room's floor tiles in row-major order, no gaps;
             * "ordered" also preserves the initial row-major value sequence */
            int idx[MAXCUBES];
            for (int k = 0; k < S->nic; k++) idx[k] = k;
            for (int i = 0; i < S->nic; i++)
                for (int j = i + 1; j < S->nic; j++)
                    if (S->icy[idx[j]] < S->icy[idx[i]]
                        || (S->icy[idx[j]] == S->icy[idx[i]] && S->icx[idx[j]] < S->icx[idx[i]])) {
                        int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
                    }
            int filled = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->reach[y][x]) continue;
                    bool want = (filled < S->nic);
                    if (S->grid[y][x].has_cube != want) return false;
                    if (want && L->goal_a && S->grid[y][x].cube != S->icv[idx[filled]]) return false;
                    if (want) filled++;
                }
            return filled == S->nic;
        }
        case G_GOODBYE: {
            int last = -1;
            for (int i = 0; i < S->nw; i++) {
                if (!S->w[i].exited) return false;
                if (last < 0 || S->w[i].exit_beat > S->w[last].exit_beat) last = i;
            }
            return S->ntellev == 1 && S->tellev[0].worker == last;
        }
        case G_UNKNOWN:
            return false;
    }
    return false;
}

/* ---------------------------------------------------------------- round --- */

typedef struct {
    int  action;      /* instr index or -1 */
    int  tx, ty;      /* movement target this beat (-1 = none) */
    bool walk_only;   /* mid macro-walk: move but do not execute/advance */
} Intent;

static void fall_check(Sim *S, Worker *w) {
    if (S->grid[w->y][w->x].terrain == T_HOLE
        || (S->door_exit && w->x == S->L->door_x && w->y == S->L->door_y)) {
        w->alive = false; w->exited = true;
        w->exit_x = w->x; w->exit_y = w->y;
        w->exit_beat = S->beat;
        w->holding = false;                      /* the cube falls with them */
    }
}

/* resolve a mem slot to a tile; false if it holds no tile */
static bool mem_tile(Worker *w, int slot, int *tx, int *ty) {
    if (slot < 0 || w->mem[slot].k != MV_TILE) return false;
    *tx = w->mem[slot].x; *ty = w->mem[slot].y;
    return true;
}

/* like mem_tile, but a stale nearest-ref (the thing is gone from that tile)
 * re-resolves to the current nearest of the same type -- the game's workers
 * chase the THING they remembered, not the square it stood on */
static bool mem_tile_fresh(Sim *S, Worker *w, int slot, int *tx, int *ty) {
    if (!mem_tile(w, slot, tx, ty)) return false;
    MemVal *m = &w->mem[slot];
    if (m->ntype >= 0 && !nearest_matches(S, w, (CmpKind)m->ntype, *tx, *ty)) {
        int x, y;
        if (find_nearest(S, w, (CmpKind)m->ntype, &x, &y)) {
            m->x = x; m->y = y;
            *tx = x; *ty = y;
        } else {
            m->k = MV_NOTHING;
            m->ntype = -1;
            return false;
        }
    }
    return true;
}

/* nearest / set / calc assignment (control-flow speed: executes inline).
 * Operands evaluate BEFORE the slot updates: "mem1 = calc mem1 + c" must
 * read the old mem1 (accumulator loops in Dangerous Spreadsheeting). */
static void exec_assign(Sim *S, Worker *w, Instr *ins) {
    MemVal nv = { MV_NOTHING, 0, 0, 0, -1 };
    if (ins->akind == 0) {                      /* nearest <type> */
        int x, y;
        if (find_nearest(S, w, ins->near_type, &x, &y)) {
            nv.k = MV_TILE; nv.x = x; nv.y = y; nv.ntype = (int)ins->near_type;
        }
    } else if (ins->akind == 1) {               /* set <operand> */
        Operand *o = &ins->op1;
        if (o->kind == 1) { nv.k = MV_TILE; nv.x = w->x + DX[o->dir]; nv.y = w->y + DY[o->dir]; }
        else if (o->kind == 2) nv = w->mem[o->mem];
        else if (o->kind == 3) { if (w->holding) { nv.k = MV_NUM; nv.num = w->held; } }
        else { nv.k = MV_NUM; nv.num = o->num; }
    } else if (ins->akind == 3) {               /* set <dir,dir,...>: random pick */
        Dir d = ins->dirs[ins->ndirs == 1 ? 0 : (int)(rnd(S) % (unsigned)ins->ndirs)];
        nv.k = MV_TILE; nv.x = w->x + DX[d]; nv.y = w->y + DY[d];
    } else if (ins->akind == 4) {               /* set nothing: clear the slot */
        ;                                       /* nv already nothing */
    } else {                                    /* calc <a> <op> <b> */
        /* a missing operand counts as 0 ("1 + wall" labels the line's first
         * cube 1 in Identify Yourselves); only division by zero fails */
        int a = 0, b = 0;
        operand_value(S, w, &ins->op1, &a);
        operand_value(S, w, &ins->op2, &b);
        switch (ins->calcop) {
            case '+': nv.k = MV_NUM; nv.num = a + b; break;
            case '-': nv.k = MV_NUM; nv.num = a - b; break;
            case '*': nv.k = MV_NUM; nv.num = a * b; break;
            case '/': if (b != 0) { nv.k = MV_NUM; nv.num = a / b; } break;
        }
    }
    w->mem[ins->slot] = nv;
}

/* shared action helpers (used by both dir- and mem-targeted forms) */

static void feed_shredder(Sim *S, Worker *w, int wi, int nx, int ny) {
    if ((S->L->rules & R_UNIQUE_SHRED) && S->shred_used[ny][nx]) {
        w->alive = false;                        /* violently destroyed */
        w->holding = false;
        return;
    }
    S->shred_used[ny][nx]++;
    S->feeds_this_beat++;
    if ((S->L->rules & R_ONE_SHREDDER) && S->feeds_this_beat > 1) S->failed = true;
    if (S->nshrev < MAXSHREV)
        S->shrev[S->nshrev++] = (ShredEv){ w->held, w->held_src_x, w->held_src_y, nx, ny, wi };
    w->holding = false;
    w->fed++;
    S->shredded++;
}

/* returns true if something was picked up (or the worker exploded) */
static bool pickup_at(Sim *S, Worker *w, int wi, int nx, int ny) {
    Tile *t = &S->grid[ny][nx];
    if (t->terrain == T_PRINTER) {               /* fresh print */
        w->holding = true;
        w->held = (int)(rnd(S) % (unsigned)(S->L->randmax + 1));
        w->held_src_x = w->held_src_y = -1;
        w->held_owner = wi;
        w->fresh = 2;
        w->printed++;
        S->pickups++;
        return true;
    }
    if (t->has_cube) {
        if (S->label_tile[ny][nx]) {             /* DO NOT PICK UP THE LABELS */
            w->alive = false;
            S->failed = true;
            return true;
        }
        w->holding = true;
        w->held = t->cube;
        w->held_src_x = nx; w->held_src_y = ny;
        w->held_owner = t->owner;
        w->fresh = 2;
        t->has_cube = false; t->owner = -1;
        S->pickups++;
        return true;
    }
    return false;
}

static bool g_trace = false;
static bool g_nochain = false;              /* experimental movement variant */

/* how long the worker is busy after starting this command */
static long cmd_duration(Sim *S, Worker *w, Instr *ins) {
    switch (ins->op) {
        case OP_STEP: return MS_STEP;
        case OP_PICKUP: case OP_TAKEFROM: {
            if (ins->mem_target >= 0) {
                int tx, ty;
                if (mem_tile(w, ins->mem_target, &tx, &ty)
                    && S->grid[ty][tx].terrain == T_PRINTER) return MS_PRINTER;
            } else
                for (int k = 0; k < ins->ndirs; k++) {
                    int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                    if (nx >= 0 && ny >= 0 && nx < S->L->w && ny < S->L->h
                        && S->grid[ny][nx].terrain == T_PRINTER) return MS_PRINTER;
                }
            return MS_ITEM;
        }
        case OP_GIVETO: {
            if (ins->mem_target >= 0) {
                int tx, ty;
                if (mem_tile(w, ins->mem_target, &tx, &ty)
                    && S->grid[ty][tx].terrain == T_SHREDDER) return MS_SHRED;
            } else
                for (int k = 0; k < ins->ndirs; k++) {
                    int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                    if (nx >= 0 && ny >= 0 && nx < S->L->w && ny < S->L->h
                        && S->grid[ny][nx].terrain == T_SHREDDER) return MS_SHRED;
                }
            return MS_ITEM;
        }
        case OP_DROP: return MS_ITEM;
        case OP_WRITE: return MS_WRITE;
        case OP_ASSIGN: return MS_ASSIGN;
        case OP_TELL: return MS_TELL;
        case OP_IF: return MS_IF;
        default: return 0;
    }
}

static void trace_board(Sim *S, int round) {
    fprintf(stderr, "-- round %d  shredded %d --\n", round, S->shredded);
    for (int y = 0; y < S->L->h; y++) {
        char row[MAXW*3+1]; int p = 0;
        for (int x = 0; x < S->L->w; x++) {
            int wi = worker_at(S, x, y, -1);
            char ch;
            if (wi >= 0) ch = S->w[wi].holding ? 'H' : 'W';
            else switch (S->grid[y][x].terrain) {
                case T_WALL: ch = '#'; break;
                case T_HOLE: ch = 'O'; break;
                case T_SHREDDER: ch = 'S'; break;
                case T_PRINTER: ch = 'P'; break;
                default: ch = S->grid[y][x].has_cube ? 'c' : '.'; break;
            }
            row[p++] = ch;
        }
        row[p] = 0;
        fprintf(stderr, "  %s\n", row);
    }
}

/* run one trial to completion; returns win, fills *out_rounds. Real solutions
 * finish in well under a few thousand beats; the cap bounds failing runs. */
static bool run(Sim *S, Program *P, int *out_rounds) {
    int rounds = 0;
    static int cap = 0;
    if (!cap) {
        const char *e = getenv("EMU_CAP");
        cap = e ? atoi(e) : 20000;
        if (cap < 100) cap = 20000;
    }
    const int CAP = cap;
    if (level_won(S)) { *out_rounds = 0; return true; }

    while (rounds < CAP) {
        bool any_active = false;
        int nactors = 0, nidle = 0, nbusy = 0;
        Intent it[MAXWORKERS];
        S->feeds_this_beat = 0;

        /* event clock: this batch happens at the earliest pending completion.
         * Workers still mid-command (next_ms > t) hold their tiles but do not
         * act; they are "busy", not idle. */
        long t = -1;
        for (int i = 0; i < S->nw; i++)
            if (S->w[i].alive && !S->w[i].done
                && (t < 0 || S->w[i].next_ms < t)) t = S->w[i].next_ms;
        if (t < 0) break;                 /* nobody left to act */
        S->now_ms = t;

        /* phase A: every due worker flows through control ops and picks its
         * action (sensing sees the pre-move board, so mutual swaps are
         * symmetric). A worker whose control flow finds no action idles. */
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            it[i].action = -1; it[i].tx = -1; it[i].walk_only = false;
            if (!w->alive || w->done) continue;
            if (w->next_ms > t) { nbusy++; any_active = true; continue; }
            int guard = 0, budget = P->n * 2 + 16;
            bool idle = false;
            for (;;) {
                if (++guard > budget) { idle = true; break; }   /* waiting on a condition */
                if (w->pc >= P->n) { w->done = true; break; }
                Instr *ins = &P->instr[w->pc];
                switch (ins->op) {
                    case OP_NOP: case OP_LABEL: w->pc++; continue;
                    case OP_JUMP: w->pc = ins->target; continue;
                    /* OP_IF falls through as an ACTION: evaluating a condition
                     * takes time in the game (its cost shows in every
                     * if-in-loop level's recorded speed) */
                    case OP_ELSE: w->pc = ins->target; continue;
                    case OP_ENDIF: w->pc++; continue;
                    case OP_ASSIGN:
                        /* assignments take time (an action beat) unless
                         * calibrated free -- then they run inline */
                        if (MS_ASSIGN == 0) { exec_assign(S, w, ins); w->pc++; continue; }
                        break;
                    case OP_FOREACH: {
                        int *fi = &w->fe_idx[ins->fe_slot];
                        unsigned char *ord = w->fe_ord[ins->fe_slot];
                        if (*fi == 0) {
                            /* fresh sweep: visit the listed tiles in a random
                             * order (deterministic order locks wandering
                             * programs into permanent orbits) */
                            for (int k = 0; k < ins->ndirs; k++) ord[k] = (unsigned char)k;
                            for (int k = ins->ndirs - 1; k > 0; k--) {
                                int j = (int)(rnd(S) % (unsigned)(k + 1));
                                unsigned char t = ord[k]; ord[k] = ord[j]; ord[j] = t;
                            }
                        }
                        if (*fi < ins->ndirs) {
                            Dir d = ins->dirs[ord[(*fi)++]];
                            w->mem[ins->slot].k = MV_TILE;
                            w->mem[ins->slot].x = w->x + DX[d];
                            w->mem[ins->slot].y = w->y + DY[d];
                            w->mem[ins->slot].ntype = -1;
                            w->pc++;
                        } else { *fi = 0; w->pc = ins->target + 1; }
                        continue;
                    }
                    case OP_ENDFOR: w->pc = ins->target; continue;
                    case OP_LISTEN:
                        if (w->heard) { w->heard = false; w->pc++; continue; }
                        idle = true;
                        break;
                    case OP_UNSUPPORTED:
                        fprintf(stderr, "error: unsupported command: %s\n", ins->raw);
                        exit(3);
                    default: break;
                }
                break;
            }
            if (!w->alive || w->done) continue;
            any_active = true;
            if (idle) { nidle++; continue; }

            Instr *ins = &P->instr[w->pc];
            if (ins->op == OP_STEP) {
                if (S->L->rules & R_NOWALK) { S->failed = true; }
                nactors++;
                it[i].action = w->pc;
                if (w->pend_x >= 0) {
                    /* resuming a parked (queued) step: stay committed to the
                     * same target -- but give up when the blocker has seated
                     * for good (they will never move; the program must get
                     * to re-check its conditions: Reverse Line stops behind
                     * the finished line instead of climbing onto it) */
                    if (done_worker_at(S, w->pend_x, w->pend_y))
                        w->pend_x = w->pend_y = -1;  /* bump: pc advances */
                    else { it[i].tx = w->pend_x; it[i].ty = w->pend_y; }
                } else if (ins->mem_target >= 0) {
                    int tx, ty;
                    if (mem_tile(w, ins->mem_target, &tx, &ty)) {
                        int d = route_step(S, w, tx, ty, false);
                        if (d >= 0) { it[i].tx = w->x + DX[d]; it[i].ty = w->y + DY[d]; }
                    }
                } else {
                    /* choose among listed dirs: random pick among passable ones */
                    int cand[8], nc = 0;
                    for (int k = 0; k < ins->ndirs; k++) {
                        Dir d = ins->dirs[k];
                        if (walkable(S, w->x + DX[d], w->y + DY[d])) cand[nc++] = d;
                    }
                    if (nc > 0) {
                        int d = (nc == 1) ? cand[0] : cand[rnd(S) % (unsigned)nc];
                        it[i].tx = w->x + DX[d]; it[i].ty = w->y + DY[d];
                    }
                }
            } else if (ins->mem_target >= 0
                       && (ins->op == OP_PICKUP || ins->op == OP_GIVETO || ins->op == OP_TAKEFROM)) {
                /* using a remembered thing walks there first (implicit travel) --
                 * but an action that would no-op anyway skips instantly, no walk */
                int tx, ty;
                if (w->x != w->last_x || w->y != w->last_y) {
                    w->last_x = w->x; w->last_y = w->y; w->blocked_beats = 0;
                } else w->blocked_beats++;
                /* pickup goes to STAND ON the thing (you lift what's under
                 * you); giveto/takefrom act from an adjacent tile */
                bool onto = (ins->op == OP_PICKUP);
                #define ARRIVED() (onto ? (w->x == tx && w->y == ty) \
                                        : (abs(w->x - tx) <= 1 && abs(w->y - ty) <= 1))
                if (((ins->op == OP_PICKUP || ins->op == OP_TAKEFROM) && w->holding)
                    || (ins->op == OP_GIVETO && !w->holding)) {
                    nactors++;
                    it[i].action = w->pc;        /* no-op action beat */
                } else if (!mem_tile(w, ins->mem_target, &tx, &ty)) {
                    nactors++;
                    it[i].action = w->pc;        /* nothing remembered: no-op */
                } else if (ARRIVED()) {
                    /* arrived -- but if the remembered thing is gone from this
                     * tile, chase its kind to the next nearest and keep going */
                    if (!mem_tile_fresh(S, w, ins->mem_target, &tx, &ty) || ARRIVED()) {
                        nactors++;
                        it[i].action = w->pc;    /* act (or no-op) this beat */
                    } else {
                        int d = route_step(S, w, tx, ty, !onto);
                        if (d >= 0) {
                            nactors++;
                            it[i].action = w->pc;
                            it[i].walk_only = true;
                            it[i].tx = w->x + DX[d]; it[i].ty = w->y + DY[d];
                        } else nidle++;
                    }
                } else {
                    if (S->L->rules & R_NOWALK) S->failed = true;
                    int d = route_step(S, w, tx, ty, !onto);
                    if (d >= 0 && w->blocked_beats > 3) {
                        /* jammed in a crowd: shuffle to any free floor tile */
                        int cand[8], nc = 0;
                        for (int k = 0; k < 8; k++) {
                            int nx = w->x + DX[k], ny = w->y + DY[k];
                            if (nx>=0&&ny>=0&&nx<S->L->w&&ny<S->L->h
                                && S->grid[ny][nx].terrain == T_FLOOR
                                && blocking_worker_at(S, nx, ny, i) < 0) cand[nc++] = k;
                        }
                        if (nc > 0) d = cand[rnd(S) % (unsigned)nc];
                    }
                    if (d >= 0) {
                        nactors++;
                        it[i].action = w->pc;
                        it[i].walk_only = true;
                        it[i].tx = w->x + DX[d]; it[i].ty = w->y + DY[d];
                    } else nidle++;              /* no route: wait */
                }
                #undef ARRIVED
            } else {
                nactors++;
                it[i].action = w->pc;
            }
        }
        if (S->failed) { *out_rounds = rounds; return false; }
        if (!any_active) break;
        /* all waiting and nothing in flight: frozen for good */
        if (nactors == 0 && nidle > 0 && nbusy == 0) break;

        /* speak-order rule: only one worker may tell per beat -- the one who
         * spoke least recently (leftmost on a tie); the rest retry next beat */
        if (S->L->rules & R_SPEAK_ORDER) {
            int winner = -1;
            for (int i = 0; i < S->nw; i++) {
                if (it[i].action < 0 || P->instr[it[i].action].op != OP_TELL) continue;
                if (winner < 0
                    || S->w[i].last_tell < S->w[winner].last_tell
                    || (S->w[i].last_tell == S->w[winner].last_tell && S->w[i].x < S->w[winner].x))
                    winner = i;
            }
            for (int i = 0; i < S->nw; i++)
                if (i != winner && it[i].action >= 0 && P->instr[it[i].action].op == OP_TELL)
                    it[i].action = -1;           /* blocked; pc unchanged */
        }

        /* phase B: resolve movement simultaneously (swaps, chains, bumps) */
        #define IS_MOVER(i) (it[i].tx >= 0 && it[i].action >= 0 \
            && (it[i].walk_only || P->instr[it[i].action].op == OP_STEP))
        int prex[MAXWORKERS], prey[MAXWORKERS];
        for (int i = 0; i < S->nw; i++) { prex[i] = S->w[i].x; prey[i] = S->w[i].y; }
        bool resolved[MAXWORKERS] = { false };
        if (g_nochain) {
            /* variant: a move only succeeds into a tile that was free at beat
             * start; the sole exception is the mutual swap */
            int px[MAXWORKERS], py[MAXWORKERS];
            for (int i = 0; i < S->nw; i++) { px[i] = S->w[i].x; py[i] = S->w[i].y; }
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->alive || w->done || resolved[i]) continue;
                if (!IS_MOVER(i)) continue;
                int occ = -1;
                for (int j = 0; j < S->nw; j++)
                    if (j != i && S->w[j].alive && px[j] == it[i].tx && py[j] == it[i].ty) { occ = j; break; }
                if (occ < 0) {
                    if (blocking_worker_at(S, it[i].tx, it[i].ty, i) < 0) {
                        w->x = it[i].tx; w->y = it[i].ty; resolved[i] = true;
                    }
                } else if (!resolved[occ] && IS_MOVER(occ)
                           && it[occ].tx == px[i] && it[occ].ty == py[i]) {
                    Worker *o = &S->w[occ];
                    o->x = it[occ].tx; o->y = it[occ].ty;
                    w->x = it[i].tx; w->y = it[i].ty;
                    resolved[i] = resolved[occ] = true;
                }
            }
        } else {
        for (;;) {
            bool progress = false;
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->alive || w->done || resolved[i]) continue;
                if (!IS_MOVER(i)) continue;
                int occ = blocking_worker_at(S, it[i].tx, it[i].ty, i);
                if (occ < 0) {
                    w->x = it[i].tx; w->y = it[i].ty;
                    w->pend_x = w->pend_y = -1;
                    resolved[i] = true; progress = true;
                } else if (!resolved[occ] && IS_MOVER(occ)
                           && it[occ].tx == w->x && it[occ].ty == w->y) {
                    /* mutual swap: both trying to walk into each other */
                    Worker *o = &S->w[occ];
                    int ox = o->x, oy = o->y;
                    o->x = it[occ].tx; o->y = it[occ].ty;
                    w->x = ox; w->y = oy;
                    w->pend_x = w->pend_y = -1;
                    o->pend_x = o->pend_y = -1;
                    resolved[i] = resolved[occ] = true; progress = true;
                } else {
                    /* blocked -- but if the blocker left a standing intent to
                     * step into OUR tile, the trade happens now */
                    Worker *o = &S->w[occ];
                    if (o->pend_x == w->x && o->pend_y == w->y) {
                        int ox = o->x, oy = o->y;
                        o->x = w->x; o->y = w->y;
                        w->x = ox; w->y = oy;
                        o->pend_x = o->pend_y = -1;
                        w->pend_x = w->pend_y = -1;
                        resolved[i] = true; progress = true;
                        /* the trade completes o's parked step command */
                        if (it[occ].action < 0 && o->pc < P->n
                            && P->instr[o->pc].op == OP_STEP) {
                            if (o->fresh > 0) o->fresh--;
                            o->pc++;
                            fall_check(S, o);
                        }
                    }
                }
            }
            if (!progress) break;
        }
        /* blocked steppers leave their intent standing for a later trade */
        for (int i = 0; i < S->nw; i++)
            if (S->w[i].alive && !S->w[i].done && !resolved[i] && IS_MOVER(i)
                && !it[i].walk_only) {
                S->w[i].pend_x = it[i].tx;
                S->w[i].pend_y = it[i].ty;
            }
        /* rotation cycles: A->B's tile, B->C's, C->A's -- everyone rotates */
        for (int i = 0; i < S->nw; i++) {
            if (resolved[i] || !S->w[i].alive || S->w[i].done || !IS_MOVER(i)) continue;
            int chain[MAXWORKERS], cn = 0, cur = i;
            bool cyc = false;
            while (cn < S->nw) {
                chain[cn++] = cur;
                int occ = blocking_worker_at(S, it[cur].tx, it[cur].ty, cur);
                if (occ < 0 || resolved[occ] || !IS_MOVER(occ)) break;
                if (occ == i) { cyc = true; break; }
                bool seen = false;
                for (int k = 0; k < cn; k++) if (chain[k] == occ) seen = true;
                if (seen) break;
                cur = occ;
            }
            if (cyc && cn > 1) {
                for (int k = 0; k < cn; k++) {
                    Worker *w = &S->w[chain[k]];
                    w->x = it[chain[k]].tx; w->y = it[chain[k]].ty;
                    resolved[chain[k]] = true;
                }
            }
        }
        }
        /* an explicit step blocked by another worker WAITS (uncharged, pc
         * held) and re-attempts at the next world event: walkers queue, and
         * the retry lands in the same batch as the blocker's next move so
         * chains and swaps resolve. Bumping through instead would let
         * accumulator sweeps double-count their tile (Dangerous
         * Spreadsheeting). */
        for (int i = 0; i < S->nw; i++)
            if (IS_MOVER(i) && !it[i].walk_only
                && P->instr[it[i].action].op == OP_STEP
                && S->w[i].x == prex[i] && S->w[i].y == prey[i])
                it[i].action = -1;               /* hold pc, wait for an event */
        for (int i = 0; i < S->nw; i++)
            if (IS_MOVER(i)) {
                if (S->w[i].x != prex[i] || S->w[i].y != prey[i]) S->st_steps++;
                else S->st_bumps++;
            }
        #undef IS_MOVER
        for (int i = 0; i < S->nw; i++)
            if (S->w[i].alive && !S->w[i].done && it[i].action >= 0 && !it[i].walk_only
                && P->instr[it[i].action].op == OP_STEP) {
                /* a step blocked by another worker RETRIES (walkers queue --
                 * bumping through would let accumulator sweeps double-count
                 * their own tile: Dangerous Spreadsheeting row sums).
                 * Walls / no passable direction bump through as before. */
                bool attempted = it[i].tx >= 0;
                bool moved = (S->w[i].x != prex[i] || S->w[i].y != prey[i]);
                if (attempted && !moved) continue;       /* hold pc, retry */
                if (S->w[i].fresh > 0) S->w[i].fresh--;
                S->w[i].pc++;
                fall_check(S, &S->w[i]);
            }

        /* phase C: non-movement actions on the moved board. Two sub-passes:
         * hand-away actions (drop/giveto/pickup/end/...) resolve before
         * takefrom, so a cube given away this beat cannot also be taken. */
        for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive || w->done || it[i].action < 0 || it[i].walk_only) continue;
            Instr *ins = &P->instr[it[i].action];
            if (ins->op == OP_STEP) continue;
            if ((ins->op == OP_TAKEFROM) != (pass == 1)) continue;
            if (w->fresh > 0) w->fresh--;   /* one command boundary passed */
            switch (ins->op) {
                case OP_IF: {
                    if (ins->nconds == 0) {
                        fprintf(stderr, "error: unsupported condition: %s\n", ins->raw);
                        exit(3);
                    }
                    /* false: jump into the else body (past the OP_ELSE), or
                     * to the endif when there is no else */
                    if (if_true(S, ins, w)) w->pc++;
                    else w->pc = ins->target +
                             (P->instr[ins->target].op == OP_ELSE ? 1 : 0);
                    break;
                }
                case OP_ASSIGN: exec_assign(S, w, ins); break;
                case OP_PICKUP: {
                    if (!w->holding) {
                        if (ins->mem_target >= 0) {
                            int tx, ty;
                            if (mem_tile(w, ins->mem_target, &tx, &ty))
                                pickup_at(S, w, i, tx, ty);
                        } else
                        for (int k = 0; k < ins->ndirs; k++) {
                            Dir d = ins->dirs[k];
                            int nx = w->x + DX[d], ny = w->y + DY[d];
                            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                            if (pickup_at(S, w, i, nx, ny)) break;
                        }
                    }
                    break;
                }
                case OP_DROP: {
                    if (!w->holding) break;
                    Tile *t = &S->grid[w->y][w->x];
                    if (t->has_cube && t->terrain == T_FLOOR) {
                        /* standing on a cube: place the held one beside you
                         * (first free neighbor; the Data Backup swap needs it) */
                        for (int d = 0; d < 8 && t->has_cube; d++) {
                            int nx = w->x + DX[d], ny = w->y + DY[d];
                            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                            Tile *n = &S->grid[ny][nx];
                            if (n->terrain == T_FLOOR && !n->has_cube
                                && worker_at(S, nx, ny, i) < 0) t = n;
                        }
                    }
                    if (!t->has_cube && t->terrain == T_FLOOR) {
                        t->has_cube = true; t->cube = w->held; t->owner = w->held_owner;
                        w->holding = false;
                        S->drops++;
                    }
                    break;
                }
                case OP_GIVETO: {
                    if (!w->holding) break;
                    int cx[9], cy[9], nc = 0;
                    if (ins->mem_target >= 0) {
                        int nx, ny;
                        if (!mem_tile(w, ins->mem_target, &nx, &ny)) break;
                        if (abs(w->x-nx) > 1 || abs(w->y-ny) > 1) break;
                        cx[nc] = nx; cy[nc] = ny; nc++;
                    } else
                        for (int k = 0; k < ins->ndirs; k++) {
                            cx[nc] = w->x + DX[ins->dirs[k]];
                            cy[nc] = w->y + DY[ins->dirs[k]]; nc++;
                        }
                    for (int k = 0; k < nc && w->holding; k++) {
                        int nx = cx[k], ny = cy[k];
                        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                        if (S->grid[ny][nx].terrain == T_SHREDDER) {
                            feed_shredder(S, w, i, nx, ny);
                        } else {
                            int j = worker_at(S, nx, ny, i);
                            if (j >= 0 && !S->w[j].holding) {
                                S->w[j].holding = true; S->w[j].held = w->held;
                                S->w[j].held_src_x = w->held_src_x; S->w[j].held_src_y = w->held_src_y;
                                S->w[j].held_owner = w->held_owner;
                                S->w[j].fresh = 2;
                                w->holding = false;
                            }
                            /* giving to empty floor is a NO-OP (Little
                             * Exterminator 2 fires giveto at floor tiles
                             * between shredder visits and must keep the
                             * cube) */
                        }
                    }
                    break;
                }
                case OP_TAKEFROM: {
                    if (w->holding) break;
                    if (ins->mem_target >= 0) {
                        int tx, ty;
                        if (!mem_tile(w, ins->mem_target, &tx, &ty)) break;
                        if (abs(w->x-tx) > 1 || abs(w->y-ty) > 1) break;
                        if (S->grid[ty][tx].terrain == T_PRINTER) {
                            pickup_at(S, w, i, tx, ty);
                        } else {
                            int j = worker_at(S, tx, ty, i);
                            if (j >= 0 && S->w[j].holding && S->w[j].fresh == 0) {
                                w->holding = true; w->held = S->w[j].held;
                                w->held_src_x = S->w[j].held_src_x; w->held_src_y = S->w[j].held_src_y;
                                w->held_owner = S->w[j].held_owner;
                                w->fresh = 2;
                                S->w[j].holding = false;
                            }
                        }
                        break;
                    }
                    for (int k = 0; k < ins->ndirs; k++) {
                        Dir d = ins->dirs[k];
                        int nx = w->x + DX[d], ny = w->y + DY[d];
                        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                        if (S->grid[ny][nx].terrain == T_PRINTER) {
                            pickup_at(S, w, i, nx, ny);
                            break;
                        }
                        int j = worker_at(S, nx, ny, i);
                        if (j >= 0 && S->w[j].holding && S->w[j].fresh == 0) {
                            w->holding = true; w->held = S->w[j].held;
                            w->held_src_x = S->w[j].held_src_x; w->held_src_y = S->w[j].held_src_y;
                            w->held_owner = S->w[j].held_owner;
                            w->fresh = 2;
                            S->w[j].holding = false;
                            break;
                        }
                    }
                    break;
                }
                case OP_WRITE: {
                    int v;
                    if (w->holding && operand_value(S, w, &ins->op1, &v)) w->held = v;
                    break;
                }
                case OP_TELL: {
                    w->last_tell = S->beat;
                    if (S->ntellev < MAXTELLEV) {
                        TellEv *e = &S->tellev[S->ntellev++];
                        e->worker = i; e->x = w->x;
                        snprintf(e->word, sizeof e->word, "%s", ins->word);
                    }
                    /* release matching listeners in range of the target */
                    for (int j = 0; j < S->nw; j++) {
                        if (j == i) continue;
                        Worker *o = &S->w[j];
                        if (!o->alive || o->done || o->pc >= P->n) continue;
                        Instr *li = &P->instr[o->pc];
                        if (li->op != OP_LISTEN || strcmp(li->word, ins->word)) continue;
                        bool covered = false;
                        if (ins->tt_kind == 1) covered = true;
                        else if (ins->tt_kind == 2)
                            covered = (o->x == w->x + DX[ins->tt_dir] && o->y == w->y + DY[ins->tt_dir]);
                        else if (ins->tt_kind == 3) {
                            int tx, ty;
                            covered = mem_tile(w, ins->tt_mem, &tx, &ty) && o->x == tx && o->y == ty;
                        }
                        if (covered) o->heard = true;
                    }
                    break;
                }
                case OP_END:
                    w->done = true;
                    break;
                default: break;
            }
            if (ins->op != OP_STEP) S->st_items++;
            if (ins->op != OP_IF && (!w->done || ins->op == OP_END)) w->pc++;
        }
        S->st_waits += nidle;

        /* advance the actors' clocks; idlers re-check at the next completion */
        long batch_end = t, next_event = -1;
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive || w->done) continue;
            if (it[i].action >= 0) {
                long d = it[i].walk_only ? MS_STEP
                                         : cmd_duration(S, w, &P->instr[it[i].action]);
                w->next_ms = t + (d > 0 ? d : 1);
                if (w->next_ms > batch_end) batch_end = w->next_ms;
                if (!it[i].walk_only && P->instr[it[i].action].op != OP_STEP)
                    w->pend_x = w->pend_y = -1;   /* moved on to other work */
            }
            if (w->next_ms > t && (next_event < 0 || w->next_ms < next_event))
                next_event = w->next_ms;
        }
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive || w->done) continue;
            if (it[i].action < 0 && w->next_ms <= t)      /* idled this batch */
                w->next_ms = (next_event > t) ? next_event : t + 1;
        }

        S->beat++;
        rounds++;
        if (g_trace && (rounds <= 60 || rounds % 500 == 0)) {
            trace_board(S, rounds);
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                fprintf(stderr, "  w%d (%d,%d)%s%s%s pc=%d t=%ld act=[%s]\n", i, w->x, w->y,
                        w->holding?" hold":"", w->done?" done":"", w->alive?"":" dead",
                        w->pc, w->next_ms, it[i].action >= 0 ? P->instr[it[i].action].raw : "-");
            }
        }
        if (level_won(S)) { S->win_ms = batch_end; *out_rounds = rounds; return true; }
        if (S->failed)    { *out_rounds = rounds; return false; }
    }
    *out_rounds = rounds;
    if (g_trace) {
        fprintf(stderr, "-- final state (rounds=%d) --\n", rounds);
        trace_board(S, rounds);
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            fprintf(stderr, "  w%d (%d,%d)%s%s%s pc=%d pend=(%d,%d) t=%ld [%s]\n",
                    i, w->x, w->y,
                    w->holding?" hold":"", w->done?" done":"", w->alive?"":" dead",
                    w->pc, w->pend_x, w->pend_y, w->next_ms,
                    w->pc < P->n ? P->instr[w->pc].raw : "end");
        }
    }
    g_goal_dbg = g_trace;
    bool won = level_won(S);
    g_goal_dbg = false;
    return won;
}

int main(int argc, char **argv) {
    if (argc >= 2 && !strcmp(argv[1], "--trace")) { g_trace = true; argv++; argc--; }
    if (getenv("EMU_RIGHTASSOC")) g_rightassoc = true;
    if (getenv("EMU_NOCHAIN")) g_nochain = true;
    if (getenv("EMU_NOTHING_IGNORES_WORKERS")) g_nothing_ignores_workers = true;
    { const char *e;
      if ((e = getenv("EMU_MS_STEP")))    MS_STEP = atoi(e);
      if ((e = getenv("EMU_MS_ITEM")))    MS_ITEM = atoi(e);
      if ((e = getenv("EMU_MS_PRINTER"))) MS_PRINTER = atoi(e);
      if ((e = getenv("EMU_MS_SHRED")))   MS_SHRED = atoi(e);
      if ((e = getenv("EMU_MS_TELL")))    MS_TELL = atoi(e);
      if ((e = getenv("EMU_MS_IF")))      MS_IF = atoi(e);
      if ((e = getenv("EMU_MS_ASSIGN")))  MS_ASSIGN = atoi(e);
      if ((e = getenv("EMU_MS_WRITE")))   MS_WRITE = atoi(e); }
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "usage: %s [--trace] <level.lvl> <solution.txt> [trials]\n", argv[0]);
        return 1;
    }
    require_game();
    static Level L; static Program P;
    load_level(argv[1], &L);
    load_program(argv[2], &P);
    check_palette(&L, &P);

    if (L.win == G_UNKNOWN) {
        fprintf(stderr, "error: win predicate '%s' not implemented yet\n", L.goal_raw);
        return 5;
    }

    if (g_trace) {
        for (int i = 0; i < P.n; i++)
            if (P.instr[i].op == OP_IF) {
                fprintf(stderr, "if @pc%d [%s]:", i, P.instr[i].raw);
                for (int k = 0; k < P.instr[i].nconds; k++) {
                    Cond *c = &P.instr[i].conds[k];
                    fprintf(stderr, " %s{lhs k%d op%d %s t%d}",
                            c->conn==1?"AND ":c->conn==2?"OR ":"",
                            c->lhs.kind, (int)c->op,
                            c->rhs_is_type?"type":"val", (int)c->rhs_type);
                }
                fprintf(stderr, "\n");
            }
    }

    bool prog_random = false;    /* multi-dir steps / foreachdir sweeps randomize */
    for (int i = 0; i < P.n; i++)
        if ((P.instr[i].op == OP_STEP && P.instr[i].ndirs > 1)
            || P.instr[i].op == OP_FOREACH
            || (P.instr[i].op == OP_ASSIGN && P.instr[i].akind == 3)) prog_random = true;
    int trials = (argc == 4) ? atoi(argv[3]) : ((L.has_random || prog_random) ? 20 : 1);
    if (trials < 1) trials = 1;

    static Sim S;
    int wins = 0, min_r = -1, max_r = -1, first_fail = -1;
    int min_sp = -1, max_sp = -1;
    long sum_r = 0, sum_sp = 0, sum_steps = 0, sum_bumps = 0, sum_items = 0, sum_waits = 0;
    for (int t = 0; t < trials; t++) {
        sim_reset(&S, &L, (unsigned)(t + 1));
        int rounds = 0;
        bool won = run(&S, &P, &rounds);
        if (won) {
            wins++;
            if (min_r < 0 || rounds < min_r) min_r = rounds;
            if (rounds > max_r) max_r = rounds;
            sum_r += rounds;
            int sp = (int)((S.win_ms + 999) / 1000);
            if (min_sp < 0 || sp < min_sp) min_sp = sp;
            if (sp > max_sp) max_sp = sp;
            sum_sp += sp;
            sum_steps += S.st_steps; sum_bumps += S.st_bumps;
            sum_items += S.st_items; sum_waits += S.st_waits;
        } else if (first_fail < 0) first_fail = t + 1;
    }

    bool all = (wins == trials);
    printf("level   : %s (%dx%d, %d worker%s, %d cube%s)\n", L.name, L.w, L.h,
           L.nworkers, L.nworkers==1?"":"s", L.ncubes, L.ncubes==1?"":"s");
    printf("solution: %s\n", argv[2]);
    printf("size    : %d commands\n", program_size(&P));
    printf("trials  : %d, wins %d%s\n", trials, wins,
           first_fail > 0 ? " (first fail: trial seed above)" : "");
    if (wins) {
        printf("speed   : %.1f  (game seconds, avg over wins; range %d..%d)\n",
               (double)sum_sp/wins, min_sp, max_sp);
        printf("rounds  : %d..%d  (event batches)\n", min_r, max_r);
        printf("stats   : beats=%.1f steps=%.1f bumps=%.1f items=%.1f waits=%.1f (avg over wins)\n",
               (double)sum_r/wins, (double)sum_steps/wins, (double)sum_bumps/wins,
               (double)sum_items/wins, (double)sum_waits/wins);
    }
    /* a solution that wins some trials but not all is a luck-based ("retry
     * until it works") solution -- report it distinctly */
    printf("result  : %s\n", all ? "WIN" : wins ? "PROBABILISTIC" : "FAIL");
    return all ? 0 : wins ? 6 : 1;
}
