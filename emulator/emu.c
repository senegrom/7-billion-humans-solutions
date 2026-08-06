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
#include <math.h>
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
    int  settle;    /* frame until which the cube here is still being put
                       down -- visible already, but not yet liftable */
    int  settle_by; /* who is putting it down: a worker earlier in the line
                       finishes within the frame, so someone later in the
                       line can lift that same frame; someone earlier has
                       already had their turn and must wait one more */
} Tile;

typedef enum { CB_FIXED, CB_RAND, CB_RANDU } CubeMode;
typedef struct { int x, y; CubeMode mode; int value; } CubeDef;

/* a memory slot holds nothing, a number, or a remembered tile; tiles found by
 * `nearest <type>` remember the type so a stale reference can re-resolve */
typedef enum { MV_NOTHING, MV_NUM, MV_TILE, MV_CUBEREF } MemKind;
/* wref: when the slot was filled by `nearest worker` it names that PERSON, not
 * the square they were standing on, so it keeps their index and follows them. */
typedef struct { MemKind k; int num, x, y; int ntype; /* CmpKind or -1 */
                 int wref; /* worker index, or -1 */
                 bool fedir; /* filled by a foreachdir sweep: such a slot reads
                                its square the way a pointed direction does --
                                a cube held aloft by the worker standing there
                                still answers -- where a remembered square
                                reads only what lies on the floor */ } MemVal;

enum { NMEM = 4, MAXFOREACH = 64, WORDLEN = 32 };

typedef struct {
    int  x, y;
    bool holding;
    int  held;
    int  held_id;                 /* identity of the held cube (0 = none) */
    int  held_src_x, held_src_y;  /* floor tile the held cube came from (-1 = printed) */
    int  held_owner;              /* printing worker of the held cube (-1 = level cube) */
    int  pc;
    bool alive;     /* still on the board (false = destroyed / fell) */
    bool done;      /* program finished (END or ran off the end); stays on board */
    bool exited;    /* left via a hole */
    int  exit_x, exit_y, exit_beat;
    int  tgt_x, tgt_y;   /* aligned-hole-exit target (or -1) */
    MemVal mem[NMEM];
    int  fe_idx[MAXFOREACH];      /* foreachdir loop positions */
    unsigned char fe_ord[MAXFOREACH][8];   /* per-sweep direction order */
    bool listening;               /* parked on a listenfor */
    int  heard;                   /* ticks a word spoken to us still rings */
    char heard_word[WORDLEN];     /* which word it was */
    bool greeted;                 /* a word has reached this worker's ear */
    int  printed, fed;            /* printer takes / shredder feeds by this worker */
    int  last_tell;               /* beat of most recent tell (-1 = never) */
    int  fresh;     /* a just-acquired cube can't be taken from us until our
                       first decision about it completes (2 = acquiring,
                       1 = deciding, 0 = settled): Big Data's chains steal
                       only cubes their holder chose to keep */
    /* the movement machinery: the worker glides between tile centres at a
       fixed speed, so diagonal steps take sqrt(2)x longer and congas wave
       forward.  (x,y) is the LOGICAL tile, which only updates once the walk
       arrives; (fx,fy) is the smooth position in between. */
    double fx, fy;  /* smooth position in tile units (centre = integer coords) */
    int  wtx, wty;  /* tile being walked into (-1 = standing still) */
    bool wsingle;   /* a plain one-tile dir-step: advance pc on arrival */
    bool wflex;     /* the walk can be re-aimed: it either named several
                       directions and picked one, or is chasing an object */
    int  busy;      /* frames left on a non-move command (0 = free) */
    int  wprog, wtot; /* frames elapsed / total for the walk in progress */
    int  wintx, winty; /* tile a blocked travel walk still means to enter */
    bool wowned;       /* the tile being walked into has already been taken */
    /* the per-worker command timeline for the event-queue scheduler */
    struct { unsigned char id; float t; } evq[24];
    int  evn, evcur;
    float animms;      /* remaining time of the animation being played */
    bool fsusp;        /* suspended: the stepper may not dispatch */
    bool fready;       /* a node is ready to dispatch this frame */
    bool wsettle;      /* the walk under way belongs to a step command: on
                          landing the body settles into the square first and
                          the program follows a frame later, not right away */
    double fsx, fsy;   /* where the body was when that tile was taken */
    /* an item action aimed through a DIRECTION at a machine reads that
       direction once, where the command was taken up, and the errand then
       belongs to THAT machine however far the walk drifts.  The claim is
       remembered here until the program moves to another command. */
    int  errx, erry;   /* machine an item errand is bound to (-1 = none) */
    int  err_pc;       /* command the binding belongs to */
    int  err_t0;       /* frame the binding was made on: the look a pointed
                          takefrom pays runs from here, overlapping any wait
                          in the queue (-1 = not yet stamped) */
    int  smem_pc;      /* step-to-person command that has already strode
                          (-1 = none): its completion beside them is free */
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
    G_BINARY_COUNTER, G_DECIMAL_COUNTER, G_DECIMAL_DOUBLER,
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
    /* counting-machine furniture (sensors under the starting cubes, big red
     * button); place order leftmost = most significant */
    int     sw_x[8], sw_y[8], nsw;
    int     button_x, button_y;
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
               C_SOMETHING, C_SWITCH, C_BUTTON } CmpKind;
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
    if (!strcmp(kw,"binary_counter"))      return G_BINARY_COUNTER;
    if (!strcmp(kw,"decimal_counter"))     return G_DECIMAL_COUNTER;
    if (!strcmp(kw,"decimal_doubler"))     return G_DECIMAL_DOUBLER;
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
    if (L->win == G_BINARY_COUNTER || L->win == G_DECIMAL_COUNTER
        || L->win == G_DECIMAL_DOUBLER) {
        /* the machine's layout is fixed relative to the starting digit cubes:
         * each green sensor sits one row below its cube (leftmost = most
         * significant digit), and the big red button one tile below-right of
         * the rightmost sensor */
        L->nsw = 0;
        int rightmost = -1, cube_row = -1;
        for (int i = 0; i < L->ncubes && L->nsw < 8; i++) {
            L->sw_x[L->nsw] = L->cubes[i].x;
            L->sw_y[L->nsw] = L->cubes[i].y + 1;
            L->nsw++;
            if (L->cubes[i].x > rightmost) { rightmost = L->cubes[i].x; cube_row = L->cubes[i].y; }
        }
        for (int i = 0; i < L->nsw; i++)          /* sort sensors by x */
            for (int j = i + 1; j < L->nsw; j++)
                if (L->sw_x[j] < L->sw_x[i]) {
                    int t = L->sw_x[i]; L->sw_x[i] = L->sw_x[j]; L->sw_x[j] = t;
                    t = L->sw_y[i]; L->sw_y[i] = L->sw_y[j]; L->sw_y[j] = t;
                }
        L->button_x = rightmost + 1;
        L->button_y = cube_row + 2;
    }
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
    if (!strcmp(t,"switch"))    { *out = C_SWITCH; return 1; }
    if (!strcmp(t,"button"))    { *out = C_BUTTON; return 1; }
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
    if (!strcmp(t, "[blank]"))           { o->kind = 0; o->num = 0; return true; }
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
                && (o[0]=='+'||o[0]=='-'||o[0]=='*'||o[0]=='/'||o[0]=='x') && !o[1])
                ins->calcop = o[0] == 'x' ? '*' : o[0];   /* the editor writes "x" */
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
typedef struct { int value, src_x, src_y, shr_x, shr_y, worker, id; } ShredEv;
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
    int     cube_id[MAXH][MAXW];      /* identity of the floor cube (0 none) */
    int     next_cube_id;
    bool    label_tile[MAXH][MAXW];   /* labels_explode rules */
    /* per-trial snapshot of the initial cube placement */
    int     icx[MAXCUBES], icy[MAXCUBES], icv[MAXCUBES];
    int     ic_id[MAXCUBES];         /* cube identity of each initial cube */
    int     nic;
    ShredEv shrev[MAXSHREV]; int nshrev;
    TellEv  tellev[MAXTELLEV]; int ntellev;
    int     glory_x, glory_y;    /* G_GLORY_DIVE target hole */
    unsigned char region[MAXH][MAXW]; /* connected floor component id (0 none);
                                         machines/holes join adjacent floor */
    bool    reach[MAXH][MAXW];   /* floor reachable from worker spawns (the room;
                                    unlisted tiles outside the walls read as
                                    floor in sparse levels and must not count) */
    int     dist_door[MAXH][MAXW];   /* G_DISTANCES_FROM_DOOR (-1 unreachable) */
    int     ic_group[MAXCUBES];      /* G_FLOWER/SHRED_MAX groups (8-connected) */
    int     ngroups;
    bool    door_exit;           /* the door acts as a walk-in exit */
    long    mach_busy[MAXH][MAXW]; /* machine mid-cycle until this time: one
                                      customer at a time (the printer queue) */
    long    mach_clear[MAXH][MAXW]; /* when the last customer stepped out of
                                       the front square (0 = never served) */
    long    press_done[MAXH][MAXW]; /* when the sheet now on the press is
                                       finished (0 = the press stands idle) */
    int     prints_at[MAXH][MAXW]; /* dispense count per printer tile */
    int     print_next[MAXH][MAXW]; /* the value each printer will serve next:
                                       a fresh printer's first sheet is always
                                       0, and the roll happens as one is taken
                                       -- for the sheet after it */
    int     hist[130], hist_n;   /* counting-machine display history */
    bool    ctr_down;            /* ...and whether its button was down last frame */
    int     feeds_this_beat;
    int     beat;
    long    win_ms;              /* frame the win state was reached on */
    long    st_items;            /* non-step effects fired (texture stat) */
    unsigned rng;                /* the harness's dice: level fabrication only */
    unsigned grng;               /* the game's own dice: step picks and prints */
} Sim;

/* Per-command durations, calibrated against recorded community speeds.
 * The clock runs in FRAMES -- the game's native unit; every command
 * duration is an integer frame count.  Frame units keep same-frame
 * workers exactly simultaneous and make the idle fallback (t+1) advance
 * a full frame. */
static int MS_STEP = 21, MS_PRINTER = 72, MS_SHRED = 45,
           MS_TELL = 42, MS_WRITE = 57;

static unsigned rnd(Sim *S) { S->rng = S->rng * 1664525u + 1013904223u; return S->rng >> 8; }

/* The dice the game itself rolls -- for a step that names several
 * directions and for the value a printer prints.  It is the game's own
 * generator: xorshift over a 32-bit state that starts, on a fresh machine,
 * from one fixed power-on seed.  In the real game the state simply carries
 * on from wherever the menus and earlier levels left it, so each trial
 * seeds it differently; the first trial uses the power-on seed itself. */
static unsigned game_rnd(Sim *S) {
    unsigned x = S->grng ? S->grng : 0xABAB1981u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return S->grng = x;
}
static bool cube_locate(Sim *S, int id, int *tx, int *ty);

static void sim_reset(Sim *S, Level *L, unsigned seed) {
    memset(S, 0, sizeof *S);
    S->L = L;
    S->rng = seed * 2654435761u + 12345u;
    /* trials are numbered from 1, so it is the FIRST trial that plays on
     * the game's power-on seed; later trials sample other histories */
    S->grng = seed == 1 ? 0xABAB1981u : S->rng | 1u;
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++)
            S->grid[y][x] =
                (Tile){ L->terr[y][x], L->goalpad[y][x], false, 0, -1, 0, 0 };

    /* per-game randomizer specials (the game rolls these from its seed):
     * Terrain Leveler picks its value range per game -- half the games run
     * 0..6, a sixth run 0..10, the rest use the level's full range; Seek
     * and Destroy 3 lifts the whole range onto a random floor so the room
     * minimum varies */
    int rmax = L->randmax, vfloor = 0;
    if (L->win == G_CUBES_AVG) {
        if (seed % 2 == 0)      rmax = 6;
        else if (seed % 3 == 0) rmax = 10;
    }
    if (L->win == G_SHRED_MIN_ROOM) vfloor = (int)(seed % 30u);

    /* distinct-value pool for CB_RANDU cubes */
    int pool[10000]; int pn = rmax + 1;
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
        if (c->mode == CB_RANDU) {
            if (vfloor > 0)                      /* floor-lifted independent draw */
                v = vfloor + (int)(rnd(S) % (unsigned)(rmax - vfloor + 1));
            else { v = pool[pi]; pi = (pi + 1) % pn; }
        }
        S->grid[c->y][c->x].has_cube = true;
        S->grid[c->y][c->x].cube = v;
        S->cube_id[c->y][c->x] = ++S->next_cube_id;
        if (((L->rules & R_LABELS_EXPLODE) && c->mode == CB_FIXED)
         || ((L->rules & R_LABELS_EXPLODE_NONZERO) && c->mode == CB_FIXED && c->value != 0))
            S->label_tile[c->y][c->x] = true;
        S->icx[S->nic] = c->x; S->icy[S->nic] = c->y; S->icv[S->nic] = v;
        S->ic_id[S->nic] = S->next_cube_id; S->nic++;
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

    /* connected floor components: nearest only binds things in the
     * worker's own region (Community Training Day's students must not
     * sense the instructor's caged machines through the pit ring) */
    {
        static int q[MAXW*MAXH];
        unsigned char rid = 0;
        for (int sy2 = 0; sy2 < L->h; sy2++)
            for (int sx2 = 0; sx2 < L->w; sx2++) {
                if (L->terr[sy2][sx2] != T_FLOOR || S->region[sy2][sx2]) continue;
                rid++;
                int head = 0, tail = 0;
                S->region[sy2][sx2] = rid;
                q[tail++] = sy2 * MAXW + sx2;
                while (head < tail) {
                    int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
                    for (int d = 0; d < 8; d++) {
                        int nx = cx + DX[d], ny = cy + DY[d];
                        if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
                        if (S->region[ny][nx] || L->terr[ny][nx] != T_FLOOR) continue;
                        S->region[ny][nx] = rid;
                        q[tail++] = ny * MAXW + nx;
                    }
                }
            }
        /* machines and holes belong to the region they border */
        for (int y = 0; y < L->h; y++)
            for (int x = 0; x < L->w; x++) {
                if (L->terr[y][x] == T_FLOOR || L->terr[y][x] == T_WALL) continue;
                for (int d = 0; d < 8 && !S->region[y][x]; d++) {
                    int nx = x + DX[d], ny = y + DY[d];
                    if (nx >= 0 && ny >= 0 && nx < L->w && ny < L->h
                        && L->terr[ny][nx] == T_FLOOR)
                        S->region[y][x] = S->region[ny][nx];
                }
            }
    }

    if (L->win == G_DISTANCES_FROM_DOOR && L->door_x >= 0) {
        /* the boss occupies a 2x2 pocket whose base tile is the door marker;
         * a cube's expected number = walking steps (8-dir over floor, not
         * through the boss) to the straight-line-nearest of his four tiles */
        static int cd[4][MAXH][MAXW];
        int corner[4][2] = {
            { L->door_x,     L->door_y     }, { L->door_x - 1, L->door_y     },
            { L->door_x,     L->door_y - 1 }, { L->door_x - 1, L->door_y - 1 },
        };
        static int q[MAXW*MAXH];
        for (int c = 0; c < 4; c++) {
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) cd[c][y][x] = -1;
            int head = 0, tail = 0;
            int sx2 = corner[c][0], sy2 = corner[c][1];
            if (sx2 < 0 || sy2 < 0) continue;
            cd[c][sy2][sx2] = 0;
            q[tail++] = sy2 * MAXW + sx2;
            while (head < tail) {
                int cur = q[head++], cx = cur % MAXW, cy = cur / MAXW;
                for (int d = 0; d < 8; d++) {
                    int nx = cx + DX[d], ny = cy + DY[d];
                    if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
                    if (cd[c][ny][nx] >= 0 || L->terr[ny][nx] != T_FLOOR) continue;
                    bool boss = false;
                    for (int b = 0; b < 4; b++)
                        if (nx == corner[b][0] && ny == corner[b][1]) boss = true;
                    if (boss) continue;
                    cd[c][ny][nx] = cd[c][cy][cx] + 1;
                    q[tail++] = ny * MAXW + nx;
                }
            }
        }
        for (int y = 0; y < L->h; y++)
            for (int x = 0; x < L->w; x++) {
                int best = 0x7fffffff, pick = -1;
                for (int c = 0; c < 4; c++) {
                    int ddx = corner[c][0] - x, ddy = corner[c][1] - y;
                    int d2 = ddx*ddx + ddy*ddy;
                    if (d2 < best) { best = d2; pick = c; }
                }
                S->dist_door[y][x] = pick >= 0 ? cd[pick][y][x] : -1;
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
        w->fx = w->x; w->fy = w->y; w->wtx = w->wty = -1;
        w->wintx = w->winty = -1;
        w->alive = true;
        w->exit_x = w->exit_y = -1;
        w->tgt_x = w->tgt_y = -1;
        w->held_src_x = w->held_src_y = -1;
        w->held_owner = -1;
        w->last_tell = -1;
        w->errx = w->erry = -1; w->err_pc = -1; w->err_t0 = -1;
        w->smem_pc = -1;
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

/* A word spoken to someone reaches them whatever they happen to be doing --
 * it waits in the ear rather than having to be caught in the act of
 * listening -- but only for a tenth of a second, after which it is gone.  So
 * a listener still has to be about ready for it; they just need not be
 * standing at the listen command in the very instant it is spoken. */
#define MS_EARSHOT 7

/* Where SENSES see a worker.  Movement bookkeeping gives a walker its
 * destination tile the moment it sets off (that is what blocks other
 * movers), but eyes track the body: a walking worker is seen on the tile
 * its body is nearest, so it crosses over to the destination only at the
 * midpoint of the step.  Senses, the counting-machine button and the win
 * checks all read this; only movement reads the claimed tile.
 *
 * Everyone in a frame sees the SAME picture: positions are snapshotted
 * once at the top of the frame, so a worker early in the update order is
 * not seen a frame fresher than one later in it. */
static int  g_snap_n = -1;                       /* <0 = snapshot off */
static int  g_snap_x[MAXWORKERS], g_snap_y[MAXWORKERS];
static int body_tx(const Worker *w) { return w->wtx >= 0 ? (int)lround(w->fx) : w->x; }
static int body_ty(const Worker *w) { return w->wtx >= 0 ? (int)lround(w->fy) : w->y; }
static int seen_tx(const Sim *S, int i) {
    return (g_snap_n > i) ? g_snap_x[i] : body_tx(&S->w[i]);
}
static int seen_ty(const Sim *S, int i) {
    return (g_snap_n > i) ? g_snap_y[i] : body_ty(&S->w[i]);
}

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
                if (&S->w[i] != self && S->w[i].alive
                    && seen_tx(S, i) == x && seen_ty(S, i) == y)
                    return true;
            return false;
        case C_NOTHING: {
            if (t != T_FLOOR || S->grid[y][x].has_cube) return false;
            for (int i = 0; i < S->nw; i++)
                if (&S->w[i] != self && S->w[i].alive
                    && seen_tx(S, i) == x && seen_ty(S, i) == y)
                    return false;
            return true;
        }
        case C_SOMETHING:
            return !tile_contains(S, x, y, self, C_NOTHING);
        case C_SWITCH:
            /* the whole pad bank reads as "switch" -- the button included
             * (workers park on their pad because "c != switch" goes false) */
            if (L->nsw > 0 && x == L->button_x && y == L->button_y) return true;
            for (int i = 0; i < L->nsw; i++)
                if (L->sw_x[i] == x && L->sw_y[i] == y) return true;
            return false;
        case C_BUTTON:
            return L->nsw > 0 && x == L->button_x && y == L->button_y;
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
        if (&S->w[i] != self && S->w[i].alive
            && seen_tx(S, i) == x && seen_ty(S, i) == y) {
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
 * on the floor of that square (Neighborly Sweeper tip).  A foreachdir loop
 * variable is a pointed direction, not a remembered square: its read still
 * answers while the neighbor holds their cube in the air -- which is what
 * lets ten sweepers count each other's cubes mid-shuffle without the tally
 * going stale. */
static bool operand_value(Sim *S, Worker *w, const Operand *o, int *out) {
    switch (o->kind) {
        case 0: *out = o->num; return true;
        case 1: return value_at2(S, body_tx(w) + DX[o->dir], body_ty(w) + DY[o->dir],
                                 w, true, out);
        case 3: if (w->holding) { *out = w->held; return true; } return false;
        case 2: {
            MemVal *m = &w->mem[o->mem];
            if (m->k == MV_NUM)  { *out = m->num; return true; }
            /* a slot that remembers a PERSON asks THEM for a number, and what
             * a person has to show is whatever is in their hands -- wherever
             * they have wandered off to since.  (A slot that remembers a
             * SQUARE still reads only what lies on that floor.) */
            if (m->k == MV_TILE && m->wref >= 0) {
                Worker *o2 = &S->w[m->wref];
                if (!o2->alive || !o2->holding) return false;
                *out = o2->held; return true;
            }
            if (m->k == MV_TILE) return value_at2(S, m->x, m->y, w, m->fedir, out);
            if (m->k == MV_CUBEREF) {
                for (int i = 0; i < S->nw; i++)
                    if (S->w[i].alive && S->w[i].holding && S->w[i].held_id == m->num) {
                        *out = S->w[i].held; return true;
                    }
                for (int y = 0; y < S->L->h; y++)
                    for (int x = 0; x < S->L->w; x++)
                        if (S->cube_id[y][x] == m->num) {
                            *out = S->grid[y][x].cube; return true;
                        }
                return false;
            }
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
            eq = tile_contains(S, body_tx(w) + DX[c->lhs.dir],
                               body_ty(w) + DY[c->lhs.dir], w, c->rhs_type);
        else if (c->lhs.kind == 2) {
            MemVal *m = &w->mem[c->lhs.mem];
            if (m->k == MV_TILE)         eq = tile_contains(S, m->x, m->y, w, c->rhs_type);
            else if (m->k == MV_CUBEREF) {
                int tx, ty;
                bool exists = cube_locate(S, m->num, &tx, &ty);
                if (c->rhs_type == C_DATACUBE || c->rhs_type == C_SOMETHING)
                    eq = exists;
                else if (c->rhs_type == C_NOTHING)
                    eq = !exists;
                else eq = false;
            }
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
            /* two slots that remember PEOPLE are asking whether it is the same
             * person, and a person who has walked on is still that person --
             * so where they were first noticed does not enter into it */
            bool eq = (ma->wref >= 0 || mb->wref >= 0)
                        ? (ma->wref == mb->wref)
                        : (ma->x == mb->x && ma->y == mb->y);
            return (c->op == O_NE) ? !eq : eq;
        }
        if (ma->k == MV_CUBEREF && mb->k == MV_CUBEREF) {
            bool eq = (ma->num == mb->num);        /* same remembered cube */
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
    /* an untouched mem slot reads as 0 against a number literal, matching
     * calc's accumulator coercion (Printing Etiquette counts "mem2 < 5"
     * before ever setting mem2) */
    if (!ha && c->lhs.kind == 2 && w->mem[c->lhs.mem].k == MV_NOTHING
        && hb && c->rhs.kind == 0) { a = 0; ha = true; }
    if (!hb && c->rhs.kind == 2 && w->mem[c->rhs.mem].k == MV_NOTHING
        && ha && c->lhs.kind == 0) { b = 0; hb = true; }
    /* A worker asked to hold something up against ITSELF always agrees: the
     * same square, or its own hands, compared with the very same thing is a
     * match whether or not there is anything there to look at.  (Two DIFFERENT
     * empty things are not therefore alike -- an empty square is not the same
     * as a memory of nothing -- so this only covers the identical operand.) */
    if (!ha && !hb && c->lhs.kind == c->rhs.kind
        && (c->op == O_EQ || c->op == O_NE)) {
        bool same = false;
        switch (c->lhs.kind) {
            case 0: same = (c->lhs.num == c->rhs.num); break;
            case 1: same = (c->lhs.dir == c->rhs.dir); break;
            case 2: same = (c->lhs.mem == c->rhs.mem); break;
            case 3: same = true; break;                  /* myitem vs myitem */
            default: same = false; break;
        }
        if (same) return c->op == O_EQ;
    }
    /* A comparison with nothing on one side is simply not true, for != just
     * as for ==: a glance at a bare square during the instant a neighbour
     * has lifted its cube reads as no answer, not as "different from 0" --
     * which is what lets a counting loop poll `nw != 0` across a column
     * being rebuilt without firing on the gap. */
    if (!ha || !hb) return false;
    return num_cmp(c->op, a, b);
}

/* Conditions fold left to right: A op B, then op C, then op D -- the way
 * the game's own editor brackets them. */
static bool if_true(Sim *S, Instr *ins, Worker *w) {
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

static bool walkable(Sim *S, int x, int y) {
    if (x < 0 || y < 0 || x >= S->L->w || y >= S->L->h) return false;
    if (S->door_exit && x == S->L->door_x && y == S->L->door_y) return true;
    Terrain t = S->grid[y][x].terrain;
    return t == T_FLOOR || t == T_HOLE;
}

/* Routing, as the game does it: a best-first search over floor tiles scored
 * f = g + 10 * manhattan-to-goal.  Three details of its arithmetic are what
 * shape crowds, and all three are deliberate here:
 *
 *  - Every interior step costs the same, diagonal or not, so routes come out
 *    the shape a plain breadth-first search would give.  (The game means to
 *    charge more for a diagonal and misses, testing the destination's own
 *    coordinates rather than the direction taken.  Only the outermost row and
 *    column are ever charged the lower price.)
 *  - A square someone is STANDING on costs a heavy toll on top -- enough to
 *    walk three tiles around them, not enough to treat them as a wall.
 *    Someone already under way costs nothing, so a queue forming at a machine
 *    does not shove away the people joining it.
 *  - The heuristic outweighs the step cost, which makes the search greedy and
 *    means the route returned is not always the shortest one.
 *
 * Ties go to whichever square was reached first, so the order neighbours are
 * offered in settles which of several equal routes is taken -- and that
 * settles who reaches a contested tile first.
 *
 * Returns the first-step direction, -2 if already at the goal, -1 if there is
 * no way through.  adjacent_ok: standing anywhere Chebyshev-adjacent to
 * (tx,ty) counts as arrived; otherwise the goal is that tile itself, which may
 * be a hole -- diving in is allowed as the final step. */
enum { RT_STEP = 14, RT_EDGE = 10, RT_STAND = 40, RT_HEUR = 10, RT_INF = 0x7FFFFFFF };

static int path_step(Sim *S, const Worker *self, int tx, int ty,
                     bool adjacent_ok) {
    Level *L = S->L;
    #define ISGOAL(X,Y) (adjacent_ok ? (abs((X)-tx)<=1 && abs((Y)-ty)<=1) \
                                     : ((X)==tx && (Y)==ty))
    if (ISGOAL(self->x, self->y)) return -2;
    /* The toll board.  A square costs extra only while its holder has SETTLED
     * on it -- arrived somewhere and stopped.  Being stuck behind a jam is not
     * settling: someone waiting their turn is still on their way, so a queue
     * stays permeable and routing runs straight through it rather than around.
     * (Only arriving clears the moving flag, and it raises the settled flag in
     * the same breath.) */
    static bool stood[MAXH][MAXW];
    memset(stood, 0, sizeof stood);
    int me = (int)(self - S->w);
    for (int i = 0; i < S->nw; i++) {
        if (i == me || !S->w[i].alive) continue;
        if (S->w[i].wtx >= 0 || S->w[i].wintx >= 0) continue;
        int bx = body_tx(&S->w[i]), by = body_ty(&S->w[i]);
        if (bx >= 0 && by >= 0 && bx < MAXW && by < MAXH) stood[by][bx] = true;
    }
    static int g[MAXH][MAXW], f[MAXH][MAXW], from[MAXH][MAXW];
    static bool closed[MAXH][MAXW], opened[MAXH][MAXW];
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++) {
            from[y][x] = -9; closed[y][x] = false; opened[y][x] = false;
        }
    /* neighbours are offered x before y, positive before negative, cardinals
     * before diagonals -- the order that breaks ties between equal routes */
    static const int NEIGH[8] = { D_E, D_W, D_S, D_N, D_SE, D_SW, D_NE, D_NW };
    static int openq[MAXW * MAXH * 4];
    int on = 0, goal = -1;
    g[self->y][self->x] = 0;
    f[self->y][self->x] = 0;
    from[self->y][self->x] = -2;
    opened[self->y][self->x] = true;
    openq[on++] = self->y * MAXW + self->x;
    while (on > 0) {
        int bi = 0, best = RT_INF;
        for (int k = 0; k < on; k++) {          /* earliest entry wins a tie */
            int c = openq[k];
            if (f[c / MAXW][c % MAXW] < best) { best = f[c / MAXW][c % MAXW]; bi = k; }
        }
        int cur = openq[bi];
        memmove(&openq[bi], &openq[bi + 1], (size_t)(on - bi - 1) * sizeof openq[0]);
        on--;
        int cx = cur % MAXW, cy = cur / MAXW;
        if (closed[cy][cx]) continue;
        closed[cy][cx] = true; opened[cy][cx] = false;
        if (ISGOAL(cx, cy)) { goal = cur; break; }
        for (int k = 0; k < 8; k++) {
            int d = NEIGH[k];
            int nx = cx + DX[d], ny = cy + DY[d];
            if (nx < 0 || ny < 0 || nx >= L->w || ny >= L->h) continue;
            if (closed[ny][nx]) continue;
            bool target_tile = (!adjacent_ok && nx == tx && ny == ty);
            Terrain t = S->grid[ny][nx].terrain;
            bool pass = (t == T_FLOOR) || (t == T_HOLE && target_tile);
            if (!pass) continue;
            int ng = g[cy][cx] + ((nx == 0 || ny == 0) ? RT_EDGE : RT_STEP)
                   + (stood[ny][nx] ? RT_STAND : 0);
            int nf = ng + RT_HEUR * (abs(nx - tx) + abs(ny - ty));
            if (opened[ny][nx] && f[ny][nx] <= nf) continue;
            g[ny][nx] = ng; f[ny][nx] = nf; from[ny][nx] = d;
            /* one entry per square is enough: a re-reached square keeps the
             * place it already had, and the scan would find that one first */
            if (!opened[ny][nx]) {
                opened[ny][nx] = true;
                if (on < (int)(sizeof openq / sizeof openq[0])) openq[on++] = ny * MAXW + nx;
            }
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

/* one movement step toward a target.  There is no second opinion: the toll
 * already prices standing bodies into the route, and walking at one anyway is
 * a legitimate outcome -- the walk itself then sorts out the right of way. */
static int route_step(Sim *S, const Worker *self, int tx, int ty, bool adjacent_ok) {
    return path_step(S, self, tx, ty, adjacent_ok);
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

/* How far a thing counts as being, for `nearest`.  The measurement is a plain
 * straight line between the two bodies -- but before the subtraction the game
 * nudges the far body half a unit down the y axis, and a tile is forty-five
 * units across.  Half a unit is far too little to reorder two things at
 * different distances and exactly enough to settle two at the same distance,
 * so the nudge is not really about distance at all: it is the tie-break.  Of
 * two things equally far off, the more northerly one is taken.
 *
 * Written out over whole tiles (and dropping the quarter that every candidate
 * carries alike) the ranking value is exact in integers. */
static long near_key(int ux, int uy) {
    return 2025L * (ux * ux + uy * uy) + 45L * uy;
}

/* Where a body counts as being, for `nearest`.  A worker in mid-stride does not
 * cross a square in a dead-straight line when a floor cube sits on its path: it
 * swings about a third of a tile to the side to round the cube, then straightens
 * again.  That swing is too small to change which square the body counts as
 * standing on -- so it never shows up in what the conditions read -- but
 * `nearest` compares raw distances, and a third of a tile is exactly enough to
 * decide which of two people is nearer when they are otherwise a hair apart.
 * A worker that is standing still, or gliding past open floor, is measured at
 * its square, the same tidy board the conditions see. */
#define BOW_SIDESTEP 0.3
static void nearest_body(Sim *S, Worker *o, double *px, double *py) {
    if (o->wtx < 0) { *px = o->x; *py = o->y; return; }          /* standing still */
    Level *L = S->L;
    int sxt = (int)lround(o->fsx), syt = (int)lround(o->fsy);
    bool cube =
        (sxt >= 0 && syt >= 0 && syt < L->h && sxt < L->w && S->grid[syt][sxt].has_cube) ||
        (o->wtx >= 0 && o->wty >= 0 && o->wty < L->h && o->wtx < L->w && S->grid[o->wty][o->wtx].has_cube);
    if (!cube) { *px = lround(o->fx); *py = lround(o->fy); return; } /* plain glide -> its square */
    /* rounding a cube: the true gliding position, pushed to the (-dy,dx) side of
     * travel by a third of a tile */
    double bx = o->fx, by = o->fy;
    double tx = o->wtx - o->fsx, ty = o->wty - o->fsy, len = sqrt(tx * tx + ty * ty);
    if (len > 1e-6) { bx += BOW_SIDESTEP * (-ty / len); by += BOW_SIDESTEP * (tx / len); }
    *px = bx; *py = by;
}

/* nearest thing of a type; false if none.  The caller's own tile counts (Seek
 * and Destroy remembers the cube underfoot that way).
 *
 * Nothing here walks anywhere -- it is a measurement, not a journey -- so a
 * wall standing between the two hides nothing, and a machine is measured from
 * the square in FRONT of it rather than from the machine itself, the same near
 * side the walk to it aims for. */
static int g_near_who;      /* index of the person `nearest` last settled on */
static bool find_nearest(Sim *S, Worker *w, CmpKind type, int *ox, int *oy) {
    Level *L = S->L;
    long best = 0; bool have = false; int bx = 0, by = 0;
    g_near_who = -1;
    /* Measured from where the BODIES actually are.  This is not the tidied-up
     * board the conditions read: it is a tape measure held between two people
     * mid-stride, so someone half way across a square has already half left
     * the one behind them. */
    int sx = body_tx(w), sy = body_ty(w);
    unsigned char myreg = S->region[w->y][w->x];
    if (type == C_PERSON) {
        /* people are compared by the smooth distance between their bodies, so
         * the cube sidestep above can tip a near-tie; equal distances settle on
         * the earlier-born (and, if that is equal too, the more northerly),
         * exactly as the whole-tile ranking did.  For a body measured at its
         * square this value is the very integer the tile ranking produced. */
        double sxf, syf; nearest_body(S, w, &sxf, &syf);
        double bestk = 0;
        for (int i = 0; i < S->nw; i++) {
            Worker *o = &S->w[i];
            if (o == w || !o->alive) continue;
            if (S->region[o->y][o->x] != myreg) continue;
            double pxf, pyf; nearest_body(S, o, &pxf, &pyf);
            double dx = pxf - sxf, dy = pyf - syf;
            double k = 2025.0 * (dx * dx + dy * dy) + 45.0 * dy;
            if (!have || k < bestk) {
                bestk = k; have = true;
                bx = (int)lround(pxf); by = (int)lround(pyf); g_near_who = i;
            }
        }
        if (!have) return false;
        *ox = bx; *oy = by;
        return true;
    }
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++) {
            Terrain t = L->terr[y][x];
            /* Things standing on the floor of another room do not answer: a
             * worker sealed into one room does not fetch from the next.  A
             * WALL is not in any room -- it is what the rooms are made of --
             * so that test cannot be asked of it, and asking it anyway was
             * telling every worker there were no walls anywhere at all. */
            if (t != T_WALL && S->region[y][x] != myreg) continue;
            if (!nearest_matches(S, w, type, x, y)) continue;
            int my = (t == T_SHREDDER || t == T_PRINTER) ? y - 1 : y;
            long k = near_key(x - sx, my - sy);
            if (!have || k < best) { best = k; have = true; bx = x; by = y; }
        }
    if (!have) return false;
    *ox = bx; *oy = by;
    return true;
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
            if (g_goal_dbg)
                fprintf(stderr, "shredded %d of %d\n", S->shredded, L->goal_a);
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
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || S->w[i].printed < L->goal_a || S->w[i].fed < L->goal_a)
                    return false;
            return true;
        case G_PRINTSHRED_QUIET:
            /* the game counts machines, not people: every shredder must have
             * eaten exactly goal_a cubes (whose they were doesn't matter --
             * a worker may well feed their cubicle-number token through) */
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (L->terr[y][x] != T_SHREDDER) continue;
                    int n = 0;
                    for (int e = 0; e < S->nshrev; e++)
                        if (S->shrev[e].shr_x == x && S->shrev[e].shr_y == y) n++;
                    if (n != L->goal_a) return false;
                }
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
            /* the game only demands the pattern tiles be COVERED -- every
             * room tile of the seed cube's parity needs a cube (the printer
             * excuses its own tile); what lands on the other color is
             * nobody's business */
            int par = (S->icx[0] + S->icy[0]) & 1;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->reach[y][x]) continue;
                    if (((x + y) & 1) != par) continue;
                    if (!S->grid[y][x].has_cube) return false;
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
            /* the game can't see whose label is whose: it wants 25 cubes on
             * the floor, nobody holding, five of each value 1..5, and every
             * worker's print counter at exactly five */
            for (int i = 0; i < S->nw; i++)
                if (S->w[i].holding || S->w[i].printed != 5) return false;
            int hist[6] = { 0 }, n = 0;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    Tile *t = &S->grid[y][x];
                    if (!t->has_cube) continue;
                    n++;
                    if (t->cube >= 1 && t->cube <= 5) hist[t->cube]++;
                }
            if (n != S->nw * 5) return false;
            for (int v = 1; v <= 5; v++) if (hist[v] != 5) return false;
            return true;
        }
        case G_DECRYPT_LEFT_EXIT: {
            for (int k = 0; k < S->nic; k++) {
                int x = S->icx[k] - S->icv[k], y = S->icy[k];
                if (x < 0 || !S->grid[y][x].has_cube) return false;
            }
            if (floor_cube_count(S) != S->nic) return false;
            /* everyone must be gone -- pit exit or otherwise perished
             * (the community solution culls some workers via div-zero) */
            for (int i = 0; i < S->nw; i++) if (S->w[i].alive) return false;
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
            bool ok = true;
            for (int k = 0; k < S->nic; k++) {
                if (!(L->cubes[k].mode == CB_FIXED && S->icv[k] == 0)) continue;
                int rh = -1, ch = -1;
                for (int j = 0; j < S->nic; j++) {
                    if (L->cubes[j].mode != CB_FIXED || S->icv[j] == 0) continue;
                    if (S->icy[j] == S->icy[k]) rh = S->icv[j];
                    if (S->icx[j] == S->icx[k]) ch = S->icv[j];
                }
                if (rh < 0 || ch < 0) {
                    if (!g_goal_dbg) return false;
                    fprintf(stderr, "mult_table: (%d,%d) has no row/col header\n",
                            S->icx[k], S->icy[k]);
                    ok = false; continue;
                }
                Tile *t = &S->grid[S->icy[k]][S->icx[k]];
                if (!t->has_cube || t->cube != rh * ch) {
                    if (!g_goal_dbg) return false;
                    fprintf(stderr, "mult_table: (%d,%d) wants %d*%d=%d, ",
                            S->icx[k], S->icy[k], rh, ch, rh * ch);
                    if (!t->has_cube) fprintf(stderr, "tile is empty\n");
                    else              fprintf(stderr, "found %d\n", t->cube);
                    ok = false;
                }
            }
            return ok;
        }
        case G_FASHION_UNIQUE: {
            /* survivors hold one of each value; the redundant were disposed
             * of one way or another (exit dive or div-zero purge) */
            int seen[128] = { 0 };
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (w->alive) {
                    if (!w->holding || w->held < L->goal_a || w->held > L->goal_b) return false;
                    if (seen[w->held]++) return false;
                }
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
            /* everyone but the westmost must have been handed the word.
             * The greetings may land in any order -- a keen worker whose
             * word arrives from the wrong side, or ahead of its turn,
             * still counts as a greeting given and received */
            int minx = 9999;
            for (int i = 0; i < S->nw; i++)
                if (S->w[i].x < minx) minx = S->w[i].x;
            for (int i = 0; i < S->nw; i++)
                if (S->w[i].x > minx && !S->w[i].greeted) return false;
            return true;
        }
        case G_TRAINING_DAY:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].alive || S->w[i].printed < 1 || S->w[i].fed < 1) return false;
            return true;
        case G_ALTERNATE_SHRED: {
            /* the original cubes must go through the shredder in converging
             * outside-in order: leftmost, rightmost, next-left, next-right...
             * -- it is the SPECIFIC starting cube that matters each turn,
             * not just which side it came from */
            if (S->shredded != S->nic || S->nshrev != S->nic) return false;
            int ord[MAXCUBES];
            for (int k = 0; k < S->nic; k++) ord[k] = k;
            for (int i = 0; i < S->nic; i++)
                for (int j = i + 1; j < S->nic; j++)
                    if (S->icx[ord[j]] < S->icx[ord[i]]) { int t = ord[i]; ord[i] = ord[j]; ord[j] = t; }
            int lo = 0, hi = S->nic - 1;
            for (int e = 0; e < S->nshrev; e++) {
                int want = (e % 2 == 0) ? ord[lo++] : ord[hi--];
                if (S->shrev[e].id != S->ic_id[want]) return false;
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
            if (g_goal_dbg) {
                fprintf(stderr, "values missing:");
                for (int v = L->goal_a; v <= L->goal_b; v++)
                    if (!seen[v]) fprintf(stderr, " %d", v);
                fprintf(stderr, "\n");
            }
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
        case G_BINARY_COUNTER:
            if (S->hist_n == 0) return false;
            return S->hist_n >= (S->hist[0] == 1 ? 15 : 16);
        case G_DECIMAL_COUNTER:
            for (int i = 0; i < S->hist_n; i++)
                if (S->hist[i] == L->goal_b) return true;
            return false;
        case G_DECIMAL_DOUBLER:
            return S->hist_n > 0 && S->hist[S->hist_n - 1] >= L->goal_b;
        case G_NEIGHBOR_COUNTS: {
            /* only cubes on the floor are graded -- one still riding in a
             * worker's hands is exempt (and invisible as a neighbor) */
            bool ok = true;
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    int nb = 0;
                    for (int d = 0; d < 8; d++) {
                        int nx = x + DX[d], ny = y + DY[d];
                        if (nx>=0&&ny>=0&&nx<L->w&&ny<L->h&&S->grid[ny][nx].has_cube) nb++;
                    }
                    if (S->grid[y][x].cube != nb) {
                        if (!g_goal_dbg) return false;
                        fprintf(stderr, "  goal: cube (%d,%d) shows %d, wants %d\n",
                                x, y, S->grid[y][x].cube, nb);
                        ok = false;
                    }
                }
            return ok;
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
        case G_DISTANCES_FROM_DOOR:
            /* floor cubes only; a held cube is exempt from grading */
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++) {
                    if (!S->grid[y][x].has_cube) continue;
                    if (S->grid[y][x].cube != S->dist_door[y][x]) return false;
                }
            return true;
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

/* Do this level's holes swallow whoever steps in? Generic holes are
 * shallow standable pits (Checkerboard's wanderers survive their
 * renovation pits); swallowing is per-level behavior on the levels whose
 * goal involves going (or being thrown) in. */
static bool holes_swallow(const Level *L) {
    switch (L->win) {
        case G_ROOM_CLEARED:        /* everything into the pits */
        case G_ALIGNED_HOLE_EXIT:   /* the safe hole / instant doom */
        case G_ALL_EXITED:
        case G_ALL_CUBES_HELD:      /* Little Exterminator dooms */
        case G_WORKERS_EXIT_DOOR:
        case G_ROYALE_MAX_REMAINS:
        case G_DECRYPT_LEFT_EXIT:
        case G_GLORY_DIVE:
        case G_FASHION_UNIQUE:      /* redundant workers dive out */
        case G_CUBES_LINE_ROW:      /* Collation Station's disposal */
        case G_GOODBYE:
            return true;
        case G_SHRED_ALL:
            return L->goal_a != 0;  /* the alive_all variant (LE2) */
        default:
            return false;
    }
}

static void fall_check(Sim *S, Worker *w) {
    if ((S->grid[w->y][w->x].terrain == T_HOLE && holes_swallow(S->L))
        || (S->door_exit && w->x == S->L->door_x && w->y == S->L->door_y)) {
        w->alive = false; w->exited = true;
        w->exit_x = w->x; w->exit_y = w->y;
        w->exit_beat = S->beat;
        w->holding = false;                      /* the cube falls with them */
    }
}

/* find a cube by identity: in someone's hands or on the floor */
static bool cube_locate(Sim *S, int id, int *tx, int *ty) {
    if (!id) return false;
    for (int i = 0; i < S->nw; i++)
        if (S->w[i].alive && S->w[i].holding && S->w[i].held_id == id) {
            *tx = S->w[i].x; *ty = S->w[i].y;
            return true;
        }
    for (int y = 0; y < S->L->h; y++)
        for (int x = 0; x < S->L->w; x++)
            if (S->cube_id[y][x] == id) { *tx = x; *ty = y; return true; }
    return false;
}

/* resolve a mem slot to a tile; false if it holds no tile. A cube ref
 * follows the CUBE wherever it now is (Defrag Ordered's packing anchor). */
static bool mem_tile(Sim *S, Worker *w, int slot, int *tx, int *ty) {
    if (slot < 0) return false;
    if (w->mem[slot].k == MV_CUBEREF)
        return cube_locate(S, w->mem[slot].num, tx, ty);
    if (w->mem[slot].k != MV_TILE) return false;
    /* a slot that remembers a PERSON names the person, not the square they
     * stood on: every use follows them to wherever they have since walked --
     * read at the square they hold RIGHT NOW, which for someone mid-step is
     * already the square they are stepping into */
    if (w->mem[slot].wref >= 0 && w->mem[slot].wref < S->nw) {
        Worker *o = &S->w[w->mem[slot].wref];
        if (o->alive && !o->exited) {
            *tx = o->x; *ty = o->y;
            return true;
        }
    }
    *tx = w->mem[slot].x; *ty = w->mem[slot].y;
    return true;
}

/* like mem_tile, but a stale nearest-ref (the thing is gone from that tile)
 * re-resolves to the current nearest of the same type -- the game's workers
 * chase the THING they remembered, not the square it stood on */
static bool mem_tile_fresh(Sim *S, Worker *w, int slot, int *tx, int *ty) {
    if (!mem_tile(S, w, slot, tx, ty)) return false;
    MemVal *m = &w->mem[slot];
    if (m->ntype >= 0 && !nearest_matches(S, w, (CmpKind)m->ntype, *tx, *ty)) {
        int x, y;
        if (find_nearest(S, w, (CmpKind)m->ntype, &x, &y)) {
            m->x = x; m->y = y;
            /* settling on a different person makes it THAT person the slot
             * names from now on */
            if (m->ntype == (int)C_PERSON) m->wref = g_near_who;
            *tx = x; *ty = y;
        } else {
            m->k = MV_NOTHING;
            m->ntype = -1;
            m->wref = -1;
            return false;
        }
    }
    return true;
}

/* nearest / set / calc assignment (control-flow speed: executes inline).
 * Operands evaluate BEFORE the slot updates: "mem1 = calc mem1 + c" must
 * read the old mem1 (accumulator loops in Dangerous Spreadsheeting). */
static void exec_assign(Sim *S, Worker *w, Instr *ins) {
    MemVal nv = { MV_NOTHING, 0, 0, 0, -1, -1, false };
    if (ins->akind == 0) {                      /* nearest <type> */
        int x, y;
        bool got = find_nearest(S, w, ins->near_type, &x, &y);
        if (got) {
            nv.k = MV_TILE; nv.x = x; nv.y = y; nv.ntype = (int)ins->near_type;
            /* remembering the nearest PERSON remembers the person */
            if (ins->near_type == C_PERSON) nv.wref = g_near_who;
        }
        /* EMU_NEARLOG prints every nearest resolution -- who asked, at what
         * beat, and what won -- the way to hold two runs side by side */
        if (getenv("EMU_NEARLOG")) {
            int wi = (int)(w - S->w);
            if (!got)
                fprintf(stderr, "[near] t%d w%d ty%d -> NONE\n",
                        S->beat, wi, (int)ins->near_type);
            else if (ins->near_type == C_PERSON)
                fprintf(stderr, "[near] t%d w%d ty%d -> w%d\n",
                        S->beat, wi, (int)ins->near_type, g_near_who);
            else
                fprintf(stderr, "[near] t%d w%d ty%d -> (%d,%d)\n",
                        S->beat, wi, (int)ins->near_type, x, y);
        }
    } else if (ins->akind == 1) {               /* set <operand> */
        Operand *o = &ins->op1;
        if (o->kind == 1) {
            /* set remembers the THING on the tile; bare floor leaves the
             * slot empty (Terrain Leveler's approach march relies on
             * "step mem1" no-opping until an anchor cube exists).  A cube
             * is remembered AS THAT CUBE -- later reads give its current
             * value and later steps walk toward wherever it has been
             * carried, exactly like a remembered held item.  (The decimal
             * counting-house sorts its couriers by digits they looked at
             * two errands ago, on cubes that have long since been picked
             * up, rewritten and set down somewhere else.) */
            int nx = w->x + DX[o->dir], ny = w->y + DY[o->dir];
            if (nx >= 0 && ny >= 0 && nx < S->L->w && ny < S->L->h) {
                int held = -1;
                for (int k = 0; k < S->nw; k++)
                    if (&S->w[k] != w && S->w[k].alive
                        && seen_tx(S, k) == nx && seen_ty(S, k) == ny) {
                        if (S->w[k].holding) held = S->w[k].held_id;
                        break;
                    }
                if (S->grid[ny][nx].has_cube) {
                    nv.k = MV_CUBEREF; nv.num = S->cube_id[ny][nx];
                } else if (held >= 0) {
                    /* a cube in a neighbour's hands is seen too, and it is
                     * the CUBE that is remembered -- reads give whatever is
                     * written on it by the time of the read (the decimal
                     * doubling-house sorts its scribes by digits their
                     * neighbours were holding up long before the digits'
                     * final rewrite) */
                    nv.k = MV_CUBEREF; nv.num = held;
                } else if (S->grid[ny][nx].terrain != T_FLOOR
                           || worker_at(S, nx, ny, (int)(w - S->w)) >= 0) {
                    nv.k = MV_TILE; nv.x = nx; nv.y = ny;
                }
            }
        }
        else if (o->kind == 2) nv = w->mem[o->mem];
        else if (o->kind == 3) {
            /* remember the THING in my hands -- the ref follows the cube
             * after it is set down (Defrag Ordered's packing anchor) */
            if (w->holding) { nv.k = MV_CUBEREF; nv.num = w->held_id; }
        }
        else { nv.k = MV_NUM; nv.num = o->num; }
    } else if (ins->akind == 3) {
        /* set <dir,dir,...>: remember the first listed tile that holds a
         * THING (wall, machine, cube, or person); bare floor tries the next
         * (Unique Fashion Party's "set sw,n" sorts keepers by what's there) */
        for (int k = 0; k < ins->ndirs; k++) {
            int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
            bool thing = nx < 0 || ny < 0 || nx >= S->L->w || ny >= S->L->h
                || S->grid[ny][nx].terrain != T_FLOOR
                || S->grid[ny][nx].has_cube
                || worker_at(S, nx, ny, (int)(w - S->w)) >= 0;
            if (thing) { nv.k = MV_TILE; nv.x = nx; nv.y = ny; break; }
        }
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
            case '/':
                if (b != 0) { nv.k = MV_NUM; nv.num = a / b; }
                else {
                    /* dividing by zero is FATAL: the worker perishes (with
                     * their cube). Solutions weaponize "calc 0 / 0" to cull
                     * workers (Unique Fashion Party's duplicate purge). */
                    w->alive = false;
                    w->holding = false;
                }
                break;
        }
    }
    w->mem[ins->slot] = nv;
}

/* shared action helpers (used by both dir- and mem-targeted forms) */

static bool g_trace = false;

static bool divert_shredder(Sim *S, Worker *w, int wi, int px, int py);

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
        S->shrev[S->nshrev++] = (ShredEv){ w->held, w->held_src_x, w->held_src_y, nx, ny, wi, w->held_id };
    w->holding = false;
    w->fed++;
    S->shredded++;
    /* Legacy stamp in ABSOLUTE frames (the clock it once added was removed
     * with the batch engine at zero): after the opening seconds this lands
     * in the past, so the effect re-opens the machine the moment it fires
     * and the dispatch-side hold above is what actually spaces a queue.
     * Every validated timing rests on exactly this -- read the machine's
     * real cycle before touching it. */
    S->mach_busy[ny][nx] = MS_SHRED;
    if (g_trace)
        fprintf(stderr, "FEED w%d -> shredder(%d,%d) total=%d\n", wi, nx, ny, S->shredded);
}

/* a directional giveto aimed next to a shredder still feeds it: the machine
 * is wider than its home tile, and giveto ranks shredders first.  The reach
 * limit (2.4 tiles, calibrated in-game) keeps far machines out of it. */
static bool divert_find(Sim *S, Worker *w, int px, int py, int *ox, int *oy) {
    /* only a probe aimed AT a machine sees the whole bank; giveto at plain
     * floor stays a strict no-op (Uniquely Disposed's march depends on it) */
    if (S->grid[py][px].terrain != T_PRINTER) return false;
    for (int d = 0; d < 8; d++) {
        int sx = px + DX[d], sy = py + DY[d];
        if (sx<0||sy<0||sx>=S->L->w||sy>=S->L->h) continue;
        if (S->grid[sy][sx].terrain != T_SHREDDER) continue;
        int ddx = sx - w->x, ddy = sy - w->y;
        if (ddx*ddx + ddy*ddy > 5) continue;      /* 2.4^2 = 5.76 */
        *ox = sx; *oy = sy;
        return true;
    }
    return false;
}

static bool divert_shredder(Sim *S, Worker *w, int wi, int px, int py) {
    int sx, sy;
    if (!divert_find(S, w, px, py, &sx, &sy)) return false;
    feed_shredder(S, w, wi, sx, sy);
    return true;
}

/* An item action aimed through a direction at a machine binds to that
 * machine the moment the command is taken up: the direction is read once,
 * from the square the worker stood on then, and afterwards the errand
 * follows the machine itself -- the worker queues for its front square and
 * is served there, even though from the front the original direction may
 * point at empty floor.  The binding lasts as long as the command does. */
static bool dir_machine_lock(Sim *S, Worker *w, Instr *ins, int now,
                             int *ox, int *oy) {
    if (ins->mem_target >= 0) return false;
    if (ins->op == OP_TAKEFROM ? w->holding : !w->holding) return false;
    if (w->err_pc == w->pc && w->errx >= 0) {
        *ox = w->errx; *oy = w->erry;
        return true;
    }
    Terrain want = ins->op == OP_TAKEFROM ? T_PRINTER : T_SHREDDER;
    for (int k = 0; k < ins->ndirs; k++) {
        int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
        if (S->grid[ny][nx].terrain == want) {
            w->errx = nx; w->erry = ny; w->err_pc = w->pc; w->err_t0 = now;
            *ox = nx; *oy = ny;
            return true;
        }
    }
    return false;
}

/* which machine (printer for takefrom, shredder for giveto) this command is
 * about to use from where the worker stands -- the queueing gate needs to
 * know before the action fires */
static bool machine_target(Sim *S, Worker *w, Instr *ins, int *ox, int *oy) {
    /* a bound errand already knows its machine, wherever the walk ended */
    if (ins->mem_target < 0 && w->err_pc == w->pc && w->errx >= 0
        && (ins->op == OP_TAKEFROM || ins->op == OP_GIVETO)
        && abs(w->x - w->errx) <= 1 && abs(w->y - w->erry) <= 1) {
        *ox = w->errx; *oy = w->erry;
        return ins->op == OP_TAKEFROM ? !w->holding : w->holding;
    }
    if (ins->op == OP_TAKEFROM) {
        if (w->holding) return false;
        if (ins->mem_target >= 0) {
            int tx, ty;
            if (!mem_tile(S, w, ins->mem_target, &tx, &ty)) return false;
            if (abs(w->x-tx) > 1 || abs(w->y-ty) > 1) return false;
            if (S->grid[ty][tx].terrain != T_PRINTER) return false;
            *ox = tx; *oy = ty;
            return true;
        }
        for (int k = 0; k < ins->ndirs; k++) {
            int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
            if (S->grid[ny][nx].terrain == T_PRINTER) { *ox = nx; *oy = ny; return true; }
        }
        return false;
    }
    if (ins->op != OP_GIVETO || !w->holding) return false;
    if (ins->mem_target >= 0) {
        int tx, ty;
        if (!mem_tile(S, w, ins->mem_target, &tx, &ty)) return false;
        if (abs(w->x-tx) > 1 || abs(w->y-ty) > 1) return false;
        if (S->grid[ty][tx].terrain != T_SHREDDER) return false;
        *ox = tx; *oy = ty;
        return true;
    }
    for (int k = 0; k < ins->ndirs; k++) {
        int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
        if (S->grid[ny][nx].terrain == T_SHREDDER) { *ox = nx; *oy = ny; return true; }
        if (divert_find(S, w, nx, ny, ox, oy)) return true;
    }
    return false;
}

/* While anyone stands on the big red button the green display tracks the
 * sensors; each distinct number shown is remembered, and a number out of
 * sequence makes the display forget everything (start the count over). */
static void counter_press(Sim *S) {
    const Level *L = S->L;
    bool pressed = false;
    for (int i = 0; i < S->nw; i++) {
        Worker *w = &S->w[i];
        if (!w->alive || w->exited) continue;
        if (seen_tx(S, i) == L->button_x && seen_ty(S, i) == L->button_y)
            pressed = true;
    }
    {
        const char *dbg = getenv("EMU_CTRDBG");
        if (dbg && atoi(dbg) >= 2) {
            static int fr = 0;
            fprintf(stderr, "f%05d %c ", fr++, pressed ? 'P' : '.');
            for (int i = 0; i < S->nw; i++)
                fprintf(stderr, "w%d(%d,%d%s%s) ", i, body_tx(&S->w[i]), body_ty(&S->w[i]),
                        S->w[i].holding ? "*" : "", S->w[i].wtx >= 0 ? ">" : "");
            fprintf(stderr, "pads:");
            for (int i = 0; i < L->nsw; i++)
                fprintf(stderr, "%d", S->grid[L->sw_y[i]][L->sw_x[i]].has_cube ? 1 : 0);
            fprintf(stderr, "\n");
        }
    }
    /* The machine reads its pads when the button GOES down, not for as long as
     * it is held: it remembers whether anyone was standing there a moment ago
     * and does nothing at all while that stays true.  Standing on the button is
     * therefore one reading, not one a frame -- which is what stops the display
     * photographing the pads half way through being rearranged. */
    {
        bool was = S->ctr_down;
        S->ctr_down = pressed;
        if (!pressed || was) return;
    }
    long v = 0;
    for (int i = 0; i < L->nsw; i++) {
        Tile *t = &S->grid[L->sw_y[i]][L->sw_x[i]];
        if (L->win == G_BINARY_COUNTER)
            v = v * 2 + (t->has_cube ? 1 : 0);
        else {
            int d = t->has_cube ? t->cube : 0;
            if (d < 0) d = 0;
            if (d > 9) d = 9;
            v = v * 10 + d;
        }
    }
    if (S->hist_n > 0 && S->hist[S->hist_n - 1] == (int)v) return;  /* same picture */
    if (S->hist_n < 128) S->hist[S->hist_n++] = (int)v;
    if (getenv("EMU_CTRDBG"))
        fprintf(stderr, "[ctr] +%ld (hist %d)\n", v, S->hist_n);
    /* The counting-up display takes whatever the pads spell at each press.
     * Mid-carry a press can catch the digits half-rearranged and flash a
     * number far out of sequence -- the machine does not sulk about it, and
     * the goal asks only that the count ARRIVE at its target.  (The pads
     * spell the carry preparation before the lower digits reset, so even a
     * winning routine flashes such numbers; a strict step-by-step check
     * here would fail solutions the level itself accepts.) */
    if (L->win == G_DECIMAL_COUNTER) return;
    for (int i = 0; i < S->hist_n; i++) {
        long want;
        if (L->win == G_BINARY_COUNTER)      want = (S->hist[0] == 1 ? 1 : 0) + i;
        else                                  want = (long)L->goal_a << i;
        if (S->hist[i] != want) {
            if (getenv("EMU_CTRDBG"))
                fprintf(stderr, "[ctr] wipe: hist[%d]=%d want %ld\n", i, S->hist[i], want);
            S->hist_n = 0; return;
        }
    }
}

/* returns true if something was picked up (or the worker exploded) */
static bool pickup_at(Sim *S, Worker *w, int wi, int nx, int ny) {
    Tile *t = &S->grid[ny][nx];
    if (t->terrain == T_PRINTER) {               /* fresh print */
        /* the quiet-office printers hold exactly the ration of paper the goal
         * asks for -- runs dry after goal_a sheets, freezing the counts the
         * win check wants (the size solution loops unboundedly otherwise) */
        if (S->L->win == G_PRINTSHRED_QUIET && S->prints_at[ny][nx] >= S->L->goal_a)
            return false;
        S->prints_at[ny][nx]++;
        w->holding = true;
        w->held = S->print_next[ny][nx];
        S->print_next[ny][nx] =
            (int)(game_rnd(S) % (unsigned)(S->L->randmax + 1));
        w->held_id = ++S->next_cube_id;
        w->held_src_x = w->held_src_y = -1;
        w->held_owner = wi;
        w->fresh = 2;
        w->printed++;
        S->pickups++;
        S->mach_busy[ny][nx] = MS_PRINTER;   /* legacy absolute stamp: see
                                                feed_shredder's note */
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
        w->held_id = S->cube_id[ny][nx];
        w->held_src_x = nx; w->held_src_y = ny;
        w->held_owner = t->owner;
        w->fresh = 2;
        t->has_cube = false; t->owner = -1;
        S->cube_id[ny][nx] = 0;
        S->pickups++;
        return true;
    }
    return false;
}

/* Does this condition LOOK anywhere?  A term with a direction operand on
 * either side (n/e/s/w/...) makes the worker raise the think bubble and
 * study the square for the full thinking time.  A condition that consults
 * only the worker's own head -- memory slots, the held item, plain
 * numbers, kind words -- is answered in a single frame, no bubble. */
static bool if_looks(const Instr *ins) {
    for (int k = 0; k < ins->nconds; k++) {
        if (ins->conds[k].lhs.kind == 1) return true;
        if (!ins->conds[k].rhs_is_type && ins->conds[k].rhs.kind == 1)
            return true;
    }
    return false;
}

/* Is this item action about to use a machine (a takefrom or pickup at a
 * printer, a giveto at a shredder)?  The machine event shapes and the
 * one-customer busy gate apply exactly when it is.  A machine bound by a
 * direction errand counts wherever the walk to its front square ended;
 * otherwise the remembered tile or the named directions are scanned. */
static bool item_at_machine(Sim *S, Worker *w, Instr *ins) {
    Terrain want;
    if (ins->op == OP_PICKUP || ins->op == OP_TAKEFROM) want = T_PRINTER;
    else if (ins->op == OP_GIVETO) want = T_SHREDDER;
    else return false;
    if (ins->op != OP_PICKUP && ins->mem_target < 0
        && w->err_pc == w->pc && w->errx >= 0
        && S->grid[w->erry][w->errx].terrain == want
        && abs(w->x - w->errx) <= 1 && abs(w->y - w->erry) <= 1)
        return true;
    if (ins->mem_target >= 0) {
        int tx, ty;
        return mem_tile(S, w, ins->mem_target, &tx, &ty)
            && S->grid[ty][tx].terrain == want;
    }
    for (int k = 0; k < ins->ndirs; k++) {
        int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
        if (nx >= 0 && ny >= 0 && nx < S->L->w && ny < S->L->h
            && S->grid[ny][nx].terrain == want) return true;
    }
    return false;
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

/* Apply the non-movement effect of the instruction at w->pc and advance pc.
 * Shared by the schedulers so their action semantics stay identical -- never
 * called for OP_STEP (movement lives in the scheduler itself). */
static void exec_action(Sim *S, Program *P, int i) {
    Worker *w = &S->w[i];
    Instr *ins = &P->instr[w->pc];
    if (w->fresh > 0) w->fresh--;   /* one command boundary passed */
    switch (ins->op) {
        case OP_IF: {
            if (ins->nconds == 0) {
                fprintf(stderr, "error: unsupported condition: %s\n", ins->raw);
                exit(3);
            }
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
                    if (mem_tile(S, w, ins->mem_target, &tx, &ty))
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
            if (!t->has_cube && t->terrain == T_FLOOR) {
                t->has_cube = true; t->cube = w->held; t->owner = w->held_owner;
                S->cube_id[w->y][w->x] = w->held_id;
                w->holding = false;
                S->drops++;
                if (g_trace)
                    fprintf(stderr, "DROP w%d @(%d,%d) parity %d\n",
                            i, w->x, w->y, (w->x + w->y) & 1);
            }
            break;
        }
        case OP_GIVETO: {
            if (!w->holding) break;
            int cx[9], cy[9], nc = 0;
            if (ins->mem_target < 0 && w->err_pc == w->pc && w->errx >= 0
                && S->grid[w->erry][w->errx].terrain == T_SHREDDER
                && abs(w->x - w->errx) <= 1 && abs(w->y - w->erry) <= 1) {
                /* a bound errand feeds ITS shredder, wherever the walk ended */
                cx[nc] = w->errx; cy[nc] = w->erry; nc++;
            } else if (ins->mem_target >= 0) {
                int nx, ny;
                if (!mem_tile(S, w, ins->mem_target, &nx, &ny)) break;
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
                } else if (ins->mem_target < 0 && divert_shredder(S, w, i, nx, ny)) {
                    /* a shredder overlapping the probed tile took the cube */
                } else {
                    int j = worker_at(S, nx, ny, i);
                    if (j >= 0 && !S->w[j].holding) {
                        S->w[j].holding = true; S->w[j].held = w->held;
                        S->w[j].held_src_x = w->held_src_x; S->w[j].held_src_y = w->held_src_y;
                        S->w[j].held_owner = w->held_owner;
                        S->w[j].held_id = w->held_id;
                        S->w[j].fresh = 2;
                        w->holding = false;
                    }
                }
            }
            break;
        }
        case OP_TAKEFROM: {
            if (w->holding) break;
            if (ins->mem_target >= 0) {
                int tx, ty;
                if (!mem_tile(S, w, ins->mem_target, &tx, &ty)) break;
                if (abs(w->x-tx) > 1 || abs(w->y-ty) > 1) break;
                if (S->grid[ty][tx].terrain == T_PRINTER) {
                    pickup_at(S, w, i, tx, ty);
                } else {
                    int j = worker_at(S, tx, ty, i);
                    if (j >= 0 && S->w[j].holding && S->w[j].fresh == 0) {
                        w->holding = true; w->held = S->w[j].held;
                        w->held_src_x = S->w[j].held_src_x; w->held_src_y = S->w[j].held_src_y;
                        w->held_owner = S->w[j].held_owner;
                        w->held_id = S->w[j].held_id;
                        w->fresh = 2;
                        S->w[j].holding = false;
                    }
                }
                break;
            }
            /* a bound machine errand takes from ITS machine, however the
             * walk to the front has turned the original direction */
            if (w->err_pc == w->pc && w->errx >= 0
                && S->grid[w->erry][w->errx].terrain == T_PRINTER
                && abs(w->x - w->errx) <= 1 && abs(w->y - w->erry) <= 1) {
                pickup_at(S, w, i, w->errx, w->erry);
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
                    w->held_id = S->w[j].held_id;
                    w->fresh = 2;
                    S->w[j].holding = false;
                    break;
                }
            }
            break;
        }
        case OP_WRITE: {
            int v;
            if (w->holding && operand_value(S, w, &ins->op1, &v))
                w->held = v % 100;
            break;
        }
        case OP_TELL: {
            w->last_tell = S->beat;
            if (S->ntellev < MAXTELLEV) {
                TellEv *e = &S->tellev[S->ntellev++];
                e->worker = i; e->x = w->x;
                snprintf(e->word, sizeof e->word, "%s", ins->word);
            }
            for (int j = 0; j < S->nw; j++) {
                if (j == i) continue;
                Worker *o = &S->w[j];
                if (!o->alive || o->done) continue;
                bool covered = false;
                if (ins->tt_kind == 1) covered = true;
                else if (ins->tt_kind == 2)
                    covered = (o->x == w->x + DX[ins->tt_dir] && o->y == w->y + DY[ins->tt_dir]);
                else if (ins->tt_kind == 3) {
                    int tx, ty;
                    covered = mem_tile(S, w, ins->tt_mem, &tx, &ty) && o->x == tx && o->y == ty;
                }
                if (covered) {
                    o->heard = MS_EARSHOT;
                    o->greeted = true;
                    snprintf(o->heard_word, sizeof o->heard_word, "%s", ins->word);
                }
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

/* ------------------------------------------------- continuous scheduler --- */
/* The game moves workers as smooth bodies gliding between tile centres at a
 * fixed speed, committing the logical tile (and snapping to the centre) only
 * on arrival.  Modelled here: diagonal steps take sqrt(2)x longer, a walker
 * keeps its source tile until it arrives (so a follower must wait for the
 * leader to vacate -- the conga wave), and two workers aimed into each other
 * swap.  Reuses every effect helper via exec_action(). */

/* A failed item action stops the worker for the error bubble before the
 * program moves on: picking up or taking with full hands, giving with empty
 * hands, dropping with empty hands or onto a tile that already has a cube.
 * (Taking from a worker whose hands are empty is the exception -- that is a
 * silent instant retry, not an error.) */
#define MS_ERRB 94              /* the error bubble, 1.5 s in ticks */


/* Which tile is a worker IN?  The one it holds -- and it takes hold of the tile
 * it is stepping into at the START of the step, letting go of the one behind it
 * then, not on arrival.  Its body slides across afterwards and is only ever
 * animation.  So a follower may enter the tile behind as soon as the leader has
 * set off, which is what makes a queue flow as a wave. */
static int cont_occupant(Sim *S, int x, int y, int self) {
    for (int j = 0; j < S->nw; j++) {
        if (j == self) continue;
        Worker *o = &S->w[j];
        if (o->alive && !o->exited && o->x == x && o->y == y) return j;
    }
    return -1;
}
/* the worker whose LOGICAL tile is (x,y).  A walking worker keeps its logical
 * tile until it lands, so this -- not the half-way body position -- is what a
 * standoff is made of: use it when deciding whether a set of blocked workers
 * forms a closed ring that can rotate. */
static int cont_holder(Sim *S, int x, int y, int self) {
    for (int j = 0; j < S->nw; j++) {
        if (j == self) continue;
        Worker *o = &S->w[j];
        if (o->alive && !o->exited && o->x == x && o->y == y) return j;
    }
    return -1;
}
static bool cont_reserved(Sim *S, int x, int y, int self) {
    for (int j = 0; j < S->nw; j++) {
        if (j == self) continue;
        Worker *o = &S->w[j];
        if (!o->alive || o->exited) continue;
        if (o->x == x && o->y == y) return true;
        if (o->wtx == x && o->wty == y) return true;
    }
    return false;
}

/* land worker i on its walk target: update the logical tile + advance the
 * step (single-tile dir-steps advance pc; mem/travel walks re-evaluate). */
static void cont_land(Sim *S, int i) {
    Worker *w = &S->w[i];
    if (!w->wowned) { w->x = w->wtx; w->y = w->wty; }   /* else already taken */
    w->fx = w->x; w->fy = w->y;
    w->wtx = w->wty = -1;
    w->wintx = w->winty = -1;
    w->wowned = false;
    if (w->wsingle) { if (w->fresh > 0) w->fresh--; w->pc++; }
    fall_check(S, w);
}

/* begin a one-tile glide toward (tx,ty) */
static void cont_walk(Sim *S, int i, int tx, int ty, bool single, bool flex) {
    Worker *w = &S->w[i];
    w->wtx = tx; w->wty = ty; w->wsingle = single; w->wflex = flex;
    w->wsettle = false;            /* each walk decides afresh how it lands */
    w->wintx = w->winty = -1;      /* an actual walk supersedes any intent */
    w->wowned = false;             /* the tile is not ours until it is free */
    /* A walk lasts a FIXED whole number of frames, decided when it starts and
     * independent of where the worker is standing.  Deriving arrival from a
     * floating-point distance instead makes the frame count depend on the
     * absolute coordinates (accumulated rounding differs at y=8 and y=6), so
     * two workers taking geometrically identical steps can land a frame apart
     * and a synchronized crowd silently drifts out of step. */
    int diag = (tx != w->x && ty != w->y);
    int base = MS_STEP > 0 ? MS_STEP : 1;
    w->wtot = diag ? (int)(base * 1.41421356 + 0.5) : base;
    if (w->wtot < 1) w->wtot = 1;
    w->wprog = 0;
}

/* A machine has a FRONT: a walk aimed at one heads for the single square one
 * tile off on the near side of it, not for whichever neighbour happens to be
 * closest.  (The level geometry agrees -- across every level no machine has a
 * wall on that side, while over half of them are backed by one.)  So a crowd
 * queueing for a shredder queues on one square, and the order it is served in
 * falls out of contention for that square rather than out of pathfinding. */
static bool machine_front(const Sim *S, int *tx, int *ty) {
    Terrain t = S->grid[*ty][*tx].terrain;
    if (t != T_SHREDDER && t != T_PRINTER) return false;
    if (*ty - 1 < 0) return false;
    (*ty)--;
    return true;
}

/* the tile a walker is bound for, counted from the one it stands on.  Holding
 * a tile and standing on it are the same thing here: once the tile ahead has
 * been taken it is where the worker is and it has nowhere left to be, so it
 * reads as arrived however much ground the body still has to cover. */
static void mover_goal(const Worker *o, int *tx, int *ty) {
    if (o->wtx >= 0 && !o->wowned)        { *tx = o->wtx;   *ty = o->wty; }
    else if (o->wtx < 0 && o->wintx >= 0) { *tx = o->wintx; *ty = o->winty; }
    else                                  { *tx = o->x;     *ty = o->y; }
}

/* Start a glide into a tile that is already ours: the claim flips now, the
 * body sets off from wherever it is and covers the ground over a real step's
 * worth of frames.  Everything that trades places moves this way -- nothing
 * about changing squares is instantaneous. */
static void cont_glide_owned(Sim *S, int k, int tx, int ty, bool single) {
    Worker *m = &S->w[k];
    int diag = (tx != m->x && ty != m->y);
    m->fsx = m->fx; m->fsy = m->fy;
    m->x = tx; m->y = ty;                   /* the claim flips at set-off */
    m->wtx = tx; m->wty = ty;
    m->wowned = true; m->wsingle = single; m->wflex = false;
    m->wintx = m->winty = -1;
    int base = MS_STEP > 0 ? MS_STEP : 1;
    m->wtot = diag ? (int)(base * 1.41421356 + 0.5) : base;
    if (m->wtot < 1) m->wtot = 1;
    m->wprog = 0;
    (void)S;
}

/* trade places with the worker holding the tile we are entering: we glide
 * onto it, they glide onto the one we are leaving.  Both claims change at
 * once and the two bodies cross in mid-tile, each taking a real step's time
 * to arrive -- a swap is two walks, not a teleport.
 *
 * Being displaced does not change the other's mind: it keeps the tile it was
 * aiming for (otx,oty) and re-routes there once it lands, one square over.
 * Only when that tile is the one we hand over does its own step finish. */
static void cont_exchange(Sim *S, int i, int j, int otx, int oty) {
    Worker *w = &S->w[i], *o = &S->w[j];
    int ax = w->x, ay = w->y;
    int wtx = w->wtx, wty = w->wty;
    bool osingle = o->wsingle;
    cont_glide_owned(S, i, wtx, wty, w->wsingle);
    cont_glide_owned(S, j, ax, ay, osingle && otx == ax && oty == ay);
}

/* Move a gliding body one frame along its line.  The body covers a FIXED
 * DISTANCE each frame -- walking speed, 3.0 tiles/s at 16 ms a frame = 0.048
 * tiles -- not an equal share of the walk.  The two agree on straight steps
 * (both cross the half-tile line on frame 11 of 21) but not on diagonals: at
 * frame 15 of 30 an equal-share body sits EXACTLY on the half-tile line and
 * the tile it reads as depends on which way the tie rounds, while a fixed-
 * speed body is already 0.009 past it, so both coordinates flip cleanly ON
 * frame 15.  A counting machine's button feels that frame: its presser
 * approaches diagonally, and the press must land the frame the body reads as
 * on the button tile.  Arrival stays the fixed frame count decided at
 * set-off (see cont_walk); a body that runs out of ground early -- a
 * displaced worker setting off mid-tile -- parks at the target and waits
 * out its walk clock. */
#define GLIDE_PER_FRAME 0.048   /* tiles of ground covered per frame */
static void glide_body(Worker *w) {
    double dx = w->wtx - w->fsx, dy = w->wty - w->fsy;
    double dist = sqrt(dx * dx + dy * dy);
    double f = dist > 1e-9 ? (double)w->wprog * GLIDE_PER_FRAME / dist : 1.0;
    if (f > 1.0) f = 1.0;
    w->fx = w->fsx + dx * f;
    w->fy = w->fsy + dy * f;
}

/* advance a gliding worker; commit on arrival (with swap/cycle).  returns
 * true if anything about the board changed (for stall detection). */
static bool cont_glide(Sim *S, Program *P, int i) {
    Worker *w = &S->w[i];
    int tx = w->wtx, ty = w->wty;
    if (w->wowned) {
        /* the tile is already ours: nothing left to contest, the body is just
         * covering the ground between the two tiles. */
        w->wprog++;
        if (w->wprog >= w->wtot) { cont_land(S, i); return true; }
        glide_body(w);
        return true;
    }
    int occ = cont_occupant(S, tx, ty, i);
    /* target still held by another worker -> either a swap or a wait */
    if (occ >= 0) {
        Worker *o = &S->w[occ];
        if (!o->done && o->wtx == w->x && o->wty == w->y) {
            /* mutual swap: glide both across and land them */
            cont_exchange(S, i, occ, w->x, w->y);
            return true;
        }
        /* Near head-on.  The blocker is bound for a tile no farther from ours
         * than from its own, and its walk is a flexible one -- it either named
         * several directions and took whichever came up, or it is chasing an
         * object.  Neither is committed to one square, so it gives way: it is
         * sent into the tile we are leaving and we take its place. */
        if (!o->done && o->wflex) {
            int otx, oty;
            mover_goal(o, &otx, &oty);
            int d1 = (otx - o->x) * (otx - o->x) + (oty - o->y) * (oty - o->y);
            int d2 = (otx - w->x) * (otx - w->x) + (oty - w->y) * (oty - w->y);
            if (d2 <= d1) { cont_exchange(S, i, occ, otx, oty); return true; }
        }
        /* closed rotation cycle: everyone in the loop advances together.
         * Follow LOGICAL tiles here -- a mid-step body has already flipped to
         * the tile it is entering, so tracing bodies walks off the ring and
         * a genuine standoff never looks closed. */
        #define WANTS(k)  (S->w[k].wtx >= 0 || S->w[k].wintx >= 0)
        #define WANT_X(k) (S->w[k].wtx >= 0 ? S->w[k].wtx : S->w[k].wintx)
        #define WANT_Y(k) (S->w[k].wtx >= 0 ? S->w[k].wty : S->w[k].winty)
        int chain[MAXWORKERS], cn = 0, cur = i; bool closed = false;
        while (cn < S->nw) {
            chain[cn++] = cur;
            int nxt = cont_holder(S, WANT_X(cur), WANT_Y(cur), cur);
            if (nxt < 0) break;
            if (nxt == i) { closed = true; break; }
            if (S->w[nxt].done || !WANTS(nxt)) break;
            bool seen = false; for (int k = 0; k < cn; k++) if (chain[k] == nxt) seen = true;
            if (seen) break;
            cur = nxt;
        }
        if (closed && cn > 1) {
            /* the whole ring sets off together: every member glides into the
             * tile it wanted, each taking a real step's worth of frames */
            for (int k = 0; k < cn; k++) {
                int mx = WANT_X(chain[k]), my = WANT_Y(chain[k]);
                cont_glide_owned(S, chain[k], mx, my, S->w[chain[k]].wsingle);
            }
            return true;
        }
        #undef WANTS
        #undef WANT_X
        #undef WANT_Y
        /* A worker that has finished its program is still solid, but it is a
         * bystander: rather than wait on it forever, the mover has it pushed
         * out into the tile being vacated.  A shove is an ORDER, not a move:
         * this tick the seat merely sets off toward our square while we hold
         * position, and on the next pass the two walks meet head-on and trade
         * as any mutual swap does.  Being made to wait that beat is part of
         * the price of going through someone. */
        if (o->done && o->wtx < 0 && o->wintx < 0) {
            /* ...unless the one in the way has finished AND is standing on a
             * laid-down cube.  Then the tile is not a seat to be shoved out of
             * but a finished piece of work, and the walker only set off into it
             * because the cube was not there yet when it looked.  A marching
             * loop -- "while the way ahead is clear, walk on" -- has its step
             * cut short here and goes round again: this time it sees the cube,
             * falls out of the loop, and lays its own down where it stands
             * instead of walking over the line it was helping to build. */
            if (w->wsingle && S->grid[o->y][o->x].has_cube
                && w->pc + 1 < P->n && P->instr[w->pc + 1].op == OP_JUMP) {
                w->wtx = w->wty = -1; w->wowned = false;
                w->fx = w->x; w->fy = w->y;
                w->pc++;
                return true;
            }
            cont_walk(S, occ, w->x, w->y, false, false);
            return true;
        }
        /* blocked: hold position and wait for the tile to clear (the wave).
         * a travel walk abandons this tile so its command can re-route -- but
         * it still MEANS to go there, so remember the intent: a ring of
         * blocked travellers only ever looks closed if the members that are
         * between routes still count as links in it. */
        if (!w->wsingle) {
            w->wintx = tx; w->winty = ty;
            w->wtx = w->wty = -1;
            /* the walk never happened, so the body belongs back on the tile it
             * never left: a smooth position abandoned part-way through rounds
             * to the tile ahead, and the worker would go on blocking a tile it
             * is not standing on for the rest of the run. */
            w->fx = w->x; w->fy = w->y;
            return true;
        }
        return false;
    }
    /* nobody holds the tile: take it now and let go of the one behind us.  The
     * body stays where it is and slides across over the rest of the walk. */
    w->fsx = w->fx; w->fsy = w->fy;
    w->x = tx; w->y = ty;
    w->wowned = true;
    w->wprog++;
    if (w->wprog >= w->wtot) { cont_land(S, i); return true; }
    glide_body(w);
    return true;
}

/* run one instruction for a free worker (skipping free control flow). */
/* Dispatch a step command: weigh the squares it names, start the walk --
 * or advance past a step with nowhere to go.  This is the scheduler's one
 * entry into the walking machinery for a plain step. */
static void step_dispatch(Sim *S, Program *P, int i, bool *progressed) {
    Worker *w = &S->w[i];
    if (!w->alive || w->done) return;
    Instr *ins = &P->instr[w->pc];
        if (S->L->rules & R_NOWALK) { S->failed = true; return; }
        if (ins->mem_target >= 0) {
            /* A step toward a remembered PERSON walks up beside them, not
             * onto them.  The square to stand on is chosen first -- the
             * touching square nearest the walker, judged the same way
             * `nearest` judges closeness -- and the route is then drawn to
             * that square, so a cardinal neighbour beats a diagonal one
             * even where the path itself would happily cut the corner. */
            bool person = (w->mem[ins->mem_target].wref >= 0);
            int tx, ty;
            if (!mem_tile(S, w, ins->mem_target, &tx, &ty)
                || (person ? (abs(w->x - tx) <= 1 && abs(w->y - ty) <= 1)
                           : (w->x == tx && w->y == ty))) {
                if (getenv("EMU_CMDLOG"))
                    fprintf(stderr, "[smem] w%d noop tgt=%d,%d person=%d\n",
                            (int)(w - S->w), tx, ty, (int)person);
                if (w->fresh > 0) w->fresh--;
                /* finding yourself already beside the person still costs
                 * the stride you do not take: the body turns to them and
                 * the program only then moves on.  Arriving beside them at
                 * the end of a stride costs nothing more. */
                if (person && w->smem_pc != w->pc) w->busy = 11;
                w->smem_pc = -1;
                w->pc++; *progressed = true; return;
            }
            if (person) {
                long bestk = 0; int bx = -1, by = -1;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++) {
                        if (!dx && !dy) continue;
                        int ax = tx + dx, ay = ty + dy;
                        if (ax < 0 || ay < 0 || ax >= S->L->w || ay >= S->L->h)
                            continue;
                        if (S->grid[ay][ax].terrain != T_FLOOR) continue;
                        long k = near_key(ax - w->x, ay - w->y);
                        if (bx < 0 || k < bestk) { bestk = k; bx = ax; by = ay; }
                    }
                if (bx >= 0) { tx = bx; ty = by; }
            }
            int d = route_step(S, w, tx, ty, false);
            if (d < 0) { if (w->fresh > 0) w->fresh--; w->pc++; *progressed = true; return; }
            if (person) w->smem_pc = w->pc;
            /* only a walk that chases a PERSON can be made to give way --
             * a walk to a remembered square is bound for that square */
            cont_walk(S, i, w->x + DX[d], w->y + DY[d], false, person);
            *progressed = true; return;
        }
        int cand[8], nc = 0, freec[8], fnc = 0;
        for (int k = 0; k < ins->ndirs; k++) {
            Dir d = ins->dirs[k];
            int nx = w->x + DX[d], ny = w->y + DY[d];
            if (!walkable(S, nx, ny)) continue;
            cand[nc++] = d;
            if (g_snap_n >= 0) {
                /* the picker judges a square by the bodies on it, not by
                 * claims: a tile someone has merely set off toward is still
                 * picked (the walk itself then sorts out the right of way) */
                bool occ = false;
                for (int j = 0; j < S->nw; j++)
                    if (j != i && S->w[j].alive
                        && seen_tx(S, j) == nx && seen_ty(S, j) == ny)
                        occ = true;
                if (!occ) freec[fnc++] = d;
            } else if (!cont_reserved(S, nx, ny, i)) freec[fnc++] = d;
        }
        int *pool = fnc > 0 ? freec : cand, pn = fnc > 0 ? fnc : nc;
        if (pn == 0) {
            /* a step with nowhere to go still takes its frame before the
             * program moves on -- the skipped stride is not free */
            if (w->fresh > 0) w->fresh--;
            w->pc++; w->busy = 1;
            *progressed = true; return;
        }
        /* A step that names one direction has nothing to choose.  One that
         * names several draws over ALL of them, in the order written, and
         * simply draws again when the pick is unwalkable or a body stands
         * there -- uniform over the free squares, but the dice are rolled
         * for the rejected picks too.  With every named square occupied
         * there is nothing acceptable to draw: settle on any one of them
         * and queue there. */
        int d;
        if (ins->ndirs == 1) d = pool[0];
        else if (fnc > 0) {
            for (;;) {
                int cd = ins->dirs[game_rnd(S) % (unsigned)ins->ndirs];
                bool ok = false;
                for (int k = 0; k < fnc; k++) if (freec[k] == cd) ok = true;
                if (ok) { d = cd; break; }
            }
        } else d = cand[game_rnd(S) % (unsigned)nc];
        /* a plain stride is bound for the square it drew, however many
         * directions were in the hat -- it is never made to give way.
         * Only a walk chasing a thing (an errand, a person) can be
         * crossed by someone it is blocking. */
        cont_walk(S, i, w->x + DX[d], w->y + DY[d], true, false);
        *progressed = true; return;
}



/* ---- the event-queue scheduler -------------------------------------------
 * A command queues a little timeline and the queue carries the time:
 * waits and new animations line up behind a running animation, while the
 * command's effect and the suspend/resume bookkeeping are instant.  An
 * animation only holds its own queue -- its tail plays out under whatever
 * the worker does next, so a pickup before a walk costs less wall time
 * than a pickup before another animation. */
enum { FQ_WAIT = 1, FQ_ANIM, FQ_WAITANIM, FQ_EFFECT, FQ_SUSPEND, FQ_RESUME,
       FQ_GRAB, FQ_ERRND };

static const float FQ_DT = 1.0f;      /* queue times are in frames */

static void fq_push(Worker *w, int id, float t) {
    if (w->evn < (int)(sizeof w->evq / sizeof w->evq[0])) {
        w->evq[w->evn].id = (unsigned char)id;
        w->evq[w->evn].t = t;
        w->evn++;
    }
}

/* frames of gating before an item action's effect, and the animation tail
 * that plays out afterwards (30 frames of animation in all) */
static int FQ_ITEM_PRE = 16, FQ_ITEM_TAIL = 16;
/* The think bubble is a half-second affair, and the condition is read when it
 * is exactly half spent: 250ms of sixteen-millisecond ticks puts the reading
 * on the sixteenth, and the bubble lingers to its 500ms before the branch is
 * taken.  It had been living a 333ms life with an eleventh-tick reading --
 * long enough to look right, short enough to read a stepping neighbour's
 * square four ticks too early, which is the difference between a whole
 * counting-house choreography finding its caller or freezing forever. */
static int FQ_IF_WAIT = 16;
static int FQ_IF_HOLD = 17;
static int MS_CALC = 122;             /* the calc arithmetic animation, plus
                                         the half-second thought that follows
                                         it before the hands move again; a
                                         direction operand adds one whole
                                         look on top (see the dispatch) */
static int MS_SHRED_HOLD = 38;        /* one full shredder cycle per customer */
static int FQ_FOREACH_BASE = 333;     /* ms of one standard command per sweep */
#define MS_TICK 16                    /* milliseconds in a tick */

/* The standard look: half the thought before the eyes land on the square,
 * the sample, then the other half while the thought completes.  Every
 * command that turns to a neighbouring square pays exactly this shape --
 * a condition, a set from a direction, a reach that finds empty hands. */
static void fq_push_look(Worker *w) {
    fq_push(w, FQ_WAIT, (float)FQ_IF_WAIT);
    fq_push(w, FQ_EFFECT, 0);
    fq_push(w, FQ_WAIT, (float)FQ_IF_HOLD);
}

/* A whole look with nothing there to act on, then the long red bubble of
 * error; the program moves on once the bubble fades. */
static void fq_push_look_error(Worker *w) {
    fq_push(w, FQ_WAIT, (float)(FQ_IF_WAIT + FQ_IF_HOLD));
    fq_push(w, FQ_WAIT, (float)MS_ERRB);
    fq_push(w, FQ_ERRND, 0);
}

/* A hand-off between workers is a throw and a catch: the cube is in the
 * air for the throw, changes owner mid-gesture, and the catch is held out
 * to the end. */
static void fq_push_throw(Worker *w) {
    fq_push(w, FQ_WAIT, 15);
    fq_push(w, FQ_EFFECT, 0);
    fq_push(w, FQ_ANIM, 21);
    fq_push(w, FQ_WAITANIM, 0);
}

/* Taking from another worker's hands is no throw: the taker looks, then
 * reaches across and lifts the cube out, and it is theirs only at the very
 * end of the reach.  The one being robbed is pinned for the reach --
 * whatever they were doing resumes only after the hand has withdrawn. */
static void fq_push_steal(Worker *w) {
    fq_push(w, FQ_WAIT, (float)(FQ_IF_WAIT + FQ_IF_HOLD));
    fq_push(w, FQ_GRAB, 0);
    fq_push(w, FQ_WAIT, 83);
    fq_push(w, FQ_EFFECT, 0);
}

/* run the queue; returns false while something in it is still holding */
static bool fq_pump(Sim *S, Program *P, int i) {
    Worker *w = &S->w[i];
    while (w->evcur < w->evn) {
        struct { unsigned char id; float t; } *ev =
            (void *)&w->evq[w->evcur];
        switch (ev->id) {
            case FQ_WAIT:               /* pure timer, blind to animations */
                ev->t -= FQ_DT;
                if (ev->t > 0) return false;
                w->evcur++; continue;
            case FQ_ANIM:               /* starts at once, replacing any prior */
                w->animms = ev->t;
                w->evcur++; continue;
            case FQ_WAITANIM:           /* the one thing that waits for one */
                if (w->animms > 0) return false;
                w->evcur++; continue;
            case FQ_EFFECT:
                exec_action(S, P, i);
                w->evcur++; continue;
            case FQ_SUSPEND: w->fsusp = true;  w->evcur++; continue;
            case FQ_RESUME:  w->fsusp = false; w->evcur++; continue;
            case FQ_GRAB: {
                /* The taker's look has landed on the one to be robbed: pin
                 * them for the reach.  The pin goes in AHEAD of whatever
                 * resolution their own timeline was building towards, so a
                 * command caught mid-flight settles only after the hand has
                 * withdrawn. */
                Instr *ins = &P->instr[w->pc];
                int tx, ty, j = -1;
                if (ins->mem_target >= 0) {
                    if (mem_tile(S, w, ins->mem_target, &tx, &ty))
                        j = worker_at(S, tx, ty, i);
                } else {
                    for (int k = 0; k < ins->ndirs && j < 0; k++) {
                        int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                        int c = worker_at(S, nx, ny, i);
                        if (c >= 0 && S->w[c].holding) j = c;
                    }
                }
                if (j >= 0 && S->w[j].holding) {
                    Worker *v = &S->w[j];
                    int cap = (int)(sizeof v->evq / sizeof v->evq[0]);
                    if (v->evn < cap) {
                        int at = v->evcur;
                        while (at < v->evn && v->evq[at].id != FQ_EFFECT
                               && v->evq[at].id != FQ_ERRND) at++;
                        for (int m = v->evn; m > at; m--) v->evq[m] = v->evq[m-1];
                        v->evq[at].id = FQ_WAIT; v->evq[at].t = 84;
                        v->evn++;
                    }
                }
                w->evcur++; continue;
            }
            case FQ_ERRND:
                /* the bubble fades and the program moves on -- the reach is
                 * NOT retried on whatever the hands hold by then; a take
                 * emptied mid-bubble advances empty-handed */
                if (w->fresh > 0) w->fresh--;
                S->st_items++;
                w->pc++;
                w->evcur++; continue;
            default: w->evcur++; continue;
        }
    }
    w->evn = w->evcur = 0;
    return true;
}

/* dispatch the instruction at pc: control nodes take their frame, a step
 * starts the walk, everything else queues its timeline */
static void fq_dispatch(Sim *S, Program *P, int i, int now,
                        bool *progressed, int *told) {
    Worker *w = &S->w[i];
    if (w->pc >= P->n) { w->done = true; *progressed = true; return; }
    Instr *ins = &P->instr[w->pc];
    if (w->err_pc != w->pc) {
        w->errx = w->erry = -1; w->err_pc = -1; w->err_t0 = -1;
        w->smem_pc = -1;
    }
    /* EMU_CMDLOG prints when each worker takes up each command -- the way to
     * see one worker's loop length against another's */
    if (getenv("EMU_CMDLOG"))
        fprintf(stderr, "[cmd] t%d w%d pc%d op%d\n", now, i, w->pc, ins->op);
    switch (ins->op) {
        case OP_NOP: case OP_LABEL: w->pc++; *progressed = true; return;
        case OP_JUMP:  w->pc = ins->target; *progressed = true; return;
        case OP_ELSE:  w->pc = ins->target; *progressed = true; return;
        case OP_ENDIF: w->pc++; *progressed = true; return;
        case OP_ENDFOR:
            /* no sweep under way (body jumped into) -> fall out of the loop */
            if (w->fe_idx[P->instr[ins->target].fe_slot] == 0) w->pc++;
            else w->pc = ins->target;
            *progressed = true; return;
        case OP_FOREACH: {
            static const int FE_RANK[9] = { 1, 5, 3, 7, 2, 0, 4, 6, 8 };
            int *fi = &w->fe_idx[ins->fe_slot];
            unsigned char *ord = w->fe_ord[ins->fe_slot];
            if (*fi == 0) {
                for (int k = 0; k < ins->ndirs; k++) ord[k] = (unsigned char)k;
                for (int k = 1; k < ins->ndirs; k++)
                    for (int j = k; j > 0
                         && FE_RANK[ins->dirs[ord[j]]] < FE_RANK[ins->dirs[ord[j-1]]]; j--) {
                        unsigned char t = ord[j]; ord[j] = ord[j-1]; ord[j-1] = t;
                    }
            }
            if (*fi < ins->ndirs) {
                Dir d = ins->dirs[ord[(*fi)++]];
                w->mem[ins->slot].k = MV_TILE;
                w->mem[ins->slot].x = w->x + DX[d];
                w->mem[ins->slot].y = w->y + DY[d];
                w->mem[ins->slot].ntype = -1;
                w->mem[ins->slot].wref = -1;
                w->mem[ins->slot].fedir = true;
                w->pc++;
            } else { *fi = 0; w->pc = ins->target + 1; }
            if (ins->ndirs > 0) {
                /* a full sweep costs one standard command, split evenly
                 * over the directions it names (rounded up per step) */
                int n = ins->ndirs;
                int b = (FQ_FOREACH_BASE + MS_TICK * n - 1) / (MS_TICK * n);
                fq_push(w, FQ_WAIT, (float)(b > 0 ? b : 1));
                w->fready = false;
            }
            *progressed = true; return;
        }
        case OP_LISTEN:
            if (w->heard > 0 && !strcmp(w->heard_word, ins->word))
                { w->heard = 0; w->pc++; }
            *progressed = true; return;        /* waiting costs the frame */
        case OP_STEP: {
            step_dispatch(S, P, i, progressed);
            /* the step command covers its first stretch of ground on the
             * frame it is issued -- the body is already under way */
            if (w->wtx >= 0 && w->wprog == 0) cont_glide(S, P, i);
            if (w->wtx >= 0) w->wsettle = true;
            if (w->wtx >= 0 || w->busy > 0) w->fready = false;
            *progressed = true; return;
        }
        default: break;
    }
    /* a failed item action stops the worker for the error bubble */
    {
        bool err = false;
        if ((ins->op == OP_PICKUP || ins->op == OP_TAKEFROM) && w->holding)
            err = true;
        /* Reaching for a cube that is not there fails just as squarely as
         * reaching with your hands already full: the worker closes on nothing
         * and stands looking at the bubble.  It had been treated as a quiet
         * nothing-happened, which let a worker whose loop speculatively
         * reaches for a square that is usually empty go round far faster than
         * it should -- and on the counting machines that worker is the one
         * pressing the button. */
        else if (ins->op == OP_PICKUP && !w->holding && ins->mem_target < 0) {
            bool found = false;
            for (int k = 0; k < ins->ndirs && !found; k++) {
                int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                if (nx < 0 || ny < 0 || nx >= S->L->w || ny >= S->L->h) continue;
                if (S->grid[ny][nx].has_cube
                    || S->grid[ny][nx].terrain == T_PRINTER) found = true;
            }
            if (!found) err = true;
        }
        else if (ins->op == OP_GIVETO && !w->holding)
            err = true;
        /* and so does writing with nothing in hand -- there must be a cube
         * to write on, and finding none stops the worker just as long */
        else if (ins->op == OP_WRITE && !w->holding)
            err = true;
        else if (ins->op == OP_DROP) {
            if (!w->holding) err = true;
            else if (S->grid[w->y][w->x].terrain == T_FLOOR
                     && S->grid[w->y][w->x].has_cube) err = true;
        }
        if (err) {
            /* The error runs to its end and the program simply moves on --
             * the reach is never retried on whatever the hands hold by
             * then.  A take emptied mid-bubble (robbed from behind, in a
             * bucket brigade) advances empty-handed all the same, which is
             * why a head that reaches while still holding falls out of its
             * loop for good.
             * A POINTED take with full hands looks at the square first and
             * only then stands through the bubble -- measured end to end at
             * one frame under a look plus the bubble */
            bool looked = (ins->op == OP_PICKUP || ins->op == OP_TAKEFROM)
                       && w->holding && ins->mem_target < 0;
            fq_push(w, FQ_SUSPEND, 0);
            if (looked)
                fq_push(w, FQ_WAIT, (float)(FQ_IF_WAIT + FQ_IF_HOLD - 1));
            fq_push(w, FQ_WAIT, (float)MS_ERRB);
            fq_push(w, FQ_ERRND, 0);
            fq_push(w, FQ_RESUME, 0);
            w->fready = false;
            *progressed = true; return;
        }
    }
    /* An item action aimed at a thing walks to it first and acts only on
     * arrival.  What it is aimed at resolves when the command comes up: a
     * remembered target is re-derived as it moves -- the errand can be
     * nudged, and if the remembered thing is gone it chases the next
     * nearest of its kind -- while a direction is read once, where the
     * worker stood, and a machine it lands on owns the errand however far
     * the walk then drifts.  Machines are used from their front square
     * alone; a pickup stands on the remembered square itself; everything
     * else acts from any square beside the target. */
    if (ins->op == OP_PICKUP || ins->op == OP_GIVETO || ins->op == OP_TAKEFROM) {
        bool onto = (ins->op == OP_PICKUP);
        int tx, ty;
        bool have = false, bound = false;
        if (ins->mem_target >= 0)
            have = mem_tile(S, w, ins->mem_target, &tx, &ty);
        else if (!onto)
            have = bound = dir_machine_lock(S, w, ins, now, &tx, &ty);
        if (have) {
            int fx0 = tx, fy0 = ty;
            bool sfront = !onto && machine_front(S, &fx0, &fy0);
            /* an idle press starts on its first sheet as its customer comes
             * within a stride of the front square -- someone bound to it
             * from further off has not started anything yet, and the serve
             * itself dates the first sheet from the arrival instead */
            if (ins->op == OP_TAKEFROM && !w->holding && sfront
                && S->grid[ty][tx].terrain == T_PRINTER
                && S->press_done[ty][tx] == 0
                && abs(w->x - fx0) <= 1 && abs(w->y - fy0) <= 1)
                S->press_done[ty][tx] = now + 36;
            #define FARR() (onto  ? (w->x == tx && w->y == ty) \
                          : sfront ? (w->x == fx0 && w->y == fy0) \
                                  : (abs(w->x - tx) <= 1 && abs(w->y - ty) <= 1))
            bool act = false;
            if (FARR())
                act = bound
                   || !mem_tile_fresh(S, w, ins->mem_target, &tx, &ty) || FARR();
            #undef FARR
            if (!act) {
                int rx = tx, ry = ty;
                bool front = !onto && machine_front(S, &rx, &ry);
                int d = route_step(S, w, rx, ry, front ? false : !onto);
                if (getenv("EMU_ERRLOG"))
                    fprintf(stderr, "[errw] t%d w%d @%d,%d -> %d,%d hop=%d\n",
                            now, i, w->x, w->y, rx, ry, d);
                if (d >= 0) {
                    cont_walk(S, i, w->x + DX[d], w->y + DY[d], false, true);
                    if (w->wtx >= 0 && w->wprog == 0) cont_glide(S, P, i);
                    w->fready = false;
                }
                *progressed = true; return;   /* no route: wait a frame */
            }
        }
    }
    /* A cube still being put down cannot be lifted.  The square shows the
     * cube from the moment the drop begins -- conditions and counters read
     * it there -- but a hand reaching for it closes only once the putting-
     * down is finished.  The reacher stands ready and grabs on the very
     * frame the cube settles, which is what chains a row of workers passing
     * work down a line into lockstep: each one's reach is clocked by the
     * neighbour's release, not by its own loop length. */
    if (ins->op == OP_PICKUP && !w->holding) {
        bool ready = false, settling = false;
        /* the putting-down ends inside the putter's own turn: someone later
         * in the line can lift the cube that same frame, someone earlier has
         * already acted this frame and reaches it one frame on */
        #define SETTLING(t) ((t)->settle + ((t)->settle_by > i ? 1 : 0) > now)
        if (ins->mem_target >= 0) {
            int tx, ty;
            if (mem_tile(S, w, ins->mem_target, &tx, &ty)
                && S->grid[ty][tx].has_cube && SETTLING(&S->grid[ty][tx]))
                settling = true;
        } else {
            for (int k = 0; k < ins->ndirs && !ready; k++) {
                int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                Tile *t = &S->grid[ny][nx];
                if (t->terrain == T_PRINTER) ready = true;
                else if (t->has_cube) {
                    if (SETTLING(t)) settling = true;
                    else ready = true;
                }
            }
        }
        #undef SETTLING
        if (settling && !ready) { *progressed = true; return; }  /* wait for it to land */
    }
    if (ins->op == OP_TELL && (S->L->rules & R_SPEAK_ORDER)) {
        if (*told >= 0) { *progressed = true; return; }   /* retry next frame */
        *told = i;
    }
    switch (ins->op) {
        case OP_IF:
            /* a condition that never looks outward -- memory, held item,
             * numbers, kind words only -- is answered in a single frame:
             * decided on the spot, the program moving on the next frame.
             * It is still a command boundary: a holder spinning on such a
             * condition is between commands every time round, which is what
             * lets a neighbour lift the cube out of their hands -- the
             * fixed-in-place brigade levels hand off exactly this way */
            if (!if_looks(ins)) {
                if (w->fresh > 0) w->fresh--;
                if (if_true(S, ins, w)) w->pc++;
                else w->pc = ins->target +
                         (P->instr[ins->target].op == OP_ELSE ? 1 : 0);
                w->busy = 1;
                *progressed = true; return;
            }
            /* the think bubble: the condition is SAMPLED at the half-way
             * point but the worker stays occupied for the whole standard
             * command -- a branch decided on a world that may change again
             * before the branch is acted on */
            fq_push_look(w);
            break;
        case OP_ASSIGN:
            if (ins->akind == 0) {
                /* nearest is as free as a label: the memory points at the
                 * thing the same frame and the program moves right on */
                if (w->fresh > 0) w->fresh--;
                exec_assign(S, w, ins);
                w->pc++;
                *progressed = true; return;
            }
            if (ins->akind == 2) {
                /* calc runs the long finger-arithmetic; the slot takes the
                 * result only once the sums are done.  The frame spent
                 * taking the command up is part of its length.  An operand
                 * naming a DIRECTION costs one whole look on top -- the same
                 * look an if or a set pays for turning to a neighbouring
                 * square, and like theirs it is paid once, not per operand.
                 * Sums over blanks, numbers, memory and the own hands skip
                 * the look and run a whole look shorter. */
                int ms = MS_CALC + ((ins->op1.kind == 1 || ins->op2.kind == 1)
                                        ? FQ_IF_WAIT + FQ_IF_HOLD - 1 : 0);
                fq_push(w, FQ_WAIT, (float)(ms - 1));
                fq_push(w, FQ_EFFECT, 0);
                break;
            }
            /* set of a DIRECTION is a look: the worker turns to see what is
             * there before remembering it, and the looking is a whole think
             * bubble.  Setting from a number, a memory or the own hands
             * touches nothing out in the world, writes the slot the moment
             * it is spoken, and holds the worker for a single frame --
             * short enough to vanish inside an ordinary program, long
             * enough to be a beat, which is what the scripted speed runs
             * use a row of them for */
            if (ins->akind == 1 && ins->op1.kind == 1) {
                fq_push_look(w);
                break;
            }
            if (w->fresh > 0) w->fresh--;
            exec_assign(S, w, ins);
            w->pc++;
            w->busy = 1;
            *progressed = true; return;
        case OP_TELL:
            /* the word is delivered at the end of the telling; the frame
             * spent taking the command up is part of its length */
            fq_push(w, FQ_WAIT, (float)(MS_TELL > 1 ? MS_TELL - 1 : 1));
            fq_push(w, FQ_EFFECT, 0);
            break;
        case OP_WRITE:
            /* the writing animation runs to its end and the value lands as
             * the pen lifts (empty hands never reach here -- that is an
             * error, handled above); the frame spent taking the command up
             * is part of its length */
            fq_push(w, FQ_ANIM, 55);
            fq_push(w, FQ_WAITANIM, 0);
            fq_push(w, FQ_EFFECT, 0);
            if (MS_WRITE > 57)
                fq_push(w, FQ_WAIT, (float)(MS_WRITE - 57));
            break;
        case OP_PICKUP: case OP_DROP: case OP_GIVETO: case OP_TAKEFROM: {
            if (item_at_machine(S, w, ins)) {
                /* A POINTED takefrom pays its look before the machine
                 * engages, but the look runs from the moment the command was
                 * taken up, so a customer who waited in a queue has looked
                 * long since and pays nothing extra -- only a worker already
                 * standing at the front pays it end to end. */
                int look_left = 0;
                if (ins->op == OP_TAKEFROM && ins->mem_target < 0
                    && w->err_pc == w->pc && w->err_t0 >= 0) {
                    look_left = w->err_t0 + FQ_IF_WAIT + FQ_IF_HOLD + 1 - now;
                    if (look_left < 0) look_left = 0;
                }
                /* a machine holds one customer at a time; the next in line
                 * parks (uncharged) until the machine frees up */
                int mx, my;
                bool machok = machine_target(S, w, ins, &mx, &my);
                if (machok && S->mach_busy[my][mx] > now)
                    { *progressed = true; return; }
                if (ins->op == OP_GIVETO) {
                    if (machok)
                        S->mach_busy[my][mx] = now + MS_SHRED_HOLD;
                    /* Feeding a shredder: lean in, toss it into the maw --
                     * and be on your way at once, while the machine chews on
                     * its own.  How soon the feeder clears out after the
                     * toss decides who reaches the machine next, so that
                     * last number carries a queue's serving order. */
                    fq_push(w, FQ_WAIT, 20);
                    fq_push(w, FQ_EFFECT, 0);
                    fq_push(w, FQ_WAIT, 1);
                } else if (ins->op == OP_PICKUP) {
                    /* reaching bodily into a printer: the sheet is in hand
                     * quickly but the whole arm has to come back out */
                    fq_push(w, FQ_EFFECT, 0);
                    fq_push(w, FQ_ANIM, 137);
                    fq_push(w, FQ_WAITANIM, 0);
                } else {
                    /* Taking from a printer is a serve with a pipeline.
                     * The press runs a 36-frame print; the sheet is thrown
                     * and caught over 40 more and is in the taker's hands
                     * only at the catch.  The press starts the next sheet
                     * as the previous taker steps out IF its next customer
                     * is already stepping in -- a prompt queue rotation
                     * catches a sheet that is nearly done, a straggler
                     * finds the press idle and waits out the full print.
                     * The taker stands engaged for 25 frames after the
                     * catch while it steps out of the front square. */
                    long arr = now + look_left;
                    long pr;
                    if (machok && S->mach_clear[my][mx] > 0) {
                        long pc0 = S->mach_clear[my][mx];
                        pr = (arr - pc0 <= 24 ? pc0 : arr) + 36;
                    } else if (machok && S->press_done[my][mx] > 0) {
                        pr = S->press_done[my][mx];
                    } else {
                        pr = arr + 36;
                    }
                    long grab_at = (pr > arr ? pr : arr) + 40;
                    if (machok) {
                        S->mach_busy[my][mx] = grab_at + 25;
                        S->mach_clear[my][mx] = grab_at + 25;
                    }
                    fq_push(w, FQ_WAIT, (float)(grab_at - now));
                    fq_push(w, FQ_EFFECT, 0);
                }
            } else if (ins->op == OP_TAKEFROM && ins->mem_target < 0) {
                /* what the direction finds shapes the reach, decided once
                 * when the command is taken up: a neighbour with something
                 * in hand is the long reach into their hands; a neighbour
                 * with empty hands costs one look and the hands come back
                 * empty; a hole is the same short look, nothing there to
                 * take; only a bare square (or a cube on the floor, which
                 * is for picking up, not taking) is the look and then the
                 * long red bubble of error -- either way the program then
                 * moves on */
                bool anyone = false, laden = false, hole = false;
                for (int k = 0; k < ins->ndirs; k++) {
                    int nx = w->x + DX[ins->dirs[k]], ny = w->y + DY[ins->dirs[k]];
                    if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                    if (S->grid[ny][nx].terrain == T_HOLE) hole = true;
                    int j = worker_at(S, nx, ny, i);
                    if (j >= 0) {
                        anyone = true;
                        if (S->w[j].holding) laden = true;
                    }
                }
                if (laden) fq_push_steal(w);
                else if (anyone || hole) fq_push_look(w);
                else fq_push_look_error(w);
            } else if (ins->op == OP_TAKEFROM) {
                /* a remembered target: the same reach into their hands, or
                 * the same short look if they stand there empty-handed */
                int tx, ty, j = -1;
                if (mem_tile(S, w, ins->mem_target, &tx, &ty))
                    j = worker_at(S, tx, ty, i);
                if (j >= 0 && S->w[j].holding) fq_push_steal(w);
                else if (j >= 0) fq_push_look(w);
                else fq_push_look_error(w);
            } else if (ins->op == OP_GIVETO) {
                fq_push_throw(w);
            } else {
                /* the hand does its work in the very tick the action starts
                 * -- the cube changes hands (or hits the floor) before the
                 * next worker in line even looks -- and the animation holds
                 * the worker to the end; the frame spent taking the command
                 * up is part of its length */
                bool dropping = (ins->op == OP_DROP) && w->holding
                    && S->grid[w->y][w->x].terrain == T_FLOOR
                    && !S->grid[w->y][w->x].has_cube;
                exec_action(S, P, i);
                fq_push(w, FQ_ANIM, (float)(FQ_ITEM_PRE + FQ_ITEM_TAIL - 1));
                fq_push(w, FQ_WAITANIM, 0);
                /* the cube being put down is on show at once but stays part
                 * of the putting-down until the whole gesture ends: a hand
                 * already waiting for it closes the moment the gesture does */
                if (dropping && !w->holding) {
                    S->grid[w->y][w->x].settle =
                        now + FQ_ITEM_PRE + FQ_ITEM_TAIL;
                    S->grid[w->y][w->x].settle_by = i;
                }
            }
            break;
        }
        default:
            fq_push(w, FQ_EFFECT, 0);
            break;
    }
    w->fready = false;
    *progressed = true;
}

static bool run_frame(Sim *S, Program *P, int *out_rounds) {
    static int cap = 0;
    if (!cap) {
        const char *e = getenv("EMU_CAP");
        cap = e ? atoi(e) : 400000;
        if (cap < 1000) cap = 400000;
    }
    if (level_won(S)) { *out_rounds = 0; return true; }
    for (int i = 0; i < S->nw; i++) {
        Worker *w = &S->w[i];
        w->busy = 0; w->wtx = w->wty = -1;
        w->wintx = w->winty = -1; w->wowned = false;
        w->fx = w->x; w->fy = w->y;
        w->evn = w->evcur = 0; w->animms = 0;
        w->fsusp = false; w->fready = true;
    }
    int now = 0, stall = 0;
    while (now < cap) {
        bool progressed = false, in_flight = false;
        int told = -1;
        S->beat = now;
        S->feeds_this_beat = 0;
        /* the frame's shared picture: everything sensed this frame -- by
         * workers and by the counting machine alike -- is where the bodies
         * stood as the frame began */
        for (int i = 0; i < S->nw; i++) {
            g_snap_x[i] = body_tx(&S->w[i]);
            g_snap_y[i] = body_ty(&S->w[i]);
        }
        g_snap_n = S->nw;
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive || w->exited) continue;
            if (w->done) {
                /* a finished worker still finishes being pushed aside: its
                 * displacement glide has to land or it wedges mid-crossing */
                if (w->wtx >= 0 && cont_glide(S, P, i)) { progressed = true; in_flight = true; }
                continue;
            }
            if (w->animms > 0) { w->animms -= FQ_DT; in_flight = true; }
            if (w->heard > 0) w->heard--;      /* the word fades from the ear */
            if (w->wtx >= 0) {
                if (cont_glide(S, P, i)) progressed = true;
                in_flight = true;
                if (!(w->wtx < 0 && w->alive && !w->done && !w->exited))
                    continue;
                if (w->wsettle) {
                    /* a step's landing is not the program moving on: the body
                     * settles into the square first and the next command is
                     * only taken up a frame later -- which is what lets a
                     * neighbour see the walker standing there before it acts */
                    w->wsettle = false;
                    w->busy = 1;
                    continue;
                }
                w->fready = true;   /* landed: dispatch this same frame */
            }
            if (w->busy > 0) {      /* legacy wait from the step delegate */
                --w->busy;
                in_flight = true;
                if (w->busy > 0) continue;
                w->fready = true;
            }
            if (w->evn > 0) {
                if (!fq_pump(S, P, i)) { in_flight = true; continue; }
                progressed = true;
                if (S->failed) { *out_rounds = now; return false; }
                in_flight = true;
                w->fready = true;
                /* the game takes one more tick to notice a drained timeline:
                 * the next command starts on the FOLLOWING frame, not inside
                 * this one (starting it here was measured against ten long
                 * recorded runs and is worse on every one) */
                continue;
            }
            if (S->failed) { *out_rounds = now; return false; }
            if (w->fsusp) { in_flight = true; continue; }
            if (!w->fready) { w->fready = true; in_flight = true; continue; }
            /* Keep stepping the program while it costs nothing: labels,
             * jumps, endifs and a settled `nearest` are pure bookkeeping,
             * so a worker runs through them and only stops once a command
             * actually starts moving, timing or animating something. */
            for (int guard = 0; guard <= P->n + 8; guard++) {
                int pc0 = w->pc;
                fq_dispatch(S, P, i, now, &progressed, &told);
                if (S->failed) { *out_rounds = now; return false; }
                if (!w->alive || w->done || w->exited) break;
                if (w->evn > 0 || w->wtx >= 0 || w->busy > 0) break;
                if (w->pc == pc0) break;      /* waiting, not advancing */
            }
            if (w->evn > 0 || w->wtx >= 0 || w->busy > 0) in_flight = true;
        }
        /* EMU_POSLOG prints every worker's body position and hands when
         * anything moved -- the way to hold two runs' crowds side by side */
        if (getenv("EMU_POSLOG")) {
            static char pl_last[1024];
            char pl[1024]; int pn = 0;
            for (int i = 0; i < S->nw && pn < (int)sizeof pl - 40; i++) {
                Worker *w = &S->w[i];
                pn += snprintf(pl + pn, sizeof pl - pn, " w%d@%.1f,%.1f%s",
                               i, w->fx, w->fy, w->holding ? "H" : "");
            }
            if (strcmp(pl, pl_last) != 0) {
                fprintf(stderr, "[pos] t%d%s\n", now, pl);
                snprintf(pl_last, sizeof pl_last, "%s", pl);
            }
        }
        now++;
        if (S->L->nsw > 0) counter_press(S);
        if (level_won(S)) { S->win_ms = now; *out_rounds = now; return true; }
        if (S->failed)    { *out_rounds = now; return false; }
        if (!in_flight && !progressed) { if (++stall >= 2) break; } else stall = 0;
    }
    *out_rounds = now;
    if (g_trace) {
        fprintf(stderr, "-- FQ final (now=%d) --\n", now);
        trace_board(S, now);
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            fprintf(stderr, "  w%d (%d,%d)->(%d,%d)%s%s pc=%d ev=%d/%d anim=%.0f%s%s [%s]\n",
                    i, w->x, w->y, w->wtx, w->wty,
                    w->holding ? " hold" : "", w->done ? " done" : "",
                    w->pc, w->evcur, w->evn, w->animms,
                    w->fsusp ? " susp" : "", w->fready ? "" : " !rdy",
                    w->pc < P->n ? P->instr[w->pc].raw : "end");
        }
        /* say WHY the goal was not met, not just that it wasn't: the goal
         * checks explain themselves when asked, and a run that ends short is
         * exactly when that explanation is worth having */
        g_goal_dbg = true;
        (void)level_won(S);
        g_goal_dbg = false;
    }
    return false;
}

static bool run(Sim *S, Program *P, int *out_rounds) {
    bool won = run_frame(S, P, out_rounds);
    g_snap_n = -1;                     /* frame snapshots are its alone */
    return won;
}

int main(int argc, char **argv) {
    if (argc >= 2 && !strcmp(argv[1], "--trace")) { g_trace = true; argv++; argc--; }
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
    long sum_r = 0, sum_sp = 0, sum_items = 0;
    for (int t = 0; t < trials; t++) {
        sim_reset(&S, &L, (unsigned)(t + 1));
        int rounds = 0;
        bool won = run(&S, &P, &rounds);
        if (won) {
            wins++;
            if (min_r < 0 || rounds < min_r) min_r = rounds;
            if (rounds > max_r) max_r = rounds;
            sum_r += rounds;
            int sp = (int)((S.win_ms * 16 + 999) / 1000);  /* 16 ms ticks -> whole seconds */
            if (min_sp < 0 || sp < min_sp) min_sp = sp;
            if (sp > max_sp) max_sp = sp;
            sum_sp += sp;
            sum_items += S.st_items;
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
        printf("rounds  : %d..%d  (frames)\n", min_r, max_r);
        printf("stats   : frames=%.1f items=%.1f (avg over wins)\n",
               (double)sum_r/wins, (double)sum_items/wins);
    }
    /* a solution that wins some trials but not all is a luck-based ("retry
     * until it works") solution -- report it distinctly */
    printf("result  : %s\n", all ? "WIN" : wins ? "PROBABILISTIC" : "FAIL");
    return all ? 0 : wins ? 6 : 1;
}
