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

Rates are quoted at the live 1,400 s clock, measured plain and under the
shuffled-dispatch screen (the worker dispatch order shuffled every
frame).

Every entry links a **paste-ready program file** in
[SolutionsToTry/](SolutionsToTry/) — open it, select all, copy, and paste
into the level's editor.  Recipes and evidence below describe how each
file was derived and verified.

## Priority queue

**Queue refresh (2026-09-28):** the search fleet stays stopped.  A repo
review corrected two things.  The model had been silently ending every
run at the 1,400 s clock even when asked to run longer, so yesterday's
note that the Year 30 four only ever loses to a permanent freeze was
wrong: with the clock lifted it wins all 1000 test worlds, so its losses
are simply slow runs (its entry is corrected).  And the order below now
runs from the cheapest attempt to the most expensive, with the optional
items last, so the quick speed tie-breaks moved above the two long size
candidates.  The
Year 26 six that used to head this list was refuted in the game on
2026-09-06 and is archived in REJECTED_APPROACHES.md.

**Next session (50%+ rule in force — no low-percent testing for now):**

1. **Year 23 paste test, size 2**: one paste and one look at the editor,
   about a minute.  It decides whether the game keeps a command the
   level's editor does not offer; if the run then completes, it is a
   size-2 record against 6.
2. **Only if item 1 kept its command:** the Year 16 four (record 6) and
   the Year 15 five (records 8 and 6), each a couple of short runs.
3. **Year 56 size 4**: five quick attempts (about 12 s each) on the
   community's published four; 1000/1000 on both screens.  A three-size
   gain in its tier: the 99+ size row is 7.
4. **Speed tie-breaks**, each with the incumbent control run first, every
   run under a minute: Year 38 at 140, Year 09 at 14, Year 59 at 142,
   then the Year 68, 62, 65 and 67 ladders.
5. **Year 13 size 6**: two or three attempts of about ten minutes each;
   806/1000 plain, 770/1000 shuffled.  The attempt count is the tier
   evidence.
6. **Year 30 size 4 (eight-sided take)**: a few runs of about fifteen
   minutes, each to the clock; 925/1000 plain and 945/1000 shuffled
   against the published four's 618/1000 and 616/1000.
7. Optional control: the Year 15 community six, already the Solutions50+
   row on public evidence (16/25 live); 82.5% here.  Skip it if the
   Year 15 five in item 2 completes.
8. Optional confirmations of published rows, for a patient session: the
   Year 38 community speed 6-7, the Year 58 four, the Year 30
   probabilistic four and the Year 30 alternate five.
9. **Housekeeping, one minute:** check in the editor which early levels
   let one command take two directions (the last entry of this section);
   it settles several README paste markers.

Low-percent leads are parked in their own section further down until you
ask for that tier again.

### [ ] Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2 📋

- **Paste-ready program:** [SolutionsToTry/Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2.txt](<SolutionsToTry/Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2.txt>)
- Goal: settle a question that has never been tested, and take the size
  record from **6** to **2** if the answer is yes.  The game is known to
  keep things in a pasted program that its editor hides: Year 21's size
  record pastes a `myitem` test the editor does not offer at that level.
  Whether a paste also keeps a whole *command* the level's editor does
  not offer is unknown.  This program uses `write`, which the editor
  first offers at Year 32.  CONTRIBUTING.md already lists pasted programs
  that use "a command the game's editor will not let you build at that
  level", so a completion would count.
- Mechanism: every worker lifts the cube below it and writes 0 on it.
  All the held cubes then read 0, and a row of equal numbers is in order.
- Emulator evidence, with the level's command palette lifted because
  that is exactly the question: **1000/1000 plain and 1000/1000 under
  the shuffled-dispatch screen**, finishing in about 1.5 s of game time.
- Suggested live test:
  1. Paste, then look at the editor *before* running.  If the `write 0`
     line is missing, or the paste is refused, the game enforces the
     palette on paste: mark this entry refuted and skip the next two.
  2. If the line survived, run it.  A completion is a size-2 record.  If
     it runs but never completes, the paste kept the command but the goal
     wants more than equal numbers; try the next two entries anyway,
     since neither relies on that.
- Left out on purpose: the same trick also wins in the model on Budget
  Brigade 2 at size 4 (the all-left relay plus `write 0`), but its give
  list can land on a printer, which is how the relay six died in the
  game.
- Result: _not yet tested in the game_.

### [ ] Year 16 - Little Exterminator 2 - nearest one-shot at size 4 📋

- **Paste-ready program:** [SolutionsToTry/Year 16 - Little Exterminator 2 - nearest one-shot at size 4.txt](<SolutionsToTry/Year 16 - Little Exterminator 2 - nearest one-shot at size 4.txt>)
- **Try this only if the Year 23 paste kept its command.**  It uses
  `nearest`, which the editor first offers at Year 25.
- Goal: size **4** against the record of 6, which is also the game's par.
- Mechanism: Neural Pathways' published four, unchanged.  Each of the
  three workers takes the nearest cube and feeds it to the nearest
  shredder.  The room holds exactly three cubes, so one trip each
  finishes the level.
- Emulator evidence, level palette lifted: **1000/1000 plain and
  1000/1000 shuffled**, about 5 s of game time.  The three workers always
  pick three different cubes.
- Suggested live test: paste, confirm editor size 4 with both `nearest`
  lines present, and run it twice.
- Result: _not yet tested in the game_.

### [ ] Year 15 - Shred Lines - nearest loop at size 5 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - nearest loop at size 5.txt](<SolutionsToTry/Year 15 - Shred Lines - nearest loop at size 5.txt>)
- **Try this only if the Year 23 paste kept its command.**
- Goal: size **5** against the 99%+ record of 8 and the Solutions50+ six.
- Mechanism: the published five of My First Shredding Memory and
  Biometric Access, unchanged.  Each worker remembers its nearest
  shredder once, then keeps taking the nearest cube and feeding that
  shredder.
- Emulator evidence, level palette lifted: **1000/1000 plain and
  1000/1000 shuffled**, about 22 s of game time.
- Caution: a worker that loses a race for a cube still walks to its
  shredder and, arriving empty-handed, is lost; that is the mechanism
  behind the earlier Shred Lines failures.  In the model the other
  workers always finish the job, but stop at the first failed run.  It
  is queued under the exception in the working rules: losing a worker
  cannot fail this level, and the model, which includes the loss, wins
  every run.
- Suggested live test: paste, confirm editor size 5, and run it two or
  three times.
- Result: _not yet tested in the game_.

### [ ] Year 56 - Local Maximums - tier check at size 4

- **Paste-ready program:** [SolutionsToTry/Year 56 - Local Maximums - tier check at size 4.txt](<SolutionsToTry/Year 56 - Local Maximums - tier check at size 4.txt>)
- Goal: promote the community's published **low-percent size 4** row to
  Solutions50+.  Year 56 has no 50+ row at all today and its 99+ size
  record is 7, so a confirmed 50%+ four is a three-size gain in that
  tier.  The program is n05ucc4u's own, unchanged.
- Mechanism: no maximum is ever searched for.  Each worker lifts the cube
  on its north-west ring tile, overwrites it with `write 99`, and hands
  it to the room's single shredder.  The goal grades the number a cube
  shows as it goes in rather than the number it started with, so
  overwriting makes the carried cube its group's maximum instead of
  finding it.
- Emulator evidence: **1000/1000** at the live cap and **1000/1000 under
  the shuffled-dispatch screen** (re-taken 2026-09-14 on the current
  engine), finishing in about 6 s of simulated time.
- Why it sits in the low-percent tier, and what the attempts measure:
  values are drawn 0..99, so a group can already hold a 99.  That happens
  in **44% of worlds** (measured over 200) and the model passes those as
  ties — it rejects a group only when a cube still shows something
  strictly greater.  If the game demands a strict maximum the live rate
  is about **56%**; if it accepts the tie, near 100%.  Both clear the 50%
  bar, so the attempt is worth making, and a much lower live rate would
  instead expose a real emulator gap on this level, which is worth
  knowing either way.
- **This was withdrawn once, on 2026-08-18**, on the reasoning that the
  low-percent tier label already settled the live behaviour as workers
  jamming at the single shredder.  What is new is measurement rather
  than opinion: the shuffled-dispatch screen says the run is
  order-robust, so jamming is not a failure mode in anything we model,
  and the tie decomposition above gives a concrete mechanism for a
  sub-100% live rate that is not crowding.  The tier label is a reason
  to test it, not a reason to claim it.
- Suggested live test: five attempts, about 12 s each.  Record every win
  and loss; three or more wins supports the Solutions50+ row.  The one
  unmodelled risk is seven workers converging on the single shredder,
  where the model shows no crowding trouble.
- Result: _not yet tested in the game_.

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
  620 (the fallback: 200/200 at 614).  Item-action count identical.
  Re-screened 2026-09-03 with the build that implements the screen:
  199/200 for the candidate and 200/200 for the fallback, frames and
  item counts unchanged.
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
  100/100 wins at exactly the incumbent's 230.0 frames, and 200/200
  under the shuffled-dispatch screen re-taken 2026-09-03.  The only
  live risk
  is that the three extra tests cost wall time on the longest route
  (Year 47 showed an `if` is not free live) — a 3-second A/B decides it.
- Suggested live test: incumbent once as control, then the candidate.
- Result: _first paste refused (orphaned label, 2026-08-17); the fixed
  file has not been run yet_.

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
  Item-action count identical; 200/200 under the shuffled-dispatch
  screen re-taken 2026-09-03, at the same 455 frames.
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
- Result: _live attempt inconclusive_.  The published size-172 control failed
  all three attempts in this session, so it did not establish a passing
  baseline.  The correctly pasted size-171 rung then failed its one attempt;
  size 170 was not run under the stop-on-failure rule.  Keep this queued for a
  future session that first obtains a successful control run.

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
- Fidelity caveat: the current model does not reproduce the published
  incumbent, so this timing edit is live-only and has no emulator verdict.
- Suggested live test: run the incumbent as a loading/control check, then the
  candidate; capture both completion panels and editor size.
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
- Timing caveat: candidate and incumbent both fail in the current model, so
  it cannot referee either edit; this remains live-only.
- Expected editor size: **205**; expected displayed speed: **41**.
- Suggested live test: run the incumbent, size 209, size 208, and finally size
  205.  Stop the ladder at the first failure or displayed-speed regression and
  retain the last successful form.
- Result: _not yet tested locally in the game_.

### [ ] Year 13 - Injection Sites 2 - Solutions50+ size 6

- **Paste-ready program:** [SolutionsToTry/Year 13 - Injection Sites 2 - Solutions50+ size 6.txt](<SolutionsToTry/Year 13 - Injection Sites 2 - Solutions50+ size 6.txt>)
- Goal: a new Solutions50+ row at size **6** — one below the 99+ size
  record of 7, and a tier above the existing low-percent 6.
- Mechanism: the published low-percent six's random walk and gap-filling,
  with one change — after dropping into a gap the worker immediately
  re-picks the cube it just walked past (`pickup w` under the guard that
  already vouches for `w == datacube`), so each worker chains fills
  instead of retiring after one.  The opener `pickup s` is the published
  row's own; there are no direction lists on item commands anywhere, so
  the Year 60 list-stall class does not apply.
- Emulator evidence (re-taken 2026-09-14 on the current engine):
  **806/1000 plain** at ~40,000 frames (about 640 s of game time) and
  **770/1000 under the shuffled-dispatch screen** — several points under
  its plain rate, so this one is mildly order-sensitive and a first
  attempt can miss; the published low-percent row measures 83/200 =
  41.5% on the same model, and the pickup-before-drop ordering of the
  same idea 71%.  Found by the tier-upgrade hardening search 2026-09-02;
  no later hardening run improved on this form.
- Suggested live test: paste, confirm editor size 6, run at 12x; two or
  three attempts should land a win.  Capture the completion panel — the
  attempt count is the tier evidence.
- Result: _not yet tested in the game_.

### [ ] Year 30 - Fill the Floor - eight-sided take at size 4

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - eight-sided take at size 4.txt](<SolutionsToTry/Year 30 - Fill the Floor - eight-sided take at size 4.txt>)
- Goal: not a new row — the published 50%+ row is already a four — but a
  far more reliable one, and the seed for a top-tier four if hardening
  reaches 99% against the 99%+ record of **5**.
- Mechanism: identical to the published four except for one list.  That
  one refills only when the worker happens to stand on three of the
  printer's eight sides; this one names all eight, so every worker beside
  the machine reloads instead of one in three.  Nothing else changes.
- Emulator evidence (re-taken 2026-09-14 on the current engine):
  **925/1000 plain and 945/1000 under the shuffled-dispatch screen**,
  against the published four's 618/1000 and 616/1000 on the same
  measures — 62% to 92.5% for one edit.  Cardinals alone collapse to
  10.8%, so the diagonal sides carry the refill; adding a northward step
  to the walk costs 17 points.  A hardening search over 720 generations
  (2026-09-11 to 14) found nothing more reliable: its best, a reordered
  form with a five-direction step, measures 920/1000 and 935/1000.
- Diagonal printer takes are live-proven: the published four takes only
  diagonally (`takefrom nw,sw,ne`) and has 15/25 public wins, so the
  machine-reach caution on the shredder side does not carry over here.
- Unaffected by the 2026-09-06 machine rules: the program only ever
  *takes* from the printer and never gives to one, and its rate is
  identical before and after that change.  There is no shredder in the
  room, so the empty-handed give cannot bite either.
- Suggested live test: paste, confirm editor size 4, and run it a few
  times — wins take about 930 s of game time, so let each run go to the
  clock rather than restarting early.
- What to watch (corrected 2026-09-28): every lost run in the model is
  simply slow.  With the clock lifted, all 1000 test worlds finish,
  averaging about 62,000 frames against the 87,500-frame clock, so a
  loss in the game should look like a floor still a few squares short
  when time runs out, not a frozen crew.  The note this replaces said
  the losses were permanent freezes; that rested on a measurement the
  model had silently capped at the clock.  Since the losses are only
  slowness, a faster walk at the same size is the lever toward the 99%+
  record of **5**.
- Result: _not yet tested in the game_.

### [ ] Year 15 - Shred Lines - community size 6 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - community size 6.txt](<SolutionsToTry/Year 15 - Shred Lines - community size 6.txt>)
- Goal: locally confirm abfipes12's public size-6 program, imported to
  Solutions50+ below our size-8 main row (found in the 2026-08-17 source
  audit; it was never in our tables).
- Public evidence: abfipes12 reports 16/25 real-game wins (64%) at about
  950 seconds.  Those wins were the anchor for the Year 15 machine
  rules, settled in the game on 2026-09-06: a step aimed at a shredder
  is a fence (the worker stays put), and an empty-handed give at a
  shredder hands the worker over.  Our own Year 15 four and five, which
  relied on the old model, failed live that day and are archived in
  REJECTED_APPROACHES.md; this six is now the level's smallest published
  program.
- Measured here under the machine rules: **82.5%** at the live cap
  (95.5% before them), about 54,600 frames.
- Mechanism: a random seven-direction walk with a guarded pickup/give; the
  give lands on the south shredder row.  No `myitem` anywhere, so the
  refuted Year 15 gated-form class does not apply.
- Expected editor size: **6**; paste-only (multi-direction random step).
- The Solutions50+ row already rests on the public evidence, so a local
  confirmation is optional — after the size candidates above.  A
  shrink search for a five under the machine rules (300 narrow
  generations, then a short run of the wider operator) found nothing.
- Result: _not tested locally; the row stands on public evidence_.

### [ ] Year 38 - Seek and Destroy 3 - community speed 6-7 at size 122

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt>)
- Goal: locally confirm abfipes12 and commonnickname's public speed
  program, imported to Solutions50+ below our 9-10 main speed row
  (found in the 2026-08-17 source audit; it was never in our tables).
- Public evidence: 67/125 real-game wins (53.6%) at displayed speed 6-7.
- Local emulator: 36/50 wins, average 423.6 frames (win/fail evidence
  only; displayed speed is async wall-time and theirs is live-measured).
- Expected editor size: **122**; glitchless, so it should also be
  typable/editable normally.
- Suggested live test: repeated runs until a win; capture displayed speed
  and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 58 - Good Neighbors - size 4

- **Paste-ready program:** [SolutionsToTry/Year 58 - Good Neighbors - size 4.txt](<SolutionsToTry/Year 58 - Good Neighbors - size 4.txt>)
- Goal: live-validate the existing
  [Solutions50+ entry](<Solutions50+/Year 58 - Good Neighbors (size).txt>).
- Current capped-emulator evidence: 195/200 wins at the real 87,500-frame
  deadline, average winning speed 483.3, range 85-1,379.  The five failures
  confirm that this belongs in Solutions50+, not Solutions99+.
- Suggested live test: at least 10 runs.  A definitive frozen failure is all 20
  workers holding cubes while the level has not completed; capture the board.
- A hardening search from this program (2026-09-11 to 14, 140
  generations) found no more reliable four; the published program stands
  at 195/200 here.
- Measured again 2026-09-28: 942/1000 at the live clock and 961/1000 with
  the clock lifted, so 39 of its losses are genuinely stuck worlds (the
  frozen failure described above) and the rest are slow runs.
- Result: **one live attempt (2026-08-17) looked like an infinite loop** and
  was abandoned before the 1,400-second cutoff.  That matches either the
  known ~2.5% emulator failure mode or a live/emulator divergence at
  contended cubes; the winning tail is slow (emulator range 85-1,379
  seconds), so a run only counts as failed at the cutoff or visibly frozen.
  Low priority until a patient full-length session.

### [ ] Year 30 - Fill the Floor - probabilistic size 4

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt](<SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt>)
- Goal: live-confirm the size-4 Solutions50+ row (already published at
  ~1211) below the size-5 main entry.
- **Superseded as a paste target (2026-09-14)** by the eight-sided four
  in item 6: same size, 925/1000 against this program's 618/1000 on the
  current engine.  Run this one only as the control if
  you want the A/B, or to confirm the published row as it stands.
- Machine-reach note: this program takes from printers diagonally
  (`takefrom nw,sw,ne`), and its 15/25 public wins show that a diagonal
  printer take is safe, unlike the diagonal shredder give that killed
  Year 21's givers.
- Public evidence: abfipes12 and martinez8859 report 15/25 wins (60%).
- Current emulator evidence: 618/1000 plain and 616/1000 shuffled
  (2026-09-14).  Winning runs average about 1,211 s, so they often finish
  only just before the game deadline.
- Expected editor size: **4**; paste-only because of the multi-direction
  `takefrom`.
- Suggested live test: 10 uninterrupted runs, allowing every run to reach the
  game's own deadline.
- Result: _not yet tested locally in the game_.

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

### [ ] Housekeeping - which levels let the editor give one command two directions

- No paste file: this is a look at the editor, not a program.
- Why: the README's paste marker is inconsistent for direction lists.
  Year 6's record (`pickup c,s`) carries it; Year 4's (`pickup c,e`),
  Year 10's size row (`step n,s`) and Year 12's speed row (four step
  lists) do not; and this queue calls Year 13's and Year 22's step lists
  paste-only.  An open report on the upstream repository says Year 4's
  list cannot be built in the editor.
- What to do: open the editor on Years 4, 10 and 13, add a `step` or a
  `pickup`, and try to select two directions on it.  Note for each level
  whether the editor allows it; the README markers follow from that.
- Result: _not yet checked_.

## Low-percent leads (parked — 50%+ only for now)

Valid candidates below the Solutions50+ bar.  Not for the next session;
kept intact so nothing is rediscovered.

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
  The intended random-state model gives about **2.23%** wins; an exact
  2,000,000-state spot check produced 2.23085%.
- Expected editor size: **3**; the multi-direction `giveto` syntax is already
  established in exported solutions.
- Suggested live test: 100-200 fast fresh attempts at 12x, capturing the first
  completion and editor size.  Do not substitute diagonal `giveto sw`.
- Local-emulator cross-check (2026-08-22): 53/3,000 = **1.77%** against the
  2.23% claim — the same order of magnitude in a second model, still above
  the queue floor.
- Caution (2026-09-06 machine rules): the supervisor's give runs straight
  after its empty pickup, so if the relay's timing slips it gives
  empty-handed beside the shredder and is lost.  Both rates predate those
  rules; re-screen before any live attempt.
- Result: _not yet tested locally in the game_.

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

### [ ] Year 06 - Little Exterminator 1 - exact-route low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt](<SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt>)
- Goal: establish a practical size-**5** SolutionsLowPercent row below the
  published size-7 low-percent and size-8 main entries.
- Mechanism: six required binary moves reach the lower funnel with probability
  1/64; seven of the eight three-step tails then reach a position whose west
  pickup takes the cube.  Every earlier deviation falls into a hole.
- Exact density over the level's random start states:
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
- Caution (2026-09-06 machine rules): a worker that loses the race for a
  cube still gives, empty-handed, at the shredder and is lost.  The rate
  predates those rules; re-screen before any live attempt.
- Result: _not yet tested locally in the game_.

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
  the iid calculation is 0.072778626%, and a concrete winning start state
  exists in the model.
- Expected editor size: **2**.  Size 1 cannot both acquire cubes and remove
  redundant workers.
- Suggested live test: none under the current cutoff; retain for a future
  reproducible RNG harness or a lucky natural completion.
- Result: _not yet tested locally in the game_.

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
  program and is not a trustworthy judge of large crowds.  This is a live-only
  candidate with a positive finite schedule, not a measured success rate.
- Expected editor size: **3**; paste-only because of `step s,e,se`.
- Suggested live test: repeated fresh runs, capturing every exactly-seven
  survivor pattern and the first completion panel.
- Result: _not yet tested locally in the game_.

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
- Caution: the eight-target pickup has to skip empty squares, which the
  game does not do (Year 60), so the model's rate is not trustworthy.
- Suggested live test: repeated quick resets only if pursuing a very rare
  record.  The observed rate implies thousands of attempts per win, so this is
  lower priority than the deterministic and main-tier candidates.
- Result: _not yet tested locally in the game_.

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
- Witness: an independent exact search of the model's random worlds
  predicted the first
  winning world at seed 69,510 with counts `[13,9,11,11,12,8]` and samples
  `[5,1,3,3,4,0]`.  The capped emulator then found exactly 1/69,510 wins, at
  that final seed, completing in 342 frames with displayed speed 6.
- Expected editor size: **6**; every command is available in Year 52.
- Suggested live test: this is mathematically sound but far too rare for a
  practical manual campaign.  Preserve it for a lucky natural run or a future
  reproducible live-game RNG harness; capture the completion panel if tested.
- Result: _not yet tested locally in the game_.

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
- Witness: an independent search of the model's random worlds found seed
  3,868,438.
  All five eight-value groups sum to 36, and the isolated seed-offset emulator
  won in 2,099 frames with displayed speed 34 and 172 item actions.
- Expected editor size: **5**.
- Entry method: paste the text because the ordered multi-target `pickup c,s`
  is not constructible from Year 55's normal editor controls.
- Caution: `pickup c,s` has to skip the empty centre to reach the petal,
  and the game does not skip an empty listed square (Year 60), so this
  program most likely stalls in the game.
- Suggested live test: natural manual verification is impractical without a
  reproducible RNG-start method.  Capture the completion panel if the matching
  world can be reproduced.
- Result: _not yet tested locally in the game_.

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
- Witness: an independent search of the model's random worlds found seed
  3,281,406.
  Its selected values are `[70,87,83,79,96,98,90]`, each the maximum of its
  group.  An isolated seed-offset emulator run then won in 310 frames with
  displayed speed 5 and 14 item actions.
- Expected editor size: **3**; all commands are available in Year 56.
- Suggested live test: preserve this as a mathematically witnessed rare record;
  natural manual verification is impractical without a reproducible RNG-start
  method.  Capture the completion panel if the matching world occurs.
- Result: _not yet tested locally in the game_.

### [ ] Year 62 - The Sorting Floor - initially sorted size 0

- **Paste-ready program:** [SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt](<SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt>)
- Goal: establish a zero-command SolutionsLowPercent record below the size-10
  Solutions50+ and size-12 main entries.
- Mechanism: do nothing.  The nine independent random cubes occasionally
  spawn in weakly increasing row-major order, satisfying the goal before the
  first frame is processed.
- Exact probability: `C(108, 9) / 100^9 = 3.9113958819e-6`, or about one win
  in 255,663 worlds.
- Witness: an independent search of the model's random worlds found seed
  239,189, whose values in row-major order are
  `[10,14,18,41,62,69,80,88,95]`; the isolated seed-offset emulator accepted
  the label-only program at frame 0 with size 0 and displayed speed 0.
- Expected editor size: **0**; the free label is present only to make the text
  pasteable and does not count as a command.
- Suggested live test: natural manual verification is impractical.  If a
  reproducible live-game RNG-start method becomes available, paste the free
  label, run the matching world, and capture the immediate completion panel.
- Result: _not yet tested locally in the game_.

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
- Caution (2026-09-06 machine rules): a worker that loses the race for a
  cube still gives, empty-handed, at the shredder and is lost.  The rate
  predates those rules; re-screen before any live attempt.
- Result: _not yet tested locally in the game_.
