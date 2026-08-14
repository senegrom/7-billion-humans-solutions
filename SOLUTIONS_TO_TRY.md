# Solutions to Try in the Real Game

This is the live-game verification queue.  A candidate stays here until the
game itself reaches its completion screen (or an explicit failure), because an
emulator win is evidence rather than proof.  The game stops a run after 1,400
seconds, so an unfinished run at that point is a failure, not an eventual win.

For every attempt, record the editor-reported size, completion/failure, the
displayed speed, and a screenshot.  For stochastic programs, also record the
attempt number and do not restart merely because a run looks slow.

## Priority queue

### [ ] Year 15 - Shred Lines - event-gated size 5

- Goal: improve the current size record from 8 commands to 5.
- Current capped-emulator evidence: 300/300 wins, average speed 190.9,
  range 85-378, maximum 23,590 of 87,500 frames.  With zero failures in 300
  trials, the one-sided 95% lower confidence bound is just over 99%.
- The owned-game parser accepted the exact text and built its runtime graph.
  This confirms paste/parser legality, but not that the live level completes.
- Expected editor size: **5**.
- Entry method: paste the text; the random multi-direction `step n,s` is not
  constructible from Year 15's editor palette.
- Why it is faster than the size-4 random walk: it only attempts a pickup when
  a cube is directly north of an empty worker, or while a shredder is directly
  south.  That avoids most of the game's 1.5-second error bubbles.
- Suggested live test: five fresh runs initially; capture every completion
  screen and any timeout or unexpected error state.
- Result: _not yet tested in the game_.

```text
a:
step n,s
if n == datacube and
 myitem == nothing or
 s == shredder:
    pickup n
    giveto s
endif
jump a
```

### [ ] Year 60 - Understaffed Sorting - ordered-pickup size 8

- Goal: improve the current size record from 9 commands to 8.
- Capped-emulator evidence: 300/300 wins over seeds 1-300, average speed 604.5,
  range 264-1,224, and maximum 76,461 of 87,500 frames.  With zero failures
  in 300 trials, the one-sided 95% lower confidence bound is just over 99%.
- The reduction replaces the incumbent's consecutive `pickup w` / `pickup se`
  with ordered `pickup w,se`.  Direction lists are tried left-to-right and
  stop at the first successful pickup, preserving the intended choice while
  removing one charged instruction; the shorter schedule still needs the live
  game as its timing oracle.
- Expected editor size: **8**.
- Entry method: paste the text; multi-direction `pickup w,se` is not
  constructible from Year 60's normal editor palette.
- Suggested live test: five fresh runs initially.  Record every displayed
  speed and allow each attempt to reach the game's own cutoff.
- Result: _not yet tested locally in the game_.

```text
a:
if sw == datacube or
 se == datacube and
 w == datacube or
 s > se and
 w != worker:
    pickup s
    drop
    pickup w,se
endif
step s
drop
step nw,n,ne
jump a
```

### [ ] Year 15 - Shred Lines - deterministic size 7

- Goal: improve the current size record from 8 commands to 7.
- Emulator evidence: deterministic win in 2,167 frames; displayed emulator
  speed 35 after correcting invalid `giveto` timing.
- Expected editor size: **7**.  The emulator currently prints 6 because its
  size counter incorrectly treats `else` as free.
- Suggested live test: one completion is enough to establish correctness;
  run three times only if the displayed speed unexpectedly varies.
- Result: _not yet tested in the game_.

```text
a:
if myitem == nothing:
    step n
    pickup n
else:
    step s
    giveto s
endif
jump a
```

### [ ] Year 44 - Unique Fashion Party - size 5 speed tie-break

- Goal: retain size 5 while improving the current typical speed from about 62
  to the community source's reported 10.
- Source: dmr's public glitchless program.  The source repository classifies
  it as over 99%, but the file has no per-program sample count.
- Emulator caveat: the current model loses this program because Year 44's
  initial/random layout is not faithful enough; the real game is authoritative.
- Suggested live test: 10 fresh runs.  Record any survivors if a run fails.
- Result: _not yet tested locally in the game_.

```text
a:
if se != worker or
 ne != worker and
 w != wall and
 myitem != datacube:
    mem1 = calc 0 / 0
endif
if ne != worker and
 n != datacube and
 w != datacube or
 w == wall:
    pickup s
endif
jump a
```

### [ ] Year 47 - Automated Pleasantries - speed tie-break at size 33

- Goal: retain the current displayed speed record of 6 while reducing the
  secondary size from 34 commands to 33.
- Deterministic emulator A/B evidence: the candidate and incumbent complete on
  the same frame, 281 (modelled speed 5; the known live-game score is 6).
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 47 - Automated Pleasantries (speed).txt>)
  and delete the final standalone two-line `if myitem == myitem:` / `endif`
  immediately before `tell w morning`.
- Why it should be safe: that delay belongs only to the eastmost worker's
  pre-greeting block, and the normal west-to-east chain completes before the
  deleted instruction is reached.
- Expected editor size: **33**; expected displayed speed: **6**.
- Suggested live test: one run should suffice because the level and schedule
  are deterministic; capture the completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 39 - Printing Etiquette 1 - speed tie-break at size 172

- Goal: retain the current displayed speed record of 35 while reducing the
  secondary size from 173 commands to 172.
- Deterministic emulator A/B evidence: the candidate and incumbent both
  complete in exactly 2,581 frames with identical item counts (modelled speed
  42; the known live-game score is 35).
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 39 - Printing Etiquette 1 (speed).txt>)
  and delete branch `a`'s terminal `end`, immediately after its fifth and final
  `drop`.
- Why it should be safe: that fifth drop satisfies `printed_per_worker 5`, and
  the level's goal is checked at the end of the same frame, before the deleted
  instruction can dispatch.
- Expected editor size: **172**; expected displayed speed: **35**.
- Suggested live test: one deterministic A/B run should suffice; capture the
  completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 40 - Printing Etiquette 2 - historical speed 36 at size 176

- Goal: improve the current size-177 / speed-37 record to size 176 / speed 36.
- Source provenance: [upstream PR #92](https://github.com/hingston/7-billion-humans-solutions/pull/92)
  by commonnickname reports 36 seconds.  The author closed it because another
  contemporary solution appeared better, not because this edit failed; the
  one-line reduction is absent from the current record.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 40 - Printing Etiquette 2 (speed).txt>)
  and delete the `step w` between the final `takefrom mem1` and `step n` in the
  branch that writes and drops its fifth sheet.
- Deterministic emulator A/B evidence: the candidate wins in 2,754 frames and
  the untouched incumbent in 2,777 frames, with the same 102 item actions.
  Both round to modelled speed 45; canonical sizes are 176 and 177.
- Expected editor size: **176**; expected displayed speed: **36** based on the
  original real-game submission.
- Suggested live test: run the incumbent once as a control, delete that one
  step, and capture the candidate completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 36 - Seek and Destroy 2 - speed tie-break at size 211

- Goal: retain the current displayed speed record of 46-47 while reducing the
  secondary size from 215 commands to 211.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 36 - Seek and Destroy 2 (speed).txt>)
  and make four reductions: remove `mem1 = set myitem` immediately after the
  early `pickup n` / `mem2 = set mem1` sequence (baseline line 90); remove the
  one-time `mem4 = set nothing` initialization (line 110); and replace each of
  the two `mem1 = set c` followed by `if mem1 ==/!= datacube` pairs (lines
  16-17 and 119-120) with a direct `if c ==/!= datacube`.
- Why it should be safe: after `mem2` captures the selected cube identity,
  `mem1` is not read before a later unconditional overwrite.  `mem4` is already
  initialized to `nothing`, the setup branch is entered only once, and its
  next read therefore has the same result.  Each folded pair samples the same
  center cube at the same effect frame, and `mem1` is overwritten before its
  discarded copy could be read.
- Same-world emulator A/B evidence: at winning seed 23, incumbent and candidate
  both completed in exactly 2,987 frames with 212 item actions and modelled
  speed 48.  Canonical counting independently confirms sizes 215 and 211.
- Expected editor size: **211**; expected displayed speed: **46-47**.
- Suggested live test: run the incumbent and candidate on fresh worlds until
  each completes, then confirm the candidate's editor size and displayed speed.
- Result: _not yet tested locally in the game_.

### [ ] Year 37 - Dangerous Spreadsheeting - ordered-set speed tie-break at size 244

- Goal: retain the current displayed speed record of 10-11 while reducing the
  secondary size from 246 commands to 244.
- Exact edit: in the late state that currently does `mem1 = set c`, tests
  `mem1 != datacube`, and conditionally does `mem1 = set e`, replace the whole
  four-line block with the single ordered assignment `mem1 = set c,e`.
- Why it should be safe: the old block retains the center item when numeric and
  otherwise falls back to the east item.  Ordered multi-target `set c,e` makes
  the same first-success choice, and this state only reads the resulting value
  numerically; it never uses the saved location as an action target.
- Deterministic emulator A/B evidence: candidate and incumbent both win in
  exactly 747 frames with modelled speed 12.  The candidate uses 68 modelled
  item actions versus 70 and canonical counting confirms 244 versus 246.
- Timing caveat: the fusion contracts the fallback path, so live choreography
  and counter timing still need an A/B run despite the exact emulator result.
- Expected editor size: **244**; expected displayed speed: **10-11**.
- Suggested live test: run the current speed program as a control, then the
  candidate; capture both completion panels and candidate editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 42 - Important Email Organization - speed tie-break at size 143

- Goal: retain the current displayed speed record of 72-74 while reducing the
  secondary size from 145 commands to 143.
- Exact edit: delete both `mem1 = set mem2` copies at the entries to labels `g`
  and `h`; retarget the shared `pickup mem1`, later `if mem1 > 9`, and
  `step mem1` uses to `mem2`.
- Why it should be safe: `mem2` is the nearest-cube reference selected by the
  preceding branch on every entry.  On pickup success, the loop overwrites
  `mem1` before any read; on failure, the recovery path now reads the original
  `mem2` directly and a later state overwrites `mem1`.  The edit therefore
  preserves every target/value while removing two copied memory values.
- Emulator caveat: the incumbent fails 0/20 in the current model (and the
  candidate also failed the initial seed), so the model cannot referee timing
  on this level.  Canonical counting independently confirms 143 versus 145.
- Expected editor size: **143**; expected displayed speed: **72-74**.
- Suggested live test: run the incumbent as a loading/control check, then the
  candidate; capture either completion panel or the exact failure state.
- Result: _not yet tested locally in the game_.

### [ ] Year 25 - My First Shredding Memory - speed tie-break at size 8

- Goal: retain or improve the current displayed speed record of 129 while
  reducing the secondary size from 9 commands to 8.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 25 - My First Shredding Memory (speed).txt>)
  and remove only the `if myitem == datacube:` / `endif` guard around the first
  `giveto mem1`, leaving that give unconditional after `pickup mem2`.
- Why it should be safe: a successful pickup proceeds to the same remembered
  shredder.  A pickup-race loser instead performs a harmless empty-hand give
  error and rejoins the same loop; no cube identity or machine target changes.
- Deterministic emulator A/B evidence: candidate and incumbent both win with
  157 item actions; the candidate is 32,768 frames versus 32,839, and modelled
  speed 525 versus 526.  The model's absolute timing differs sharply from the
  known live score, so only the paired direction is evidence.
- Expected editor size: **8**; expected displayed speed: at most **129**.
- Suggested live test: run the incumbent once as a control and the candidate
  once, capturing both completion panels and the candidate's editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 29 - Biometric Access - speed tie-break at size 182

- Goal: retain the current displayed speed record of 54 while reducing the
  secondary size from 185 commands to 182.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 29 - Biometric Access (speed).txt>)
  and make three independent deletions:
  - delete the earlier `mem1 = nearest datacube` immediately before `pickup c`
    (baseline line 85); that branch does not read `mem1` before overwriting it;
  - delete the second of the consecutive `giveto mem3` commands near the end
    of the `pickup ne` branch (currently line 149); and
  - delete `mem1 = nearest datacube` immediately after the later
    `pickup mem1` (currently line 171).
- Deterministic emulator A/B evidence: candidate and incumbent both complete
  in exactly 4,365 frames with 148 items and identical modelled speed 70.
  Their canonical editor sizes are 182 and 185; the emulator prints three
  fewer because its size counter treats `else` as free.
- Why it should be safe: the first give does not complete until the worker has
  fed its remembered personal shredder and emptied its hands.  The second give
  can therefore only start an empty-hand error bubble.  The later nearest
  assignment is never read before `mem1` is overwritten with `nearest worker`.
  The earlier nearest assignment is likewise overwritten without a read.
  Both contribute only otherwise-unneeded delays.
- Rule caveat: the emulator does not enforce `personal_shredders`, but the
  deleted command is empty-handed and cannot change ownership or machine
  selection; the live game remains authoritative.
- Expected editor size: **182**; expected displayed speed: **54**.
- Suggested live test: one deterministic A/B run should suffice; capture the
  completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 215

- Goal: retain the current displayed speed range of 9-12 while reducing the
  secondary size from 216 commands to 215.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 62 - The Sorting Floor (speed).txt>)
  and delete either one of the two consecutive `mem1 = set myitem` commands
  immediately before `tell everyone hi` and the divide-by-zero exit.
- Why it may be safe: the first saved value is overwritten by the identical
  second store before any read; only its synchronization delay can matter.
- Emulator A/B evidence: candidate and incumbent both win seed 1 in exactly
  836 frames.  Across the same 100 model worlds each wins 36, with modelled
  speed 12.3 and nearly identical average frames (734.9 versus 735.0).
- Fidelity caveat: the current model badly under-reproduces the published
  incumbent reliability, so those paired aggregates support equivalence but
  cannot establish live reliability or timing.
- Expected editor size: **215**; expected displayed speed: **9-12**.
- Suggested live test: run the incumbent and one-store candidate in the game;
  capture both completion panels and candidate editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 66 - Decimal Counter - duplicate-tell speed tie-break at size 253

- Goal: retain the current displayed speed record of 24 while reducing the
  secondary size from 254 commands to 253.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 66 - Decimal Counter (speed).txt>)
  and delete either one of the two consecutive `tell everyone hi` commands
  between `step ne` and the following `step s`.
- Why it should be safe: no command in the program listens for `hi`, and one
  identical tell remains as the original cadence pad.
- Deterministic emulator A/B evidence: candidate and incumbent both win in
  exactly 1,552 frames with modelled speed 25.  The candidate performs 128
  modelled item actions versus 129; canonical sizes are 253 and 254.
- Boundary check: deleting both tells still wins but slows the model to 1,565
  frames, so only the single-command deletion is proposed.
- Expected editor size: **253**; expected displayed speed: **24**.
- Suggested live test: one deterministic incumbent/candidate A/B should
  suffice; capture the completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 67 - Decimal Doubler - unlistened-tell speed tie-break at size 209

- Goal: retain the current displayed speed record of 41 while reducing the
  secondary size from 210 commands to 209.
- Exact edit: delete the second of the two consecutive `tell everyone hi`
  commands in loop `j`, after `step mem3` and `step e`.
- Why it may be safe: the program contains no `listenfor`, so the second tell
  cannot transmit a value, alter control flow, or satisfy the level goal.  It
  is only a 42-frame cadence pad.
- Timing caveat: removing that pad can still change counter/crowd arbitration.
  Candidate and incumbent both fail the current extracted-level model on
  seed 1, so the emulator cannot referee this edit; it remains live-only.
- Expected editor size: **209**; expected displayed speed: **41**.
- Suggested live test: run incumbent and candidate in the game and capture both
  completion panels, treating any difference as timing evidence.
- Result: _not yet tested locally in the game_.

### [ ] Year 68 - Goodbye, Humans! - static speed tie-break at size 171

- Goal: retain the current displayed speed record of 16 while reducing the
  secondary size from 172 commands to 171.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 68 - Goodbye Humans (speed).txt>)
  and delete the second of the two consecutive top-level
  `tell everyone hi` commands (currently line 196).
- Static evidence: no worker listens for `hi`, so the deleted command is only a
  42-frame timing pad and cannot directly change a value or greeting.  It can
  still change crowd arbitration, which is why this remains a live-game lead.
- Emulator caveat: the current model fails both the published incumbent and
  this candidate on seed 1 and miscounts their editor sizes as 182 and 181, so
  it cannot referee the deletion.
- Expected editor size: **171**; expected displayed speed if successful:
  **16**.
- Suggested live test: run the incumbent once as a loading/control check, then
  the candidate once; capture both completion panels or the candidate's final
  failure state.
- Result: _not yet tested locally in the game_.

### [ ] Year 44 - Unique Fashion Party - speed 1 at size 8

- Goal: tie the current speed record of 1 while reducing its secondary size
  from 17 commands to 8.
- Source: abfipes12's public main-speed program, covered by that repository's
  over-99% classification but without a per-file sample count.
- Emulator caveat: do not use the current Year 44 model to accept or reject it;
  its randomized starting state is known to be unfaithful.
- Expected editor size: **8**; expected displayed speed: **1**.
- Suggested live test: 10 fresh runs, recording any survivor pattern.
- Result: _not yet tested locally in the game_.

```text
if w == wall:
    pickup s
    end
endif
if nw >= 0 and
 n != datacube:
    pickup s
endif
if nw >= 0 and
 n != datacube:
    pickup s
endif
mem1 = calc 0 / 0
```

### [ ] Year 44 - Unique Fashion Party - transient-survivor size 3

- Goal: establish a size-3 SolutionsLowPercent record below the public size-4
  low-percent entry and size-5 main entry.
- Mechanism: all 45 workers take their cubes and walk toward the room's holes.
  The goal is checked every frame, so the run wins during any transient frame
  with exactly seven survivors whose held values are a permutation of 0-6;
  the workers do not need to remain stable afterward.
- Constructive evidence: 38 workers can follow finite routes into upper/bottom
  holes within 13 strides while seven designated workers take longer cardinal
  routes, leaving a nonempty exactly-seven window.  Conditional on a fixed
  last-seven set, value uniqueness has probability `7! / 7^7`, about 0.612%.
- Fidelity caveat: the current Year 44 emulator loses even the public size-4
  program and is not a trustworthy large-crowd oracle.  This is a live-only
  candidate with a positive finite schedule, not a measured success rate.
- Expected editor size: **3**; paste-only because of `step s,e,se`.
- Suggested live test: repeated fresh runs, capturing every exactly-seven
  survivor pattern and the first completion panel.
- Result: _not yet tested locally in the game_.

```text
pickup s
a:
step s,e,se
jump a
```

### [ ] Year 44 - Unique Fashion Party - low-percent size 4

- Goal: live-confirm the new size-4 SolutionsLowPercent entry below the
  size-5 main record.
- Public evidence: abfipes12 reports positive real-game wins, but its header is
  internally inconsistent: "40 failures out of 50" implies 20%, while the
  same line labels the result 10%.
- Current emulator evidence: 0/20.  Year 44's model is already known to have an
  unfaithful randomized layout, so that result cannot overrule the live source.
- Expected editor size: **4**.
- Suggested live test: 10-20 runs; capture the final cube/worker arrangement on
  every failure.
- Result: _not yet tested locally in the game_.

```text
pickup s
if w != wall:
    a:
    step s,e,se
    jump a
endif
```

### [ ] Year 58 - Good Neighbors - size 4

- Goal: live-validate the existing
  [Solutions50+ entry](<Solutions50+/Year 58 - Good Neighbors (size).txt>).
- Current capped-emulator evidence: 195/200 wins at the real 87,500-frame
  deadline, average winning speed 483.3, range 85-1,379.  The five failures
  confirm that this belongs in Solutions50+, not Solutions99+.
- Suggested live test: at least 10 runs.  A definitive frozen failure is all 20
  workers holding cubes while the level has not completed; capture the board.
- Result: _not yet tested in the game_.

### [ ] Year 15 - Shred Lines - stochastic size 4

- Goal: a much smaller Solutions50+ or, with enough evidence, Solutions99+
  record.
- Corrected-emulator evidence: 99/100 wins at the 87,500-frame cutoff; winning
  speed averaged 655 with range 320-1,183.  One run hit the real-game deadline.
- Expected editor size: **4**.
- Entry method: paste the text because it uses `step n,s`.
- Suggested live test: 10 runs initially.  This is lower priority than the
  deterministic size-7 program because it is already known to time out rarely.
- Result: _not yet tested in the game_.

```text
a:
step n,s
pickup n
giveto s
jump a
```

### [ ] Year 30 - Fill the Floor - probabilistic size 4

- Goal: add a size-4 Solutions50+ record below the size-5 main entry.
- Public evidence: abfipes12 and martinez8859 report 15/25 wins (60%).
- Current capped-emulator evidence: 64/100 wins, average winning speed
  1,210.8, range 813-1,397.  The close agreement supports the 50+ tier, but
  successful runs often finish only just before the game deadline.
- Expected editor size: **4**; paste-only because of the multi-direction
  `takefrom`.
- Suggested live test: 10 uninterrupted runs, allowing every run to reach the
  game's own deadline.
- Result: _not yet tested locally in the game_.

```text
a:
step nw,w,sw,s,ne,e,se
drop
takefrom nw,sw,ne
jump a
```

### [ ] Year 30 - Fill the Floor - alternate size 5

- Goal: tie the size-5 record with a faster typical run.
- Emulator A/B evidence over the same 100 seeds: candidate 100/100, average
  602.7 seconds, range 372-1,277; incumbent 100/100, average 635.1 seconds.
  The incumbent's actual game score is about 588, so the emulator improvement
  may not carry over.
- Suggested live test: five alternating fresh runs of the incumbent and this
  candidate; compare medians and timeout count.
- Result: _not yet tested in the game_.

```text
mem1 = nearest printer
a:
takefrom mem1
step nw,sw,n,e,se
drop
jump a
```

### [ ] Year 12 - Unzip - one-shot low-percent size 3

- Goal: establish a size-3 SolutionsLowPercent record below the size-5 main
  entry.
- Mechanism: all 12 workers pick up their on-tile cubes, independently choose
  north or south once, and drop.  Exactly one of the `2^12 = 4,096` direction
  patterns is the required alternating zipper.
- Capped-emulator evidence: 23/100,000 wins, close to the theoretical 1/4,096
  rate; every win completed in exactly 56 frames with displayed speed 1.
- Expected editor size: **3**; paste-only because of `step n,s`.
- Suggested live test: use repeated fresh runs rather than waiting within one
  run—the program finishes immediately.  A live win may take several thousand
  attempts, so this is lower priority than the main-tier candidates.
- Result: _not yet tested locally in the game_.

```text
pickup c
step n,s
drop
```

### [ ] Year 05 - An Important Decision - absorbing low-percent size 2

- Goal: establish a size-2 SolutionsLowPercent record below the existing
  size-4 low-percent entry and the size-5 main entry.
- Mechanism: each of the four workers performs an independent one-dimensional
  random walk until it falls into one of the two row holes.  The level wins
  only when every worker reaches its designated side; under unbiased choices
  the exact success probability is `24 / 2,401`, or about 1.00%.
- Capped-emulator evidence: 12/1,000 wins (1.2%), average winning speed 8.0,
  range 5-14, and winning frames 253-862.  The observed rate agrees with the
  static probability and no winning run approached the deadline.
- Expected editor size: **2**.
- Entry method: paste the text; random multi-direction `step w,e` is not
  constructible from Year 05's normal editor palette.
- Suggested live test: repeated fresh runs; roughly 300 attempts give about a
  95% chance of seeing at least one win if the real game's direction choices
  are unbiased.  Capture the first completion panel.
- Result: _not yet tested locally in the game_.

```text
a:
step w,e
jump a
```

### [ ] Year 06 - Little Exterminator 1 - monotone low-percent size 3

- Goal: establish a size-3 SolutionsLowPercent record below the existing
  size-7 low-percent entry and the size-8 main entry.
- Mechanism: the restricted random walk moves only southward/eastward through
  the maze.  A small set of direction sequences reaches pickup range of the
  target cube; other paths absorb into holes rather than wandering until the
  game deadline.
- Capped-emulator evidence: 2/10,000 wins (0.02%); the two wins completed in
  755 and 862 frames with displayed speeds 13 and 14.
- Expected editor size: **3**.
- Entry method: paste the text; the four-direction random step and eight-target
  pickup are not constructible from Year 06's normal editor controls.
- Suggested live test: repeated quick resets only if pursuing a very rare
  record.  The observed rate implies thousands of attempts per win, so this is
  lower priority than the deterministic and main-tier candidates.
- Result: _not yet tested locally in the game_.

```text
a:
step s,sw,se,e
pickup c,s,se,sw,e,w,n,ne
jump a
```

### [ ] Year 13 - Injection Sites 2 - recoverable low-percent size 5

- Goal: improve the existing size-6 SolutionsLowPercent entry to size 5.
- Provenance: this is H-J-Granger's public low-percent program with only its
  initial `step se` removed; retain that attribution if it is promoted.
- Mechanism: all six workers first take their cubes, then use the same
  recoverable six-direction walk and exact gap predicate as the public
  program.  Removing the initializer creates additional hole-loss paths but
  leaves collision-free successful routes reachable.
- Capped-emulator evidence: 14/100 wins; winning speed averaged 365.9, ranged
  from 59 to 1,223, and used 3,635-76,436 frames.
- Expected editor size: **5**.
- Entry method: paste the text because the six-direction random step is not
  constructible from the normal editor controls.
- Suggested live test: repeated fresh runs; capture a completion panel and
  confirm editor size 5.  The emulator sample suggests this should be much
  more practical than the rarer one-shot entries below.
- Result: _not yet tested locally in the game_.

```text
pickup s
a:
step w,sw,n,s,e,se
if c == nothing and
 w == datacube:
    drop
endif
jump a
```

### [ ] Year 22 - Number Royale - survivor low-percent size 3

- Goal: improve the existing size-4 SolutionsLowPercent entry to size 3.
- Mechanism: every worker takes its own cube, then performs an independent
  north/south random walk until falling through the disposal hole.  The level
  wins when all non-maximum holders have died while at least one maximum holder
  remains alive; otherwise the run absorbs as a loss.
- Capped-emulator evidence: 84/1,000 wins; winning speed averaged 14.4, ranged
  from 5 to 34, and used 307-2,071 frames (876 average).  Every win completed
  far before the deadline.
- Expected editor size: **3**.
- Entry method: paste the text because `step n,s` is a random multi-direction
  command unavailable from the normal editor controls.
- Suggested live test: repeated quick runs; verify that the completion panel
  appears while at least one maximum-valued worker is still alive and capture
  editor size 3.
- Result: _not yet tested locally in the game_.

```text
pickup s
a:
step n,s
jump a
```

### [ ] Year 52 - The Mode Code - one-shot low-percent size 6

- Goal: establish a size-6 SolutionsLowPercent record below the size-15 main
  entry.
- Mechanism: each worker binds its own result cube, samples the input cube two
  rows north, and writes that sample plus 8.  The level wins exactly when the
  six sampled values happen to equal the six true frequency counts minus 8.
- Exact probability: conditioning on the six sampled cubes and the remaining
  58 independent uniform draws gives `2.90709234823e-6`, or about one win in
  343,986 worlds.
- RNG/emulator witness: an independent exact RNG search predicted the first
  winning world at seed 69,510 with counts `[13,9,11,11,12,8]` and samples
  `[5,1,3,3,4,0]`.  The capped emulator then found exactly 1/69,510 wins, at
  that final seed, completing in 342 frames with displayed speed 6.
- Expected editor size: **6**; every command is available in Year 52.
- Suggested live test: this is mathematically sound but far too rare for a
  practical manual campaign.  Preserve it for a lucky natural run or a future
  reproducible live-game RNG harness; capture the completion panel if tested.
- Result: _not yet tested locally in the game_.

```text
mem2 = set s
step n
mem1 = calc n + 8
pickup mem2
write mem1
drop
```

### [ ] Year 54 - Terrain Leveler - constant-average low-percent size 5

- Goal: establish a size-5 SolutionsLowPercent record below the size-9 main
  entry.
- Mechanism: all seven workers sweep straight north through their columns,
  rewriting every cube to 3.  The run wins exactly in worlds whose original
  49-cube average rounds down to 3.
- Probability analysis: accounting for the level's random 0-6, 0-10, and 0-20
  range modes gives an intended-world probability about 0.259.  The 0-6 mode
  alone has probability about 0.514 of averaging to 3.
- Capped-emulator evidence: 20/100 wins; winning speed averaged 30.6, ranged
  from 29 to 31, and used 1,786-1,936 frames.
- Expected editor size: **5**; all commands are available in Year 54.
- Suggested live test: repeated quick runs; record the random range and capture
  the first completion panel with editor size 5.
- Result: _not yet tested locally in the game_.

```text
a:
step n
pickup c
write 3
drop
jump a
```

### [ ] Year 55 - Data Flowers - constant-sum low-percent size 5

- Goal: establish a size-5 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: the five workers march north through their flower centers, move
  one south petal into each center, and write the constant 36.  The level wins
  exactly when every independent eight-value flower ring originally sums to
  36; moving a petal does not change the stored target sum.
- Exact probability: one eight-value ring sums to 36 with probability
  `4,816,030 / 10^8`; all five do so with probability
  `2.5908717630610564e-7`, or about one in 3,859,705.
- RNG/emulator witness: an independent xorshift search found seed 3,868,438.
  All five eight-value groups sum to 36, and the isolated seed-offset emulator
  won in 2,099 frames with displayed speed 34 and 172 item actions.
- Expected editor size: **5**.
- Entry method: paste the text because the ordered multi-target `pickup c,s`
  is not constructible from Year 55's normal editor controls.
- Suggested live test: natural manual verification is impractical without a
  reproducible RNG-start method.  Capture the completion panel if the matching
  world can be reproduced.
- Result: _not yet tested locally in the game_.

```text
a:
step n
pickup c,s
write 36
drop
jump a
```

### [ ] Year 56 - Local Maximums - one-shot low-percent size 3

- Goal: improve the existing size-4 SolutionsLowPercent entry and size-7 main
  entry to size 3.
- Mechanism: each worker takes the northwest cube from its own eight-cube
  group and feeds its nearest shredder.  The level wins exactly when all seven
  selected cubes are already weak maxima of their groups, so the omitted
  `write 99` is unnecessary in that world.
- Exact probability: each selected value is maximal with probability
  `sum(k^7, k=1..100) / 100^8`; across seven independent groups the win rate
  is `6.2945867338e-7`, or about one in 1,588,667.
- RNG/emulator witness: an independent xorshift search found seed 3,281,406.
  Its selected values are `[70,87,83,79,96,98,90]`, each the maximum of its
  group.  An isolated seed-offset emulator run then won in 310 frames with
  displayed speed 5 and 14 item actions.
- Expected editor size: **3**; all commands are available in Year 56.
- Suggested live test: preserve this as a mathematically witnessed rare record;
  natural manual verification is impractical without a reproducible RNG-start
  method.  Capture the completion panel if the matching world occurs.
- Result: _not yet tested locally in the game_.

```text
pickup nw
mem1 = nearest shredder
giveto mem1
```

### [ ] Year 62 - The Sorting Floor - initially sorted size 0

- Goal: establish a zero-command SolutionsLowPercent record below the size-10
  Solutions50+ and size-12 main entries.
- Mechanism: do nothing.  The nine independent random cubes occasionally
  spawn in weakly increasing row-major order, satisfying the goal before the
  first frame is processed.
- Exact probability: `C(108, 9) / 100^9 = 3.9113958819e-6`, or about one win
  in 255,663 worlds.
- RNG/emulator witness: an independent xorshift search found seed 239,189.
  Its constructor values become row-major
  `[10,14,18,41,62,69,80,88,95]`; the isolated seed-offset emulator accepted
  the label-only program at frame 0 with size 0 and displayed speed 0.
- Expected editor size: **0**; the free label is present only to make the text
  pasteable and does not count as a command.
- Suggested live test: natural manual verification is impractical.  If a
  reproducible live-game RNG-start method becomes available, paste the free
  label, run the matching world, and capture the immediate completion panel.
- Result: _not yet tested locally in the game_.

```text
a:
```

### [ ] Year 26 - Budget Brigade 2 - all-left low-percent size 3

- Goal: establish a size-3 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: the vertical printer chain relays every sheet north, the top
  worker turns it west, and the horizontal chain relays it to the left-hand
  low-value shredder.  The strict split goal wins exactly when the first 20
  sheets all have values below 50.
- Exact probability: the first printer sheet is always 0, leaving 19 uniform
  binary threshold outcomes, so success is `2^-19 = 1 / 524,288`.
- RNG/emulator witness: seed 945,093 wins the strict extracted goal in 10,574
  frames with modelled speed 170 and 1,932 item actions.  Canonical counting
  confirms size 3.
- Minimality: a loop is necessary to request at least 20 sheets from the single
  printer, and separate take/give actions are necessary to relay and shred
  them.  Thus no size-2 program can satisfy this no-walking level.
- Expected editor size: **3**; the multi-target take/give lists are legal in
  Year 26 and do not require a paste-only marker.
- Suggested live test: natural manual verification is impractical without a
  reproducible RNG-start method; capture the completion panel if the witness
  world can be reproduced.
- Result: _not yet tested locally in the game_.

```text
a:
takefrom s,e
giveto n,w,s
jump a
```

### [ ] Year 38 - Seek and Destroy 3 - one-shot low-percent size 4

- Goal: establish a size-4 SolutionsLowPercent record below the size-10 main
  and size-8 Solutions50+ entries.
- Mechanism: each worker selects and shreds one nearest cube.  The level wins
  only when the first shredded cube happens to be the room's global minimum;
  routing and selection do not inspect values.
- Capped-emulator evidence: 23/1,000 wins (2.3%); every win completed in 133
  frames with displayed speed 3.
- Expected editor size: **4**; all commands are available in Year 38.
- Suggested live test: 50-100 fresh runs; each attempt ends within a few
  seconds, so reset immediately after an explicit failure.
- Result: _not yet tested locally in the game_.

```text
mem1 = nearest datacube
pickup mem1
mem1 = nearest shredder
giveto mem1
```

### [ ] Year 33 - Data Backup Day - one-shot low-percent size 5

- Goal: establish a size-5 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: every worker remembers its east value, selects one of its two
  equidistant cubes, and overwrites the selected cube.  The correct nearest
  tie choice in all eight pairs has probability close to `1/2^8`.
- Capped-emulator evidence: 44/10,000 wins (0.44%); every win completed in 146
  frames with displayed speed 3.
- Expected editor size: **5**; all commands are available in Year 33.
- Suggested live test: repeated quick runs; roughly a few hundred attempts per
  observed win is plausible, but record the actual tie behavior.
- Result: _not yet tested locally in the game_.

```text
mem1 = set e
mem2 = nearest datacube
pickup mem2
write mem1
drop
```

### [ ] Year 34 - Seek and Destroy 1 - one-shot low-percent size 4

- Goal: establish a size-4 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: each of four workers independently selects and shreds one nearest
  cube; the level wins only when those choices are the minimum cube in every
  column.  Cross-column nearest ties make the exact probability layout-dependent.
- Capped-emulator evidence: 5/10,000 wins (0.05%); every win completed in 187
  frames with displayed speed 3.  Seed 3,769 was independently isolated as a
  reproducible winning world with 8 item actions.
- Expected editor size: **4**; all commands are available in Year 34.
- Suggested live test: this may require thousands of quick resets.  Confirm
  that each successful run reports all four per-column minima before promotion.
- Result: _not yet tested locally in the game_.

```text
mem1 = nearest datacube
pickup mem1
mem1 = nearest shredder
giveto mem1
```

## Recently imported community programs

These already have public real-game evidence and are committed to `master`.
Local smoke-testing is optional, but unchecked items have not yet been
personally reproduced in this game installation.

### Commit `c7112a1`

- [ ] Y10 speed
- [ ] Y11 speed
- [ ] Y12 size
- [ ] Y20 speed
- [ ] Y21 size, speed, and Solutions50+ speed
- [ ] Y22 speed
- [ ] Y31 size
- [ ] Y32 size
- [ ] Y39 size
- [ ] Y40 size
- [ ] Y41 speed
- [ ] Y42 speed
- [ ] Y50 size
- [ ] Y51 speed
- [ ] Y52 speed
- [ ] Y60 size and speed
- [ ] Y62 Solutions99+ size and Solutions50+ size
- [ ] Y65 speed
- [ ] Y67 speed

### Commit `412d9d1`

- [ ] Y09 speed
- [ ] Y16 size
- [ ] Y18 size
- [ ] Y22 size
- [ ] Y23 speed
- [ ] Y34 size and speed
- [ ] Y37 size
- [ ] Y38 size and speed
- [ ] Y57 size
- [ ] Y58 size
- [ ] Y68 size

### Current all-level audit

- [ ] Y05 low-percent size
- [ ] Y06 low-percent size
- [ ] Y13 low-percent size
- [ ] Y30 Solutions50+ size
- [ ] Y34 Solutions50+ speed
- [ ] Y43 Solutions50+ size
- [ ] Y44 low-percent size
- [ ] Y53 low-percent size
- [ ] Y54 Solutions50+ speed

Current-audit evidence: Y30 is 15/25 in the public header and 64/100 in the
capped emulator; Y34 is 79/100; Y43 is 144/150 publicly and 18/20 locally
(winning emulator range 639-1,349 seconds); Y53 is 46/100 and was demoted from
Solutions50+; Y54 is 64/120.  The public Y44 size-4 header is arithmetically
inconsistent, and its incidental speed is still `TBD` until a live win is
captured.

## Rejected or superseded leads

Keep failed ideas here so they are not rediscovered and mistaken for records.

- Y24 size 3: emulator false positive; real handoff semantics do not sustain it.
- Y15 public size 6: 96/100 capped emulator wins, but superseded in both size
  and reliability evidence by the novel size-5 candidate above.
- Y15 size-5 give-first ordering: 300/300, but average and worst-case timing
  were slightly worse than the retained pickup-first ordering on the same
  seeds (191.4 vs 190.9 average; 387 vs 378 worst).
- Y15 symmetric size-5 gate `(shredder and holding) or north-cube`: 300/300,
  but much slower than the retained empty-worker gate (265.5 average, 727
  worst).  This exhausts the only other non-dominated one-IF predicate family.
- Y10 random-walk size 2: 0/100 capped emulator wins; any winning path is too
  rare to justify live-game queue space without stronger evidence.
- Y10 speed size-32 branch fusion: it still wins, but takes 867 model frames
  versus the incumbent's 790 and is therefore not a speed-record tie-break.
- Y14 pickup-first size 4: the existing live size-4 and this permutation both
  retain the cube-less worker's failed pickup/give path.  Disassembly shows no
  action-animation saving, and the corrected emulator makes them exactly tied
  at 97 frames even though it misreports the incumbent's live timing.  It is
  therefore expected to retain the live score of 4, not challenge speed 2.
- Y18 random-fan size 4: 0/100 capped emulator wins; the workers do not fan out
  to ten unique shredders reliably enough to retain as a live candidate.
- Y22 public size 4: its header says 86 failures out of 150, which is 42.7%
  success rather than the claimed 57.3%; current emulator is 48/100.  It ties
  the existing LowPercent size-4 program while being far slower, so it is
  dominated rather than a Solutions50+ candidate.
- Y26 size 6: invalid for the split-50 goal; the old loader ignored that rule.
- Y26 size-5 ordered take/give fusion: three distinct routing predicates each
  failed 0/100 at the 1,400-second cap; the mechanism stalls the feeder chain.
- Y20 speed size-36 ordered-pickup fusion: 0/1 while the incumbent won; the
  removed full-hands pickup error was a required synchronization delay.
- Y30 speed first branch-terminal `end` deletion: 0/1 at the real cutoff;
  without it that worker falls into the common-tail work before the exact fill
  goal is complete.
- Y31 unconditional size 5: 0/100; removing the parity/obstacle guard causes
  irreversible wrong-square drops rather than an absorbing checkerboard fill.
- Y33 ordered-pickup size 4: 0/1,000.  Directional `pickup w,e` does not move
  the worker onto the source tile, so terminal `drop` restores the copy on the
  center square; the extra restore step in the queued size-5 form is essential.
- Y34 alternate size 7 safe-init rewrite: 29/100 at the real cutoff.
- Y36 size 7: eventual emulator wins take up to roughly 6,400 seconds, beyond
  the game's 1,400-second limit.
- Y37 constant-23 size 11: an exact RNG world with all seven row sums equal to
  23 still fails because removing the repeated `calc` changes worker timing.
  The related size-5 random walk has a finite abstract route but no seed shown
  to realize both the required values and movement schedule.
- Y38 speed deletion of one, two, or all three consecutive `pickup c` retries:
  94/100, 93/100, and 91/100 respectively, versus 100/100 for the incumbent.
  The actions are reliability choreography rather than removable dead code.
- Y39 unguarded-printer size 5: 0/100 at a 20,000-frame diagnostic cap; random
  requeueing did not produce the required exactly-five-sheets-per-worker state.
- Y40 speed branch-`b` and branch-`c` terminal-`end` deletions: each failed
  0/1 at the real cutoff, while the incumbent wins in 2,777 frames.  Both
  workers fall through into later branches before the global goal is complete,
  so neither `end` is post-goal dead code.
- Y42 size-8 loop fusions: both the outer-delivery and unified random-walk
  variants failed 0/5; the separate search and delivery drifts are material.
- Y48 four-command speed rewrite: emulator training-goal false positive; the
  instructor does not shred its demonstration cube.
- Y48 size-3 `takefrom n,s` fusion: behaviorally collapses to the previously
  reverted direct-south size-3 program and removes the incumbent's
  role-dependent opening look/error timing.  The aggregate emulator accepts
  that shortcut, but the live game already rejected it; retain size 4.
- Y49 size-3 deletion of the final east clear step: 0/1 while the size-4
  incumbent won in 530 frames; the post-feed clear is required for the later
  shredder queue.
- Y54 deletion of either duplicated `mem2 = set c` before `tell everyone hi`:
  0/1 while the incumbent won in 2,673 frames.  The overwritten value is dead,
  but the store is a required tell/listen synchronization delay.
- Y58 size-3 random, nearest, cardinal-pick, diagonal-pick, and one-shot
  pruning families produced no witness across their bounded screens.  Exact
  graph optimization shows a win must remove at least 12 interior cubes and
  at most 8 perimeter cubes; the tested policies have the wrong removal bias.
- Y60 alternate size 9: superseded by dmr's public 0/200-failure size-9 record.
- Y63 size 9 rewrite: 0/10.
- Y64 size 7 rewrite: 0/5.
- Y65 size 13 rewrite: 0/52.
- Y68 unconditional size 5: 0/20; removing the side-hole guard did not produce
  an observable low-percent win.
