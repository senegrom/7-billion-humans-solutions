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

/* ------------------------------------------------------------------ model -- */

enum { MAXW = 64, MAXH = 64, MAXWORKERS = 128, MAXPROG = 4096, MAXLABELS = 256,
       MAXCUBES = 512, MAXDEV = 64 };

typedef enum { T_FLOOR, T_WALL, T_HOLE, T_SHREDDER, T_PRINTER } Terrain;

typedef struct {
    Terrain terrain;
    bool goal;      /* v1: this floor tile is a delivery target */
    bool has_cube;
    int  cube;
} Tile;

typedef enum { CB_FIXED, CB_RAND, CB_RANDU } CubeMode;
typedef struct { int x, y; CubeMode mode; int value; } CubeDef;

typedef struct {
    int  x, y;
    bool holding;
    int  held;
    int  pc;
    bool alive;     /* still on the board (false = destroyed / fell) */
    bool done;      /* program finished (END or ran off the end); stays on board */
    bool exited;    /* left via a hole */
    int  exit_x, exit_y;
    int  tgt_x, tgt_y;   /* aligned-hole-exit target (or -1) */
} Worker;

typedef enum {
    G_CUBES_ON_GOALS, G_SHREDDED_N, G_ALL_EXITED,          /* v1 */
    G_TUT_PICKUP_DROP, G_CUBES_OFFSET, G_ROOM_CLEARED, G_ALIGNED_HOLE_EXIT,
    G_ALL_CUBES_HELD, G_UNZIP, G_SHRED_ALL, G_ALL_WORKERS_HOLDING,
    G_SORTED_ROW, G_ROWS_FILLED, G_LINE_REVERSED, G_ALL_HOLDING_MIN,
    G_UNKNOWN
} GoalKind;

enum { R_NOWALK = 1, R_UNIQUE_SHRED = 2 };   /* enforced special rules */

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
    OP_UNSUPPORTED
} Op;

typedef enum { D_N, D_S, D_E, D_W, D_NE, D_NW, D_SE, D_SW, D_HERE, D_COUNT } Dir;
static const int DX[9] = { 0, 0, 1, -1, 1, -1, 1, -1, 0 };
static const int DY[9] = { -1, 1, 0, 0, -1, -1, 1, 1, 0 };

typedef enum { C_WALL, C_DATACUBE, C_HOLE, C_NOTHING, C_SHREDDER, C_PRINTER, C_PERSON } CmpKind;
typedef enum { O_EQ, O_NE, O_LT, O_GT, O_LE, O_GE } CmpOp;

/* one term of an if-condition: <dir|myitem> <op> <type|number|dir|myitem> */
typedef struct {
    bool lhs_myitem; Dir lhs;
    CmpOp op;
    int  rhs_kind;       /* 0 type, 1 number, 2 dir, 3 myitem */
    CmpKind rhs_type; int rhs_num; Dir rhs_dir;
    int  conn;           /* connector before this term: 0 none, 1 and, 2 or */
} Cond;

typedef struct {
    Op   op;
    Dir  dirs[8];
    int  ndirs;
    int  target;
    Cond conds[8];
    int  nconds;
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
    return G_UNKNOWN;
}

static void load_level(const char *path, Level *L) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open level file");
    memset(L, 0, sizeof *L);
    L->win = G_CUBES_ON_GOALS;
    L->randmax = 99;
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
            if (!strcmp(kind, "wall") || !strcmp(kind, "door"))       L->terr[y][x] = T_WALL;
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
        if (L->cubes[i].mode != CB_FIXED) L->has_random = true;
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
        /* lhs */
        if (!strcmp(tok[i], "myitem")) c->lhs_myitem = true;
        else {
            int d = dir_from(tok[i]);
            if (d < 0) return false;
            c->lhs = (Dir)d;
        }
        /* op */
        if (!cmpop_from(tok[i+1], &c->op)) return false;
        /* rhs */
        const char *r = tok[i+2];
        CmpKind ck;
        int d2;
        if (!strcmp(r, "myitem"))                 { c->rhs_kind = 3; }
        else if (type_from(r, &ck))               { c->rhs_kind = 0; c->rhs_type = ck; }
        else if (isdigit((unsigned char)r[0]) || (r[0]=='-'&&isdigit((unsigned char)r[1])))
                                                  { c->rhs_kind = 1; c->rhs_num = atoi(r); }
        else if ((d2 = dir_from(r)) >= 0)         { c->rhs_kind = 2; c->rhs_dir = (Dir)d2; }
        else return false;
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

    if (!strcmp(verb,"comment"))       ins->op = OP_NOP;
    else if (!strcmp(verb,"endblock")) ins->op = OP_NOP;   /* prefilled-program guard */
    else if (!strncmp(verb,"define",6))ins->op = OP_NOP;   /* DEFINE COMMENT doodles */
    else if (!strcmp(verb,"step"))     { ins->op = OP_STEP;   if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"pickup"))   { ins->op = OP_PICKUP; if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"drop"))     ins->op = OP_DROP;
    else if (!strcmp(verb,"giveto"))   { ins->op = OP_GIVETO;   if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"takefrom")) { ins->op = OP_TAKEFROM; if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"end"))      ins->op = OP_END;
    else if (!strcmp(verb,"jump"))     { ins->op = OP_JUMP; ins->target = arg ? find_or_add_label(P, arg) : -1; }
    else if (!strcmp(verb,"else"))     ins->op = OP_ELSE;
    else if (!strcmp(verb,"endif"))    ins->op = OP_ENDIF;
    else if (!strcmp(verb,"if")) {
        ins->op = OP_IF;
        if (!arg || !parse_cond(ins, arg)) ins->op = OP_UNSUPPORTED;
    }
    else ins->op = OP_UNSUPPORTED;   /* calc/write/set/nearest/tell/listen/foreachdir */

    P->n++;
}

static void link_program(Program *P) {
    int stack[256], sp = 0;
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
        }
    }
    if (sp) die("unclosed if");
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
    while (fgets(line, sizeof line, f)) {
        if (P->n >= MAXPROG) die("program too long");
        rstrip(line);
        char *s = lstrip(line);
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
        default: return NULL;   /* else/endif/labels ride along with if/jump */
    }
}

static void check_palette(const Level *L, const Program *P) {
    if (!L->has_palette) return;
    for (int i = 0; i < P->n; i++) {
        const char *name = op_palette_name(P->instr[i].op);
        if (!name) continue;
        bool ok = false;
        for (int j = 0; j < L->npalette; j++)
            if (!strcmp(L->palette[j], name)) { ok = true; break; }
        if (!ok) {
            fprintf(stderr, "error: command '%s' is not available in this level\n", name);
            exit(3);
        }
    }
}

/* --------------------------------------------------------------- runtime -- */

typedef struct {
    Level  *L;
    Tile    grid[MAXH][MAXW];
    Worker  w[MAXWORKERS];
    int     nw;
    int     shredded;
    long    pickups, drops;
    bool    failed;              /* a special rule was violated */
    int     shred_used[MAXH][MAXW];
    /* per-trial snapshot of the initial cube placement */
    int     icx[MAXCUBES], icy[MAXCUBES], icv[MAXCUBES];
    int     nic;
    unsigned rng;
} Sim;

static unsigned rnd(Sim *S) { S->rng = S->rng * 1664525u + 1013904223u; return S->rng >> 8; }

static void sim_reset(Sim *S, Level *L, unsigned seed) {
    memset(S, 0, sizeof *S);
    S->L = L;
    S->rng = seed * 2654435761u + 12345u;
    for (int y = 0; y < L->h; y++)
        for (int x = 0; x < L->w; x++)
            S->grid[y][x] = (Tile){ L->terr[y][x], L->goalpad[y][x], false, 0 };

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
        if (c->mode == CB_RAND)  v = (int)(rnd(S) % (unsigned)(L->randmax + 1));
        if (c->mode == CB_RANDU) { v = pool[pi]; pi = (pi + 1) % pn; }
        S->grid[c->y][c->x].has_cube = true;
        S->grid[c->y][c->x].cube = v;
        S->icx[S->nic] = c->x; S->icy[S->nic] = c->y; S->icv[S->nic] = v; S->nic++;
    }

    S->nw = L->nworkers;
    for (int i = 0; i < L->nworkers; i++) {
        Worker *w = &S->w[i];
        memset(w, 0, sizeof *w);
        w->x = L->sx[i]; w->y = L->sy[i];
        w->alive = true;
        w->exit_x = w->exit_y = -1;
        w->tgt_x = w->tgt_y = -1;
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
        default: return false;
    }
}

/* Numeric value visible on a tile: a floor cube (even under a worker), else
 * the item held by the worker standing there ("your workers are smart enough"
 * -- in-game tip). */
static bool value_at(Sim *S, int x, int y, const Worker *self, int *out) {
    if (x < 0 || y < 0 || x >= S->L->w || y >= S->L->h) return false;
    if (S->grid[y][x].has_cube) { *out = S->grid[y][x].cube; return true; }
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

static bool cond_true(Sim *S, Cond *c, Worker *w) {
    /* type comparison: does the tile contain the thing? */
    if (c->rhs_kind == 0) {
        if (c->lhs_myitem) return false;   /* myitem vs type: not modeled */
        bool eq = tile_contains(S, w->x + DX[c->lhs], w->y + DY[c->lhs], w, c->rhs_type);
        return (c->op == O_NE) ? !eq : eq;
    }
    /* numeric comparison: both sides are values */
    int a, b;
    bool ha, hb;
    if (c->lhs_myitem) { ha = w->holding; a = w->held; }
    else ha = value_at(S, w->x + DX[c->lhs], w->y + DY[c->lhs], w, &a);
    if (c->rhs_kind == 1)      { hb = true; b = c->rhs_num; }
    else if (c->rhs_kind == 3) { hb = w->holding; b = w->held; }
    else                       hb = value_at(S, w->x + DX[c->rhs_dir], w->y + DY[c->rhs_dir], w, &b);
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

static bool walkable(Sim *S, int x, int y) {
    if (x < 0 || y < 0 || x >= S->L->w || y >= S->L->h) return false;
    Terrain t = S->grid[y][x].terrain;
    return t == T_FLOOR || t == T_HOLE;
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
        case G_UNKNOWN:
            return false;
    }
    return false;
}

/* ---------------------------------------------------------------- round --- */

typedef struct { int action; int tx, ty; } Intent;   /* action: instr index or -1 */

static void fall_check(Sim *S, Worker *w) {
    if (S->grid[w->y][w->x].terrain == T_HOLE) {
        w->alive = false; w->exited = true;
        w->exit_x = w->x; w->exit_y = w->y;
        w->holding = false;                      /* the cube falls with them */
    }
}

static bool g_trace = false;
static bool g_nochain = false;              /* experimental movement variant */

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

/* run one trial to completion; returns win, fills *out_rounds. */
static bool run(Sim *S, Program *P, int *out_rounds) {
    int rounds = 0;
    const int CAP = 200000;
    if (level_won(S)) { *out_rounds = 0; return true; }

    while (rounds < CAP) {
        bool any_active = false;
        int nactors = 0, nidle = 0;
        Intent it[MAXWORKERS];

        /* phase A: every worker flows through control ops and picks its action
         * (sensing sees the pre-move board, so mutual swaps are symmetric).
         * A worker whose control flow finds no action this beat idles (waits). */
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            it[i].action = -1; it[i].tx = -1;
            if (!w->alive || w->done) continue;
            int guard = 0, budget = P->n * 2 + 16;
            bool idle = false;
            for (;;) {
                if (++guard > budget) { idle = true; break; }   /* waiting on a condition */
                if (w->pc >= P->n) { w->done = true; break; }
                Instr *ins = &P->instr[w->pc];
                switch (ins->op) {
                    case OP_NOP: case OP_LABEL: w->pc++; continue;
                    case OP_JUMP: w->pc = ins->target; continue;
                    case OP_IF:
                        /* false: jump into the else body (past the OP_ELSE), or
                         * to the endif when there is no else */
                        if (if_true(S, ins, w)) w->pc++;
                        else w->pc = ins->target +
                                 (P->instr[ins->target].op == OP_ELSE ? 1 : 0);
                        continue;
                    case OP_ELSE: w->pc = ins->target; continue;
                    case OP_ENDIF: w->pc++; continue;
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
            nactors++;
            it[i].action = w->pc;

            Instr *ins = &P->instr[w->pc];
            if (ins->op == OP_STEP) {
                if (S->L->rules & R_NOWALK) { S->failed = true; }
                /* choose among listed dirs: random pick among the passable ones */
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
        }
        if (S->failed) { *out_rounds = rounds; return false; }
        if (!any_active) break;
        /* every active worker is waiting on a condition and nothing can change
         * the board: the simulation is frozen for good */
        if (nactors == 0 && nidle > 0) break;

        /* phase B: resolve movement simultaneously (swaps, chains, bumps) */
        bool resolved[MAXWORKERS] = { false };
        if (g_nochain) {
            /* variant: a move only succeeds into a tile that was free at beat
             * start; the sole exception is the mutual swap */
            int px[MAXWORKERS], py[MAXWORKERS];
            for (int i = 0; i < S->nw; i++) { px[i] = S->w[i].x; py[i] = S->w[i].y; }
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->alive || w->done || resolved[i]) continue;
                if (it[i].action < 0 || P->instr[it[i].action].op != OP_STEP) continue;
                if (it[i].tx < 0) continue;
                int occ = -1;
                for (int j = 0; j < S->nw; j++)
                    if (j != i && S->w[j].alive && px[j] == it[i].tx && py[j] == it[i].ty) { occ = j; break; }
                if (occ < 0) {
                    if (worker_at(S, it[i].tx, it[i].ty, i) < 0) {
                        w->x = it[i].tx; w->y = it[i].ty; resolved[i] = true;
                    }
                } else if (!resolved[occ] && it[occ].action >= 0
                           && P->instr[it[occ].action].op == OP_STEP
                           && it[occ].tx == px[i] && it[occ].ty == py[i]) {
                    Worker *o = &S->w[occ];
                    o->x = it[occ].tx; o->y = it[occ].ty;
                    w->x = it[i].tx; w->y = it[i].ty;
                    resolved[i] = resolved[occ] = true;
                }
            }
        } else
        for (;;) {
            bool progress = false;
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                if (!w->alive || w->done || resolved[i]) continue;
                if (it[i].action < 0 || P->instr[it[i].action].op != OP_STEP) continue;
                if (it[i].tx < 0) continue;
                int occ = worker_at(S, it[i].tx, it[i].ty, i);
                if (occ < 0) {
                    w->x = it[i].tx; w->y = it[i].ty;
                    resolved[i] = true; progress = true;
                } else if (!resolved[occ] && it[occ].action >= 0
                           && P->instr[it[occ].action].op == OP_STEP
                           && it[occ].tx == w->x && it[occ].ty == w->y) {
                    /* mutual swap: both trying to walk into each other */
                    Worker *o = &S->w[occ];
                    int ox = o->x, oy = o->y;
                    o->x = it[occ].tx; o->y = it[occ].ty;
                    w->x = ox; w->y = oy;
                    resolved[i] = resolved[occ] = true; progress = true;
                }
            }
            if (!progress) break;
        }
        for (int i = 0; i < S->nw; i++)
            if (S->w[i].alive && !S->w[i].done && it[i].action >= 0
                && P->instr[it[i].action].op == OP_STEP) {
                S->w[i].pc++;
                fall_check(S, &S->w[i]);
            }

        /* phase C: non-movement actions on the moved board. Two sub-passes:
         * hand-away actions (drop/giveto/pickup/end) resolve before takefrom,
         * so a cube being given away this beat cannot also be taken. */
        for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive || w->done || it[i].action < 0) continue;
            Instr *ins = &P->instr[it[i].action];
            if (ins->op == OP_STEP) continue;
            if ((ins->op == OP_TAKEFROM) != (pass == 1)) continue;
            switch (ins->op) {
                case OP_PICKUP: {
                    if (!w->holding) {
                        for (int k = 0; k < ins->ndirs; k++) {
                            Dir d = ins->dirs[k];
                            int nx = w->x + DX[d], ny = w->y + DY[d];
                            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                            Tile *t = &S->grid[ny][nx];
                            if (t->terrain == T_PRINTER) {   /* fresh print */
                                w->holding = true;
                                w->held = (int)(rnd(S) % (unsigned)(S->L->randmax + 1));
                                S->pickups++;
                                break;
                            }
                            if (t->has_cube) {
                                w->holding = true; w->held = t->cube; t->has_cube = false;
                                S->pickups++;
                                break;
                            }
                        }
                    }
                    break;
                }
                case OP_DROP: {
                    Tile *t = &S->grid[w->y][w->x];
                    if (w->holding && !t->has_cube) {
                        t->has_cube = true; t->cube = w->held; w->holding = false;
                        S->drops++;
                    }
                    break;
                }
                case OP_GIVETO: {
                    if (w->holding && ins->ndirs > 0) {
                        Dir d = ins->dirs[0];
                        int nx = w->x + DX[d], ny = w->y + DY[d];
                        if (nx>=0&&ny>=0&&nx<S->L->w&&ny<S->L->h) {
                            if (S->grid[ny][nx].terrain == T_SHREDDER) {
                                if ((S->L->rules & R_UNIQUE_SHRED) && S->shred_used[ny][nx]) {
                                    w->alive = false;       /* violently destroyed */
                                    w->holding = false;
                                } else {
                                    S->shred_used[ny][nx]++;
                                    w->holding = false; S->shredded++;
                                }
                            } else {
                                int j = worker_at(S, nx, ny, i);
                                if (j >= 0 && !S->w[j].holding) {
                                    S->w[j].holding = true; S->w[j].held = w->held;
                                    w->holding = false;
                                }
                            }
                        }
                    }
                    break;
                }
                case OP_TAKEFROM: {
                    if (!w->holding && ins->ndirs > 0) {
                        for (int k = 0; k < ins->ndirs; k++) {
                            Dir d = ins->dirs[k];
                            int nx = w->x + DX[d], ny = w->y + DY[d];
                            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                            if (S->grid[ny][nx].terrain == T_PRINTER) {
                                w->holding = true;
                                w->held = (int)(rnd(S) % (unsigned)(S->L->randmax + 1));
                                break;
                            }
                            int j = worker_at(S, nx, ny, i);
                            if (j >= 0 && S->w[j].holding) {
                                w->holding = true; w->held = S->w[j].held;
                                S->w[j].holding = false;
                                break;
                            }
                        }
                    }
                    break;
                }
                case OP_END:
                    w->done = true;
                    break;
                default: break;
            }
            if (!w->done || ins->op == OP_END) w->pc++;
        }

        rounds++;
        if (g_trace && rounds <= 60) {
            trace_board(S, rounds);
            for (int i = 0; i < S->nw; i++) {
                Worker *w = &S->w[i];
                fprintf(stderr, "  w%d (%d,%d)%s%s%s pc=%d act=[%s]\n", i, w->x, w->y,
                        w->holding?" hold":"", w->done?" done":"", w->alive?"":" dead",
                        w->pc, it[i].action >= 0 ? P->instr[it[i].action].raw : "-");
            }
        }
        if (level_won(S)) { *out_rounds = rounds; return true; }
        if (S->failed)    { *out_rounds = rounds; return false; }
    }
    *out_rounds = rounds;
    return level_won(S);
}

int main(int argc, char **argv) {
    if (argc >= 2 && !strcmp(argv[1], "--trace")) { g_trace = true; argv++; argc--; }
    if (getenv("EMU_RIGHTASSOC")) g_rightassoc = true;
    if (getenv("EMU_NOCHAIN")) g_nochain = true;
    if (getenv("EMU_NOTHING_IGNORES_WORKERS")) g_nothing_ignores_workers = true;
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
                    fprintf(stderr, " %s{%s%d op%d rhs%d t%d n%d}",
                            c->conn==1?"AND ":c->conn==2?"OR ":"",
                            c->lhs_myitem?"myitem":"dir", c->lhs_myitem?0:(int)c->lhs,
                            (int)c->op, c->rhs_kind, (int)c->rhs_type, c->rhs_num);
                }
                fprintf(stderr, "\n");
            }
    }

    bool prog_random = false;    /* multi-direction steps choose randomly */
    for (int i = 0; i < P.n; i++)
        if (P.instr[i].op == OP_STEP && P.instr[i].ndirs > 1) prog_random = true;
    int trials = (argc == 4) ? atoi(argv[3]) : ((L.has_random || prog_random) ? 20 : 1);
    if (trials < 1) trials = 1;

    static Sim S;
    int wins = 0, min_r = -1, max_r = -1, first_fail = -1;
    for (int t = 0; t < trials; t++) {
        sim_reset(&S, &L, (unsigned)(t + 1));
        int rounds = 0;
        bool won = run(&S, &P, &rounds);
        if (won) {
            wins++;
            if (min_r < 0 || rounds < min_r) min_r = rounds;
            if (rounds > max_r) max_r = rounds;
        } else if (first_fail < 0) first_fail = t + 1;
    }

    bool all = (wins == trials);
    printf("level   : %s (%dx%d, %d worker%s, %d cube%s)\n", L.name, L.w, L.h,
           L.nworkers, L.nworkers==1?"":"s", L.ncubes, L.ncubes==1?"":"s");
    printf("solution: %s\n", argv[2]);
    printf("size    : %d commands\n", program_size(&P));
    printf("trials  : %d, wins %d%s\n", trials, wins,
           first_fail > 0 ? " (first fail: trial seed above)" : "");
    if (wins)
        printf("rounds  : %d..%d  (raw lockstep rounds; NOT the game's speed metric)\n", min_r, max_r);
    /* a solution that wins some trials but not all is a luck-based ("retry
     * until it works") solution -- report it distinctly */
    printf("result  : %s\n", all ? "WIN" : wins ? "PROBABILISTIC" : "FAIL");
    return all ? 0 : wins ? 6 : 1;
}
