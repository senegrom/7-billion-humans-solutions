# 7BH emulator

A small C simulator that runs a 7 Billion Humans solution against a level and
reports whether it wins and how big it is. Purpose: machine-check candidate
solutions instead of clicking through the game.

This is a companion tool for people who **own** 7 Billion Humans. It reads none
of the game's files, but it refuses to run unless the game is installed — it is
only useful to owners, by design.

## Requires the game

At startup the emulator checks that 7 Billion Humans is installed (a Steam
install on any drive, or an existing save profile). If you own it but it isn't
detected, point at it explicitly:

```bash
export SEVENBH_GAME="D:/SteamLibrary/steamapps/common/7 Billion Humans/7 Billion Humans.exe"
```

## Build & test

```bash
bash build.sh              # -> ./emu.exe   (needs gcc; MinGW-w64 on Windows)
bash tests/run_tests.sh    # regression suite (must be all-green)
```

## Run

```bash
./emu.exe levels/year03_transport.lvl tests/year03.txt
```

Output reports `size` (command count — the game's SIZE metric) and `result`
(WIN/FAIL). It also prints raw lockstep `rounds`, which is **not** the game's
Speed score (see the caveat below).

## Level formats (`levels/*.lvl`)

v1 -- character grids, used by the hand-authored regression levels:

```
name <title>
dim  <width> <height>
row  <chars>          # one per grid row, top to bottom
...
goal <condition>
```

Grid characters: `.` floor · `#` wall · `O` hole/pit · `S` shredder ·
`P` printer · `G` goal pad (floor that must end holding a cube) · `@` worker
spawn · a digit/letter = a data cube of that value sitting on floor.

v2 -- sparse entity lists, for full-size levels (cube values above 9, random
cubes, per-level command palettes). Owners generate these locally for the real
game levels; none ship with this repo.

```
name <title>
dim <w> <h>
randmax <n>                  # max random cube value (default 99)
commands <name...>           # the level's command palette (enforced)
rule <name>                  # special rule, e.g. nowalk, unique_shredder_use
ent wall|hole|shredder|printer|worker|door|sign <x> <y>
ent cube <x> <y> <value|rand|randu>   # rand = uniform, randu = distinct draws
goal <predicate [args]>
```

Win predicates implemented so far: `cubes_on_goals`, `shredded <n>`,
`all_exited`, `tutorial_pickup_drop`, `cubes_offset <dx> <dy>`, `room_cleared`,
`aligned_hole_exit`, `all_cubes_held`, `unzip`, `shred_all [alive_all]`,
`all_workers_holding`, `sorted_row_holdable`, `rows_gaps_filled`,
`line_reversed`, `all_holding_min <n>`. Unknown goals refuse to run.

Randomized levels (or programs with multi-direction steps) run over many seeded
trials; `result` is `WIN` (all trials), `PROBABILISTIC` (some -- a luck-based
solution the game would accept on a lucky run), or `FAIL`.

## What is faithful, and what isn't

Faithful (validated by running the community repo's known-good solutions for
18 of 20 early-campaign cases against real level geometry):

- Parser: labels, `comment`, `-- headers --`, comma-separated direction lists,
  multi-line `if` conditions chained with `and`/`or` (evaluated left to right),
  comparisons `== != < > <= >=` over tile types, numbers, `myitem`, and other
  tiles' values, `else`/`else:`, `jump`, `end`.
- Mechanics: `step` (multi-direction = random pick), `pickUp` (own tile,
  adjacent tiles, printers), `drop`, `giveTo` (shredder + worker hand-off),
  `takeFrom` (printer + worker), the walk-swap rule (two workers stepping into
  each other exchange tiles), waiting on unmet conditions, holes destroying
  what falls in, wall/occupancy blocking.
- Sensing: a tile is a *set* of contents -- a worker standing on a floor cube
  matches both `worker` and `datacube`; a cube held up in the air is not a
  floor cube; numeric reads see the floor cube first, else the held item of
  the worker standing there.
- SIZE: every command except labels, comments, `else`, and `endif` (the latter
  two are part of the `if` block in the game's editor).

Not yet faithful / deliberately refused at runtime rather than guessed:

- `calc`, `write`, `set`, `nearest`, `tell`, `listen`, `forEachDir` — parsed
  but abort execution (no silent mis-simulation).
- **The Speed metric.** The game's Speed is not "lines executed" (Year 2 = 1,
  Year 46 = 0); it needs calibration against the real game before we can rank
  speed solutions. We print raw rounds only as a coarse signal.
- Exact multi-worker collision timing in dense choreography (one known case:
  the Injection Sites 2 community solution's sidestep dance resolves
  differently; needs in-game observation).

`--trace` prints the board and per-worker actions each round for debugging.
