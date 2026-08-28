# Solutions to Try in the Real Game

This is the live-game verification queue.  A candidate stays here until the
game itself reaches its completion screen (or an explicit failure), because an
emulator win is evidence rather than proof.  The game stops a run after 1,400
seconds, so an unfinished run at that point is a failure, not an eventual win.

For every attempt, record the editor-reported size, completion/failure, the
displayed speed, and a screenshot.  For stochastic programs, also record the
attempt number and do not restart merely because a run looks slow.

Rejected and superseded experiments are archived in
[REJECTED_APPROACHES.md](REJECTED_APPROACHES.md).

**Speed-evidence downgrade (2026-08-15):** two live A/Bs (Years 39 and
40) proved the game's displayed speed is asynchronous wall-time —
frame-identical simulator evidence does NOT establish it, and diagonal
step substitutions regressed 36 to 41 in both tests.  Every speed
tie-break below therefore requires a live incumbent control run first;
discard the candidate on any displayed-speed regression.  Win/loss and
size evidence is unaffected.

Every entry links a **paste-ready program file** in
[SolutionsToTry/](SolutionsToTry/) — open it, select all, copy, and paste
into the level's editor.  Recipes and evidence below describe how each
file was derived and verified.

## Priority queue

### [ ] Year 38 - Seek and Destroy 3 - cardinal-relay low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - cardinal-relay low-percent size 3.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - cardinal-relay low-percent size 3.txt>)
- Goal: establish a size-**3** SolutionsLowPercent row below the size-8
  Solutions50+ and size-10 Solutions99+ entries.
- Mechanism: the bottom workers move northwest and pick distinct northeast
  cubes.  Only the leftmost carrier can give west to the empty supervisor;
  every other carrier targets a still-full neighbour and immediately errors.
  The rendezvous pins the supervisor through its empty-pickup error, after
  which its `giveto w,s` selects the cardinal south shredder.
- The level wins exactly when the one relayed cube is a weak global minimum.
  The intended random-state model gives about **2.23%** wins; a source-exact
  2,000,000-state spot check produced 2.23085%.
- Expected editor size: **3**; the multi-direction `giveto` syntax is already
  established in exported solutions.
- Suggested live test: 100-200 fast fresh attempts at 12x, capturing the first
  completion and editor size.  Do not substitute diagonal `giveto sw`.
- Local-emulator cross-check (2026-08-22): 53/3,000 = **1.77%** against the
  2.23% claim — the same order of magnitude in a second model, still above
  the queue floor.  One mechanism caution: live pickup-lists were proven
  NOT to skip an ineligible square (Year 60 — the worker stalls); if
  give-lists behave the same way, the supervisor's `w,s` fall-through
  stalls instead of selecting the shredder.  The cheap live attempts
  double as the discriminator for that rule.
- Result: _not yet tested locally in the game_.

```text
step nw
pickup ne
giveto w,s
```

### [ ] Year 44 - Unique Fashion Party - static-cull low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - static-cull low-percent size 3.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - static-cull low-percent size 3.txt>)
- Goal: establish a practical size-**3** SolutionsLowPercent row below the
  public size-4 low-percent and size-5 Solutions99+ entries.
- Mechanism: the three-term static predicate kills 38 workers before pickup
  and leaves exactly seven stable survivors.  Four survivors hold guaranteed
  members of the level's shuffled 0-6 set; the remaining three hold ordinary
  random cubes.  A win occurs when those three supply the missing values.
- Nominal probability: `3! / 7^3 = 6 / 343`, or **1.749271%**.  A faithful
  1,000,000-state model produced 17,405 wins (1.7405%).
- Expected editor size: **3**; the condition has three terms, below the parser
  limit, and `if`, `calc`, and `pickup` are all available in Year 44.
- Suggested live test: 100-200 fresh attempts at 12x.  On a stable loss, verify
  that exactly seven workers survive; any other survivor count falsifies the
  static classification immediately.
- Result: _not yet tested locally in the game_.

```text
if ne != wall and
 sw != wall or
 n != datacube:
	mem1 = calc 0 / 0
endif
pickup s
```

### [ ] Year 06 - Little Exterminator 1 - exact-route low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt](<SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt>)
- Goal: establish a practical size-**5** SolutionsLowPercent row below the
  published size-7 low-percent and size-8 main entries.
- Mechanism: six required binary moves reach the lower funnel with probability
  1/64; seven of the eight three-step tails then reach a position whose west
  pickup takes the cube.  Every earlier deviation falls into a hole.
- Exact full-state density: `7 * 2^23 / (2^32 - 1)`, or
  **1.367187500318%**.  There is one absorbing losing tail at `(7,10)`.
- Expected editor size: **5**; the label is free and the program has one
  pickup, three steps, and one jump.  Paste is required for the direction
  lists at this early level.
- Suggested live test: repeated fresh attempts at 12x, resetting shortly
  after the longest successful path; do not wait for the 1,400-second cap when
  the worker is visibly parked in the losing corner.
- Local-emulator cross-check (2026-08-22): 36/3,000 = **1.20%**, within one
  sigma of the exact 1.367% claim — the rate is confirmed by a second model.
- Result: _not yet tested locally in the game_.

```text
a:
pickup w
step s,se
step e,sw
step sw,se
jump a
```

### [ ] Year 38 - Seek and Destroy 3 - speed tie-break at size 140 (fallback 141)

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break at size 140.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break at size 140.txt>)
- **Fallback (size 141):** [SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break fallback at size 141.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break fallback at size 141.txt>)
- Goal: retain the displayed 9-10 while reducing the size from 142 to
  **140**.
- Exact edit: inside the second `if mem3 != mem4:` block, the column walk
  goes `step w`, five `step n`, `step e`, `step w`.  The first `step w`
  and the `step e` cancel (identical endpoint), so both are deleted; the
  fallback deletes only the `step w`.
- Emulator evidence: 199/200 wins at 606 frames versus the incumbent's
  620 (the fallback: 200/200 at 614).  Item-action count identical;
  100/100 under the shuffled-dispatch screen.
- Suggested live test: incumbent once as control, then the candidate;
  9-second runs.
- Result: _not yet tested in the game_.

### [ ] Year 09 - Dynamic Angles - speed tie-break at size 14

- **Paste-ready program:** [SolutionsToTry/Year 09 - Dynamic Angles - speed tie-break at size 14.txt](<SolutionsToTry/Year 09 - Dynamic Angles - speed tie-break at size 14.txt>)
- Goal: retain the displayed speed of 3 while reducing the size from 15 to
  **14** (martinez8859, n05ucc4u and abfipes12's program; keep the credits).
- Exact edit: delete the first `jump a` (inside the first `if e == nothing:`
  block) **and the label `a:` it pointed to** (labels are free, so the size
  is unchanged).  Without the jump, the workers on the longest route re-test
  `e == nothing` at each following block instead of jumping past the test;
  on this level's diagonal edge those tests are always true, so the walk is
  the same.
- **Paste-validity fix after the first live attempt (2026-08-17):** the
  first cut deleted only the jump and left `a:` behind, and the game
  refused the paste — a label can only exist as some jump's destination,
  so an orphaned label makes the whole program invalid.  The paste file
  now removes the pair.  (Standing rule for deletion candidates: a deleted
  jump takes its label with it.)
- Emulator evidence (corpus deletion sweep, re-run on the fixed file):
  100/100 wins at exactly the incumbent's 230.0 frames.  100/100 under
  the shuffled-dispatch screen.  The only live risk
  is that the three extra tests cost wall time on the longest route
  (Year 47 showed an `if` is not free live) — a 3-second A/B decides it.
- Suggested live test: incumbent once as control, then the candidate.
- Result: _not yet tested in the game_.

### [ ] Year 59 - Glory Hole - speed tie-break at size 142

- **Paste-ready program:** [SolutionsToTry/Year 59 - Glory Hole - speed tie-break at size 142.txt](<SolutionsToTry/Year 59 - Glory Hole - speed tie-break at size 142.txt>)
- Goal: retain the displayed 6 while reducing the size from 144 to
  **142**.
- Exact edit: in the else-arm walk `step w / step sw / step sw / step sw /
  step e`, the `step w` and `step e` cancel — the three diagonals land on
  the same square, two commands shorter.
- Emulator evidence: 300/300 wins, deterministic at 455 frames versus the
  incumbent's 447 — 8 frames slower in the model, so the displayed speed
  needs the live A/B (control run first, discard on regression).
  Item-action count identical; 100/100 under the shuffled-dispatch screen.
- Result: _not yet tested in the game_.

### [ ] Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170 (fallback 171)

- **Paste-ready program:** [SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170.txt](<SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170.txt>)
- **Fallback (size 171):** [SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed fallback at size 171.txt](<SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed fallback at size 171.txt>)
- Goal: retain the displayed speed record of 16 while reducing the secondary
  size from 172 to **170** (or 171 at the conservative rung).
- Exact edit: delete both consecutive top-level `tell everyone hi` commands at
  incumbent lines 195-196; restore either one for the size-171 fallback.  No
  `listenfor` exists, so the deleted greetings carry no data and only change
  asynchronous cadence.
- Why this is reopened: the size-171 form was queued originally, then 171/170
  moved to rejected only because the nominally stronger bypassed-wrapper
  size-164 program dominated them.  Size 164 later failed live, leaving the
  tell-only rungs needing an independent test.
- Live ladder: run the incumbent size-172/speed-16 program as control, then 171,
  then 170.  Stop at the first failure or displayed speed above 16.
- Live result at 12x: _inconclusive_.  The published size-172 control failed
  all three attempts in this session, so it did not establish a passing
  baseline.  The correctly pasted size-171 rung then failed its one attempt;
  size 170 was not run under the stop-on-failure rule.  Keep this queued for a
  future session that first obtains a successful control run.

### [ ] Year 23 - Sorting Hall - low-percent speed tie-break at size 19 (fallback 21)

- **Paste-ready program:** [SolutionsToTry/Year 23 - Sorting Hall - low-percent speed tie-break at size 19.txt](<SolutionsToTry/Year 23 - Sorting Hall - low-percent speed tie-break at size 19.txt>)
- **Fallback (size 21):** [SolutionsToTry/Year 23 - Sorting Hall - low-percent speed fallback at size 21.txt](<SolutionsToTry/Year 23 - Sorting Hall - low-percent speed fallback at size 21.txt>)
- Goal: retain the low-percent speed row's displayed ~14 while reducing its
  size from 23 to **19** (n05ucc4u's program; keep the credit).
- Exact edits: delete the two three-line re-check tails — in the `> 49`
  branch `if w > myitem: jump d / endif` and in the `else` branch
  `if e < myitem: jump h / endif`.  The size-21 fallback deletes only the
  first of them.
- Emulator evidence: 300-trial A/B on one model — incumbent 155 wins at
  16.3 modelled seconds; size 21: 135 wins at 16.2; size 19: 114 wins at
  16.2.  The win rate drops from about 52% to about 38-45% (still the
  low-percent tier) with the speed distribution unchanged.
- Suggested live test: repeated ~14-second runs until a win; capture the
  displayed speed and editor size.
- Result: _not yet tested in the game_.

### [ ] Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13 📋

- **Paste-ready program:** [SolutionsToTry/Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13.txt](<SolutionsToTry/Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13.txt>)
- Goal: establish a low-percent size-13 program that retains or improves the
  reliable incumbent's displayed speed of 5 and is smaller than its size 16.
- Exact edit: in the current speed program, replace the final five-line
  `if n == nothing: step n; else: step s; endif` with random `step n,s`.
- Capped-emulator evidence: 1/100 candidate runs won in 278 frames with
  modelled speed 5 and 18 item actions.  The incumbent was 100/100 at 311
  frames and 24 actions; canonical sizes are 13 and 16.
- Construction caveat: random multi-direction movement is paste-only here, so
  retain the clipboard marker and classify the program as LowPercent.
- Suggested live test: repeated quick attempts; on a win, capture both the
  displayed speed and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17 📋

- **Paste-ready program:** [SolutionsToTry/Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17.txt](<SolutionsToTry/Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17.txt>)
- Goal: establish a low-percent speed-5 program at size 17 versus the reliable
  incumbent's size 20.
- Exact edit: retain the outer guard, but replace its inner
  `if ne != datacube: step ne; else: step sw; endif` with random `step ne,sw`.
- Capped-emulator A/B evidence: candidate won 23/100.  Every winning run was
  frame-identical to the incumbent at 326 frames and modelled speed 6, while
  using 21 instead of 24 item actions.  Canonical sizes are 17 and 20; the
  incumbent's authoritative live score is 5.
- Construction caveat: the random diagonal step is paste-only, so retain the
  clipboard marker and LowPercent classification.
- Suggested live test: a handful of attempts should normally produce a win;
  capture the completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83

- **Paste-ready program:** [SolutionsToTry/Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83.txt](<SolutionsToTry/Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83.txt>)
- Goal: retain the current low-percent speed record of about 6 while reducing
  its secondary size from 84 commands to 83.
- Exact edit: start from
  [the current low-percent speed program](<SolutionsLowPercent/Year 34 - Seek and Destroy 1 (speed).txt>)
  and delete the third `mem1 = nearest datacube` in the `mem2 == mem3` branch,
  immediately before `if n <= mem2`.
- Why it should be safe: no path reads that value; after the branch picks up
  either north or `mem2`, every continuation overwrites `mem1` with the nearest
  shredder.  `nearest` itself is timing-free in the validated model.
- Same-seed emulator A/B evidence: candidate and incumbent won the identical
  5/20 seeds, each in exactly 411 frames with modelled speed 7 and 11 item
  actions.  Canonical sizes are 83 and 84.
- Expected editor size: **83**; expected displayed speed: **about 6**.
- Suggested live test: run repeated candidate attempts until it wins, then
  capture the completion panel and editor size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214 (fallback 215)

- **Paste-ready program:** [SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214.txt](<SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214.txt>)
- **Fallback (size 215):** [SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store fallback size 215.txt](<SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store fallback size 215.txt>)
- Goal: retain the current displayed speed range of 9-12 while reducing the
  secondary size from 216 commands to 214, with the tested one-store deletion
  at size 215 as the fallback.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 62 - The Sorting Floor (speed).txt>)
  and delete both consecutive `mem1 = set myitem` commands immediately before
  `tell everyone hi` and the divide-by-zero exit.  To test the size-215
  fallback, restore either one of them.
- Why it may be safe: neither saved value is read before that worker's exit;
  only the two synchronization delays can matter.
- Fallback emulator A/B evidence: the size-215 candidate and incumbent both
  win seed 1 in exactly 836 frames.  Across the same 100 model worlds each wins
  36, with modelled
  speed 12.3 and nearly identical average frames (734.9 versus 735.0).
- Size-214 bounded gate: 9/20 model worlds won, with modelled speed averaging
  10.9 (range 6-14), winning frames averaging 648.0 (range 371-836), and 40.4
  average item actions.  The observed 45% is encouraging but not statistically
  decisive against the established 36/100 baseline.
- Fidelity caveat: the current model badly under-reproduces the published
  incumbent reliability, so those paired aggregates support equivalence but
  cannot establish live reliability or timing.
- Expected editor size: **214** if the extra cadence cut survives; otherwise
  use the tested **215** fallback.
- Suggested live test: run the incumbent, size 215, and size 214 in that order;
  stop at the first regression and capture each completion panel and editor
  size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 65 - Defrag Ordered - live-only speed tie-break at size 120

- **Paste-ready program:** [SolutionsToTry/Year 65 - Defrag Ordered - live-only speed tie-break at size 120.txt](<SolutionsToTry/Year 65 - Defrag Ordered - live-only speed tie-break at size 120.txt>)
- Goal: retain the current displayed speed record of 12 while reducing the
  secondary size from 121 commands to 120.
- Exact edit: in the inner `else` branch, delete the empty
  `mem3 = foreachdir nw,w,sw,n,ne,e,se:` loop and its matching `endfor`, leaving
  `comment 1` followed directly by `step e`.
- Why it may be safe: `mem3` is never read, and the loop body is empty; its only
  effect is seven iterations of cadence delay.
- Fidelity caveat: the current extracted model does not reproduce the published
  incumbent, so this timing edit is live-only and has no emulator verdict.
- Suggested live test: run the incumbent as a loading/control check, then the
  candidate; capture both completion panels and editor size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 67 - Decimal Doubler - live-only speed tie-break at size 205 (fallbacks 208/209)

- **Paste-ready program:** [SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt](<SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt>)
- **Fallback (size 208):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt>)
- **Fallback (size 209):** [SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt](<SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt>)
- Goal: retain the current displayed speed record of 41 while reducing the
  secondary size from 210 commands to 205.
- Exact edits: after opening `pickup ne; step n`, delete all three consecutive
  `mem2 = set c` commands; in loop `j`, after `step mem3; step e`, delete both
  consecutive `tell everyone hi` commands.
- Why they may be safe: every `mem2` occurrence is an assignment, never a read,
  and the program contains no `listenfor`; all five deleted commands are
  cadence only.  Size 208 deletes one store and one tell; the conservative
  tell-only fallback is size 209.
- Timing caveat: candidate and incumbent both fail the current extracted-level
  model, so it cannot referee either edit; this remains live-only.
- Expected editor size: **205**; expected displayed speed: **41**.
- Suggested live test: run the incumbent, size 209, size 208, and finally size
  205.  Stop the ladder at the first failure or displayed-speed regression and
  retain the last successful form.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 15 - Shred Lines - community size 6 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - community size 6.txt](<SolutionsToTry/Year 15 - Shred Lines - community size 6.txt>)
- Goal: locally confirm abfipes12's public size-6 program, newly imported to
  Solutions50+ below our size-8 main row (found in the 2026-08-17 source
  audit; it was never in our tables).
- Public evidence: abfipes12 reports 16/25 real-game wins (64%) at about
  950 seconds.  Local emulator: 47/50 wins, average 56,503 frames — same
  ballpark, though Year 15's model is known to diverge on gated forms.
- Mechanism: a random seven-direction walk with a guarded pickup/give; the
  give lands on the south shredder row.  No `myitem` anywhere, so the
  refuted Year 15 gated-form class does not apply; random steps fence at
  machine rows live (the Year 42 precedent).
- Expected editor size: **6**; paste-only (multi-direction random step).
- Suggested live test: a few full-length runs — wins are SLOW (near 1,000
  seconds), so let each run reach the game's own cutoff.
- Result: _not yet tested locally in the game_.

### [ ] Year 38 - Seek and Destroy 3 - community speed 6-7 at size 122

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt>)
- Goal: locally confirm abfipes12 and commonnickname's public speed
  program, newly imported to Solutions50+ below our 9-10 main speed row
  (found in the 2026-08-17 source audit; it was never in our tables).
- Public evidence: 67/125 real-game wins (53.6%) at displayed speed 6-7.
- Local emulator: 36/50 wins, average 423.6 frames (win/fail evidence
  only; displayed speed is async wall-time and theirs is live-measured).
- Expected editor size: **122**; glitchless, so it should also be
  typable/editable normally.
- Suggested live test: repeated runs until a win; capture displayed speed
  and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 44 - Unique Fashion Party - low-percent size 4

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - low-percent size 4.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - low-percent size 4.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 58 - Good Neighbors - size 4.txt](<SolutionsToTry/Year 58 - Good Neighbors - size 4.txt>)
- Goal: live-validate the existing
  [Solutions50+ entry](<Solutions50+/Year 58 - Good Neighbors (size).txt>).
- Current capped-emulator evidence: 195/200 wins at the real 87,500-frame
  deadline, average winning speed 483.3, range 85-1,379.  The five failures
  confirm that this belongs in Solutions50+, not Solutions99+.
- Suggested live test: at least 10 runs.  A definitive frozen failure is all 20
  workers holding cubes while the level has not completed; capture the board.
- Result: **one live attempt (2026-08-17) looked like an infinite loop** and
  was abandoned before the 1,400-second cutoff.  That matches either the
  known ~2.5% emulator failure mode or a live/emulator divergence at
  contended machines; the winning tail is slow (emulator range 85-1,379
  seconds), so a run only counts as failed at the cutoff or visibly frozen.
  Low priority until a patient full-length session.

### [ ] Year 30 - Fill the Floor - probabilistic size 4

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt](<SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt>)
- Goal: live-confirm the size-4 Solutions50+ row (already published at
  ~1211) below the size-5 main entry.
- Machine-reach caution (added after the Year 21/24 live refutations):
  this program takes from printers DIAGONALLY (`takefrom nw,sw,ne`).
  Diagonal SHREDDER gives proved fatal live (the giver walks in); if the
  diagonal printer take behaves the same way, workers will die at the
  printers and this row must be retracted.  Watch for exactly that on
  the confirmation run.
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

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - alternate size 5.txt](<SolutionsToTry/Year 30 - Fill the Floor - alternate size 5.txt>)
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

### [ ] Year 05 - An Important Decision - absorbing low-percent size 2

- **Paste-ready program:** [SolutionsToTry/Year 05 - An Important Decision - absorbing low-percent size 2.txt](<SolutionsToTry/Year 05 - An Important Decision - absorbing low-percent size 2.txt>)
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

### [ ] Year 13 - Injection Sites 2 - recoverable low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 13 - Injection Sites 2 - recoverable low-percent size 5.txt](<SolutionsToTry/Year 13 - Injection Sites 2 - recoverable low-percent size 5.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 22 - Number Royale - survivor low-percent size 3.txt](<SolutionsToTry/Year 22 - Number Royale - survivor low-percent size 3.txt>)
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

### [ ] Year 54 - Terrain Leveler - constant-average low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 54 - Terrain Leveler - constant-average low-percent size 5.txt](<SolutionsToTry/Year 54 - Terrain Leveler - constant-average low-percent size 5.txt>)
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

### [ ] Year 38 - Seek and Destroy 3 - one-shot low-percent size 4

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - one-shot low-percent size 4.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - one-shot low-percent size 4.txt>)
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

## Parked long shots (win rate below 1 in 100)

Per the maintainer's rule, candidates that win less often than **1 in
100 runs** are not part of the live-game queue: a witnessed completion
would cost more restart-grinding than the row is worth.  They remain
here (with their paste-ready files) in case the rule changes or a
higher-rate variant is found.  Nothing below this line needs game time.

### [ ] Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2.txt>)
- Goal: preserve the absolute size-**2** construction below the practical
  size-3 candidate.  It is parked because its rate is far below the 1% live
  queue threshold.
- Mechanism: `pickup n` leaves 35 cube-holders.  After those pickups, exactly
  ten still see an unpicked cube south; everyone else divides by zero and
  dies.  A win occurs when exactly seven south denominators are nonzero and
  the corresponding seven held labels are the complete set 0-6.
- Probability evidence: 131/200,000 faithful modeled states won (0.0655%);
  the iid calculation is 0.072778626%.  Initial xorshift state `0xa84e338f`
  is a concrete finite-state witness.
- Expected editor size: **2**.  Size 1 cannot both acquire cubes and remove
  redundant workers.
- Suggested live test: none under the current cutoff; retain for a future
  reproducible RNG harness or a lucky natural completion.
- Result: _not yet tested locally in the game_.

```text
pickup n
mem1 = calc 0 / s
```

### [ ] Year 44 - Unique Fashion Party - transient-survivor size 3

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - transient-survivor size 3.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - transient-survivor size 3.txt>)
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

### [ ] Year 12 - Unzip - one-shot low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 12 - Unzip - one-shot low-percent size 3.txt](<SolutionsToTry/Year 12 - Unzip - one-shot low-percent size 3.txt>)
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

### [ ] Year 06 - Little Exterminator 1 - monotone low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 06 - Little Exterminator 1 - monotone low-percent size 3.txt](<SolutionsToTry/Year 06 - Little Exterminator 1 - monotone low-percent size 3.txt>)
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

### [ ] Year 52 - The Mode Code - one-shot low-percent size 6

- **Paste-ready program:** [SolutionsToTry/Year 52 - The Mode Code - one-shot low-percent size 6.txt](<SolutionsToTry/Year 52 - The Mode Code - one-shot low-percent size 6.txt>)
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

### [ ] Year 55 - Data Flowers - constant-sum low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 55 - Data Flowers - constant-sum low-percent size 5.txt](<SolutionsToTry/Year 55 - Data Flowers - constant-sum low-percent size 5.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 56 - Local Maximums - one-shot low-percent size 3.txt](<SolutionsToTry/Year 56 - Local Maximums - one-shot low-percent size 3.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt](<SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 26 - Budget Brigade 2 - all-left low-percent size 3.txt](<SolutionsToTry/Year 26 - Budget Brigade 2 - all-left low-percent size 3.txt>)
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

### [ ] Year 33 - Data Backup Day - one-shot low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 33 - Data Backup Day - one-shot low-percent size 5.txt](<SolutionsToTry/Year 33 - Data Backup Day - one-shot low-percent size 5.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 34 - Seek and Destroy 1 - one-shot low-percent size 4.txt](<SolutionsToTry/Year 34 - Seek and Destroy 1 - one-shot low-percent size 4.txt>)
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
