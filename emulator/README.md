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

Win predicates implemented: the full campaign set except the three
counting-machine levels (`binary_counter`/`decimal_counter`/`decimal_doubler`,
whose display hardware is not in the level grid) — from `tutorial_pickup_drop`
and `shred_all [alive_all]` through `sorted_row_holdable`, `email_sort`,
`mult_table`, `fashion_unique`, `mode_counts`, `flower_sums`,
`neighbor_counts`, `glory_dive`, `distances_from_door`, `defrag [ordered]`,
`goodbye_last_tells` and more (see `goal_from` in emu.c). Unknown goals refuse
to run rather than guess.

Randomized levels (or programs with multi-direction steps) run over many seeded
trials; `result` is `WIN` (all trials), `PROBABILISTIC` (some -- a luck-based
solution the game would accept on a lucky run), or `FAIL`.

## What is faithful, and what isn't

Faithful (validated by running the community repo's known-good solutions
against real level geometry — the large majority of Years 2–43 pass):

- Parser: labels, `comment`, `-- headers --` and trailing `DEFINE COMMENT`
  doodle blocks, comma-separated direction lists, multi-line `if` conditions
  chained with `and`/`or` (evaluated left to right), comparisons
  `== != < > <= >=` over tile types (`wall`/`datacube`/`hole`/`nothing`/
  `something`/`shredder`/`printer`/`worker`), numbers, `myitem`, other tiles'
  values and memory slots, `else`/`else:`, `jump`, `end`.
- Commands: `step`, `pickup`, `drop`, `giveto` (multi-direction), `takefrom`,
  `write <n|memN>`, `tell <target> <word>` / `listenfor <word>`,
  `memN = nearest <type>`, `memN = set <x>`, `memN = calc <a> <op> <b>`
  (integer arithmetic, division by zero yields nothing), and
  `memN = foreachdir <dirs>:` ... `endfor` loops.
- Memory-slot targets: `pickup memN` walks ONTO the remembered cube's tile
  (you lift what is under your feet -- the Data Backup swap depends on it);
  `giveto/takefrom memN` walk to an adjacent tile (BFS pathfinding, jams break
  up by shuffling). A stale `nearest` reference re-resolves to the next
  nearest of its kind on arrival -- and `nearest datacube` also finds a cube
  in a carrier's hands, so crowds chase the thing rather than besiege a
  square; `nearest` includes the tile you stand on. Actions that would no-op
  (take while holding, give while empty) skip without walking. A worker whose
  program has ended sits down and stops blocking the aisle.
- Mechanics: multi-direction steps pick randomly among passable choices,
  `foreachdir` sweeps visit their tiles in a random order per sweep, the
  walk-swap rule, movement chains and rotation cycles resolve simultaneously,
  waiting on unmet conditions, holes destroying what falls in, tell/listen
  synchronization (a beat's give-aways resolve before its take-froms).
  Standing on a cube, `drop` places the held cube on a free neighboring tile.
- Sensing: a tile is a *set* of contents -- a worker standing on a floor cube
  matches both `worker` and `datacube`; a cube held up in the air is not a
  floor cube. Numeric reads: pointing a DIRECTION at a neighbor also reads
  the item in their hands ("compare their items"); a remembered TILE reads
  only what lies on the floor there. `calc` treats a missing operand as 0
  (only division by zero yields nothing). `myitem ==
  datacube/something/nothing` tests holding. `mem == mem` on two remembered
  tiles compares identity. Blank cubes (the game's numberless ones) read 0;
  only the distinct-random kind carries drawn values.
- Special rules: no-walking, unique shredder use, exploding label cubes, one
  shredder at a time, speak-in-turn.
- SIZE: every command except labels, comments, `else`, `endif`, and `endfor`.

Not yet faithful / deliberately refused at runtime rather than guessed:

- The three counting-machine levels (their sensors/display are not part of the
  extracted grid) refuse to run.
- Dense multi-worker choreography can resolve differently than the game's
  crowd behavior (known cases: Injection Sites 2 (size), Checkerboard
  Organization, Neighborly Sweeper, Data Flowers); heavily scripted
  position-dependent speed solutions are similarly sensitive.

## The Speed model

Workers run asynchronously on their own clocks: commands have per-command
durations and the reported `speed` is the win moment in whole seconds --
matching the game's TIME metric. The durations (step 333 ms; pick/drop/give/
take 250 ms; printers 1200 ms; shredders 750 ms; a condition check 333 ms;
tell 1 s; everything else free) were calibrated against the recorded
community speeds in this repo's README and reproduce a growing set of them
exactly (all early years, Content Creators, Reverse Line, Automated
Pleasantries...). Blocked workers leave a standing step intent, so two
workers walking into each other still trade places even when their programs
have drifted out of phase. Treat close calls (within a second or two) as
needing in-game verification; congested crowd levels still drift more.

`--trace` prints the board and per-worker actions each event for debugging.
