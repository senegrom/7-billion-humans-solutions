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

### [ ] Year 29 - Biometric Access - speed tie-break at size 184

- Goal: retain the current displayed speed record of 54 while reducing the
  secondary size from 185 commands to 184.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 29 - Biometric Access (speed).txt>)
  and delete the second of the consecutive `giveto mem3` commands near the end
  of the `pickup ne` branch (currently line 149).
- Deterministic emulator A/B evidence: candidate and incumbent both complete
  in exactly 4,365 frames with 148 items and identical modelled speed 70.
  Their canonical editor sizes are 184 and 185; the emulator prints three
  fewer because its size counter treats `else` as free.
- Why it should be safe: the first give does not complete until the worker has
  fed its remembered personal shredder and emptied its hands.  The second give
  can therefore only start an empty-hand error bubble and has no subsequent
  item action.
- Rule caveat: the emulator does not enforce `personal_shredders`, but the
  deleted command is empty-handed and cannot change ownership or machine
  selection; the live game remains authoritative.
- Expected editor size: **184**; expected displayed speed: **54**.
- Suggested live test: one deterministic A/B run should suffice; capture the
  completion panel and editor size.
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
  frames with displayed speed 3.
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
- Y18 random-fan size 4: 0/100 capped emulator wins; the workers do not fan out
  to ten unique shredders reliably enough to retain as a live candidate.
- Y22 public size 4: its header says 86 failures out of 150, which is 42.7%
  success rather than the claimed 57.3%; current emulator is 48/100.  It ties
  the existing LowPercent size-4 program while being far slower, so it is
  dominated rather than a Solutions50+ candidate.
- Y26 size 6: invalid for the split-50 goal; the old loader ignored that rule.
- Y30 speed first branch-terminal `end` deletion: 0/1 at the real cutoff;
  without it that worker falls into the common-tail work before the exact fill
  goal is complete.
- Y31 unconditional size 5: 0/100; removing the parity/obstacle guard causes
  irreversible wrong-square drops rather than an absorbing checkerboard fill.
- Y34 alternate size 7 safe-init rewrite: 29/100 at the real cutoff.
- Y36 size 7: eventual emulator wins take up to roughly 6,400 seconds, beyond
  the game's 1,400-second limit.
- Y40 speed branch-`b` and branch-`c` terminal-`end` deletions: each failed
  0/1 at the real cutoff, while the incumbent wins in 2,777 frames.  Both
  workers fall through into later branches before the global goal is complete,
  so neither `end` is post-goal dead code.
- Y42 size-8 loop fusions: both the outer-delivery and unified random-walk
  variants failed 0/5; the separate search and delivery drifts are material.
- Y48 four-command speed rewrite: emulator training-goal false positive; the
  instructor does not shred its demonstration cube.
- Y60 alternate size 9: superseded by dmr's public 0/200-failure size-9 record.
- Y63 size 9 rewrite: 0/10.
- Y64 size 7 rewrite: 0/5.
- Y65 size 13 rewrite: 0/52.
- Y68 unconditional size 5: 0/20; removing the side-hole guard did not produce
  an observable low-percent win.
