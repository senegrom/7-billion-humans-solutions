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
 * Status: the parser and the movement / item mechanics below are faithful to
 * the game for the commands that are unambiguous (step, pickUp, drop, giveTo,
 * jump, if/else/endif, labels, directional sensing) -- verified against the
 * community's known-good solutions. Commands whose exact semantics still need
 * pinning (takeFrom, calc, write, set, nearest, tell, listen, forEachDir) are
 * parsed but refuse to run rather than silently mis-simulate. Level layouts
 * here are hand-authored, self-consistent reconstructions (a level's known-good
 * solution wins, obvious mutations lose) -- not the game's own level data.
 *
 * IMPORTANT CAVEAT ON "SPEED": the round count this prints is NOT the game's
 * Speed score. The game's speed metric is not "lines executed" (e.g. Year 2
 * scores speed 1, Year 46 scores speed 0) and must be calibrated against the
 * real game. We print raw rounds only as a coarse progress signal.
 *
 * Build:  gcc -std=c11 -O2 -Wall -Wextra -o emu emu.c
 * Run:    ./emu <level.lvl> <solution.txt>
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ model -- */

enum { MAXW = 64, MAXH = 64, MAXWORKERS = 128, MAXPROG = 4096, MAXLABELS = 256 };

/* Tile terrain. A data cube sits *on* a FLOOR/GOAL tile (see Tile.cube). */
typedef enum { T_FLOOR, T_WALL, T_HOLE, T_SHREDDER, T_PRINTER } Terrain;

typedef struct {
    Terrain terrain;
    bool goal;      /* this floor tile is a delivery target */
    bool has_cube;  /* a data cube is lying on this tile */
    int  cube;      /* its value (if has_cube) */
} Tile;

typedef struct {
    int  x, y;
    bool holding;
    int  held;      /* value in hand (if holding) */
    int  pc;
    bool alive;     /* still on the board */
    bool exited;    /* left via a hole (some levels require this) */
} Worker;

typedef enum { WIN_CUBES_ON_GOALS, WIN_SHREDDED, WIN_ALL_EXITED } WinKind;

typedef struct {
    char    name[64];
    int     w, h;
    Tile    grid[MAXH][MAXW];
    Worker  spawn[MAXWORKERS];
    int     nworkers;
    WinKind win;
    int     win_arg;    /* e.g. required shred count */
} Level;

/* --------------------------------------------------------------- program -- */

typedef enum {
    OP_STEP, OP_PICKUP, OP_DROP, OP_GIVETO, OP_TAKEFROM,
    OP_JUMP, OP_IF, OP_ELSE, OP_ENDIF, OP_LABEL, OP_NOP,
    OP_UNSUPPORTED
} Op;

/* 8 compass directions; DIR_HERE is the pseudo-direction 'c' (current tile). */
typedef enum { D_N, D_S, D_E, D_W, D_NE, D_NW, D_SE, D_SW, D_HERE, D_COUNT } Dir;
static const int DX[9] = { 0, 0, 1, -1, 1, -1, 1, -1, 0 };
static const int DY[9] = { -1, 1, 0, 0, -1, -1, 1, 1, 0 };

typedef enum { C_WALL, C_DATACUBE, C_HOLE, C_NOTHING, C_SHREDDER, C_PRINTER, C_PERSON, C_NUM } CmpKind;

typedef struct {
    Op   op;
    Dir  dirs[8];     /* for STEP/PICKUP/GIVETO/TAKEFROM: candidate directions */
    int  ndirs;
    int  target;      /* IF/ELSE -> matching jump index; JUMP -> label line */
    Dir  sense;       /* IF: the tile direction being queried */
    bool negate;      /* IF: '!=' vs '==' */
    CmpKind cmp;      /* IF: what we compare against */
    int  cmp_num;     /* IF: numeric value when cmp == C_NUM */
    char raw[96];     /* original text, for diagnostics */
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
        while (*p && *p != '"') p++;      /* to opening quote of the value */
        if (!*p) break;
        p++;
        char root[1024];
        size_t i = 0;
        while (*p && *p != '"' && i < sizeof root - 1) {
            if (*p == '\\' && p[1] == '\\') { root[i++] = '\\'; p += 2; }  /* unescape \\ */
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
    /* Save profile -- location-independent proof the game has been run. */
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

    /* Steam install, any drive. */
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

/* Text level format (see the levels directory and tools/emu/README). */
static void load_level(const char *path, Level *L) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open level file");
    memset(L, 0, sizeof *L);
    L->win = WIN_CUBES_ON_GOALS;
    char line[256];
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
        } else if (!strncmp(s, "row ", 4)) {
            char *r = s + 4;
            for (int x = 0; x < L->w && r[x]; x++) {
                Tile *t = &L->grid[row][x];
                char c = r[x];
                switch (c) {
                    case '.': t->terrain = T_FLOOR; break;
                    case '#': t->terrain = T_WALL; break;
                    case 'O': t->terrain = T_HOLE; break;
                    case 'S': t->terrain = T_SHREDDER; break;
                    case 'P': t->terrain = T_PRINTER; break;
                    case 'G': t->terrain = T_FLOOR; t->goal = true; break;
                    case '@':
                        t->terrain = T_FLOOR;
                        L->spawn[L->nworkers].x = x;
                        L->spawn[L->nworkers].y = row;
                        L->nworkers++;
                        break;
                    default:
                        if (isalnum((unsigned char)c)) {
                            t->terrain = T_FLOOR;
                            t->has_cube = true;
                            t->cube = isdigit((unsigned char)c) ? c - '0' : toupper(c);
                        } else die("bad grid char");
                }
            }
            row++;
        } else if (!strncmp(s, "goal ", 5)) {
            char *g = s + 5;
            if (!strncmp(g, "cubes_on_goals", 14)) L->win = WIN_CUBES_ON_GOALS;
            else if (!strncmp(g, "shredded", 8)) { L->win = WIN_SHREDDED; L->win_arg = atoi(g + 8); }
            else if (!strncmp(g, "all_exited", 10)) L->win = WIN_ALL_EXITED;
            else die("unknown goal");
        }
    }
    fclose(f);
    if (L->w == 0 || L->h == 0) die("level missing dim");
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

static CmpKind cmp_from(const char *t, int *num) {
    if (!strcmp(t,"wall")) return C_WALL;
    if (!strcmp(t,"datacube")||!strcmp(t,"cube")) return C_DATACUBE;
    if (!strcmp(t,"hole")) return C_HOLE;
    if (!strcmp(t,"nothing")) return C_NOTHING;
    if (!strcmp(t,"shredder")) return C_SHREDDER;
    if (!strcmp(t,"printer")) return C_PRINTER;
    if (!strcmp(t,"person")||!strcmp(t,"worker")) return C_PERSON;
    if (isdigit((unsigned char)t[0]) || t[0]=='-') { *num = atoi(t); return C_NUM; }
    return (CmpKind)-1;
}

/* Turn one source line into an Instr (or OP_NOP for blanks/comments/headers). */
static void parse_line(Program *P, char *src) {
    Instr *ins = &P->instr[P->n];
    memset(ins, 0, sizeof *ins);
    ins->op = OP_NOP;
    char *s = lstrip(rstrip(src));
    snprintf(ins->raw, sizeof ins->raw, "%s", s);

    if (*s == 0) { P->n++; return; }
    if (s[0]=='-' && s[1]=='-') { P->n++; return; }      /* -- header/comment -- */

    /* label:  (bare identifier ending in ':') */
    size_t len = strlen(s);
    if (len >= 2 && s[len-1]==':' && !strchr(s, ' ')) {
        s[len-1] = 0;
        int li = find_or_add_label(P, s);
        P->label_line[li] = P->n;
        ins->op = OP_LABEL;
        P->n++;
        return;
    }

    /* split verb + argument */
    char verb[32] = {0};
    char *sp = strpbrk(s, " \t");
    char *arg = NULL;
    if (sp) { size_t vl = (size_t)(sp - s); if (vl > 31) vl = 31; memcpy(verb, s, vl); arg = lstrip(sp); }
    else strncpy(verb, s, 31);

    /* an 'if' condition ends with ':' -> strip it */
    if (arg) { char *colon = strrchr(arg, ':'); if (colon && colon[1]==0) *colon = 0; }
    for (char *p = verb; *p; p++) *p = (char)tolower((unsigned char)*p);

    if (!strcmp(verb,"comment"))       ins->op = OP_NOP;
    else if (!strcmp(verb,"endblock")) ins->op = OP_NOP;   /* prefilled-program guard */
    else if (!strcmp(verb,"step"))     { ins->op = OP_STEP;   if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"pickup")||!strcmp(verb,"pickUp")) { ins->op = OP_PICKUP; if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"drop"))     ins->op = OP_DROP;
    else if (!strcmp(verb,"giveto")||!strcmp(verb,"giveTo"))     { ins->op = OP_GIVETO;   if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"takefrom")||!strcmp(verb,"takeFrom")) { ins->op = OP_TAKEFROM; if (arg) parse_dirs(ins, arg); }
    else if (!strcmp(verb,"jump"))     { ins->op = OP_JUMP; ins->target = arg ? find_or_add_label(P, arg) : -1; }
    else if (!strcmp(verb,"else"))     ins->op = OP_ELSE;
    else if (!strcmp(verb,"endif"))    ins->op = OP_ENDIF;
    else if (!strcmp(verb,"if")) {
        ins->op = OP_IF;
        /* forms: "<dir> == <type>" / "<dir> != <type|num>" */
        char lhs[16]={0}, ops[4]={0}, rhs[16]={0};
        if (arg && sscanf(arg, "%15s %3s %15s", lhs, ops, rhs) == 3) {
            int d = dir_from(lhs);
            int num = 0;
            CmpKind ck = cmp_from(rhs, &num);
            if (d < 0 || d == D_HERE || (int)ck < 0) ins->op = OP_UNSUPPORTED;
            else { ins->sense = (Dir)d; ins->negate = !strcmp(ops,"!="); ins->cmp = ck; ins->cmp_num = num; }
        } else ins->op = OP_UNSUPPORTED;
    }
    else ins->op = OP_UNSUPPORTED;   /* calc/write/set/nearest/tell/listen/foreachdir */

    P->n++;
}

/* Resolve if/else/endif nesting to jump targets, and bind label lines. */
static void link_program(Program *P) {
    int stack[256], sp = 0;
    for (int i = 0; i < P->n; i++) {
        Op op = P->instr[i].op;
        if (op == OP_IF) { stack[sp++] = i; }
        else if (op == OP_ELSE) {
            if (!sp) die("else without if");
            P->instr[stack[sp-1]].target = i;   /* if -> else */
            stack[sp-1] = i;                     /* remember else for endif */
        } else if (op == OP_ENDIF) {
            if (!sp) die("endif without if");
            P->instr[stack[--sp]].target = i;    /* if/else -> endif */
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

static void load_program(const char *path, Program *P) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open solution file");
    memset(P, 0, sizeof *P);
    char line[512];
    while (fgets(line, sizeof line, f)) {
        if (P->n >= MAXPROG) die("program too long");
        parse_line(P, line);
    }
    fclose(f);
    link_program(P);
}

/* Count real commands (SIZE metric): everything that isn't a blank, a header,
 * a bare label, or a comment doodle. This matches how the game counts size. */
static int program_size(const Program *P) {
    int n = 0;
    for (int i = 0; i < P->n; i++) {
        Op op = P->instr[i].op;
        if (op==OP_NOP || op==OP_LABEL) continue;
        n++;
    }
    return n;
}

/* --------------------------------------------------------------- runtime -- */

typedef struct { Level *L; Worker w[MAXWORKERS]; int nw; int shredded; } Sim;

/* What does the worker sense in a given tile? Mirrors the game's rule that a
 * cube held up by a worker does NOT make the tile read as a datacube. */
static CmpKind sense_tile(Sim *S, int x, int y) {
    Level *L = S->L;
    if (x < 0 || y < 0 || x >= L->w || y >= L->h) return C_WALL;
    Tile *t = &L->grid[y][x];
    for (int i = 0; i < S->nw; i++)
        if (S->w[i].alive && S->w[i].x == x && S->w[i].y == y) return C_PERSON;
    switch (t->terrain) {
        case T_WALL:     return C_WALL;
        case T_HOLE:     return C_HOLE;
        case T_SHREDDER: return C_SHREDDER;
        case T_PRINTER:  return C_PRINTER;
        case T_FLOOR:    return t->has_cube ? C_DATACUBE : C_NOTHING;
    }
    return C_NOTHING;
}

static bool if_true(Sim *S, Instr *ins, Worker *w) {
    int x = w->x + DX[ins->sense], y = w->y + DY[ins->sense];
    bool eq;
    if (ins->cmp == C_NUM) {
        Tile *t = (x<0||y<0||x>=S->L->w||y>=S->L->h) ? NULL : &S->L->grid[y][x];
        eq = t && t->has_cube && t->cube == ins->cmp_num;
    } else {
        eq = (sense_tile(S, x, y) == ins->cmp);
    }
    return ins->negate ? !eq : eq;
}

static bool occupied(Sim *S, int x, int y, int self) {
    for (int i = 0; i < S->nw; i++)
        if (i != self && S->w[i].alive && S->w[i].x == x && S->w[i].y == y) return true;
    return false;
}

/* Has the level's win condition been met given the current board state? */
static bool level_won(Sim *S) {
    Level *L = S->L;
    switch (L->win) {
        case WIN_CUBES_ON_GOALS:
            for (int y = 0; y < L->h; y++)
                for (int x = 0; x < L->w; x++)
                    if (L->grid[y][x].goal && !L->grid[y][x].has_cube) return false;
            return true;
        case WIN_SHREDDED:
            return S->shredded >= L->win_arg;
        case WIN_ALL_EXITED:
            for (int i = 0; i < S->nw; i++)
                if (!S->w[i].exited) return false;
            return true;
    }
    return false;
}

/* run to completion; returns true on win. Fills *out_rounds. */
static bool run(Sim *S, Program *P, int *out_rounds) {
    int rounds = 0;
    const int CAP = 100000;
    if (level_won(S)) { *out_rounds = 0; return true; }

    while (rounds < CAP) {
        bool any_alive = false;
        for (int i = 0; i < S->nw; i++) {
            Worker *w = &S->w[i];
            if (!w->alive) continue;

            /* flow through control ops until this worker performs one action */
            int guard = 0;
            for (;;) {
                if (++guard > 100000) die("control-flow loop with no action");
                if (w->pc >= P->n) { w->alive = false; break; }   /* halt */
                Instr *ins = &P->instr[w->pc];
                switch (ins->op) {
                    case OP_NOP: case OP_LABEL: w->pc++; continue;
                    case OP_JUMP: w->pc = ins->target; continue;
                    case OP_IF:   w->pc = if_true(S, ins, w) ? w->pc + 1 : ins->target; continue;
                    case OP_ELSE: w->pc = ins->target; continue;   /* reached after true-branch: jump to endif */
                    case OP_ENDIF: w->pc++; continue;
                    case OP_UNSUPPORTED:
                        fprintf(stderr, "error: unsupported command: %s\n", ins->raw);
                        exit(3);
                    default: break;   /* an action op: fall out to perform it */
                }
                break;
            }
            if (!w->alive) continue;
            any_alive = true;

            Instr *ins = &P->instr[w->pc];
            int did_action = 1;
            switch (ins->op) {
                case OP_STEP: {
                    int chosen = -1;
                    for (int k = 0; k < ins->ndirs; k++) {   /* first legal dir (deterministic) */
                        Dir d = ins->dirs[k];
                        int nx = w->x + DX[d], ny = w->y + DY[d];
                        if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                        Terrain tr = S->L->grid[ny][nx].terrain;
                        if (tr==T_WALL || tr==T_PRINTER) continue;
                        if (occupied(S, nx, ny, i)) continue;
                        chosen = d; break;
                    }
                    if (chosen >= 0) {
                        w->x += DX[chosen]; w->y += DY[chosen];
                        if (S->L->grid[w->y][w->x].terrain == T_HOLE) { w->alive = false; w->exited = true; }
                    }
                    break;
                }
                case OP_PICKUP: {
                    if (!w->holding) {
                        for (int k = 0; k < ins->ndirs; k++) {
                            Dir d = ins->dirs[k];
                            int nx = w->x + DX[d], ny = w->y + DY[d];
                            if (nx<0||ny<0||nx>=S->L->w||ny>=S->L->h) continue;
                            Tile *t = &S->L->grid[ny][nx];
                            if (t->has_cube) { w->holding = true; w->held = t->cube; t->has_cube = false; break; }
                        }
                    }
                    break;
                }
                case OP_DROP: {
                    Tile *t = &S->L->grid[w->y][w->x];
                    if (w->holding && !t->has_cube) { t->has_cube = true; t->cube = w->held; w->holding = false; }
                    break;
                }
                case OP_GIVETO: {
                    if (w->holding && ins->ndirs > 0) {
                        Dir d = ins->dirs[0];
                        int nx = w->x + DX[d], ny = w->y + DY[d];
                        if (nx>=0&&ny>=0&&nx<S->L->w&&ny<S->L->h) {
                            Terrain tr = S->L->grid[ny][nx].terrain;
                            if (tr == T_SHREDDER) { w->holding = false; S->shredded++; }
                            else {
                                for (int j = 0; j < S->nw; j++)
                                    if (S->w[j].alive && S->w[j].x==nx && S->w[j].y==ny && !S->w[j].holding) {
                                        S->w[j].holding = true; S->w[j].held = w->held; w->holding = false; break;
                                    }
                            }
                        }
                    }
                    break;
                }
                case OP_TAKEFROM:
                    /* printers/worker hand-off source semantics unverified: refuse */
                    fprintf(stderr, "error: takefrom not yet modeled\n"); exit(3);
                default: did_action = 0; break;
            }
            w->pc++;
            (void)did_action;
        }

        rounds++;
        if (level_won(S)) { *out_rounds = rounds; return true; }
        if (!any_alive) break;
    }
    *out_rounds = rounds;
    return level_won(S);
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "usage: %s <level.lvl> <solution.txt>\n", argv[0]); return 1; }
    require_game();
    static Level L; static Program P;
    load_level(argv[1], &L);
    load_program(argv[2], &P);

    Sim S; memset(&S, 0, sizeof S); S.L = &L; S.nw = L.nworkers;
    for (int i = 0; i < L.nworkers; i++) {
        S.w[i] = L.spawn[i]; S.w[i].alive = true; S.w[i].pc = 0;
    }

    int rounds = 0;
    bool won = run(&S, &P, &rounds);

    printf("level   : %s (%dx%d, %d worker%s)\n", L.name, L.w, L.h, L.nworkers, L.nworkers==1?"":"s");
    printf("solution: %s\n", argv[2]);
    printf("size    : %d commands\n", program_size(&P));
    printf("rounds  : %d  (raw lockstep rounds; NOT the game's speed metric)\n", rounds);
    printf("result  : %s\n", won ? "WIN" : "FAIL");
    return won ? 0 : 1;
}
