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

### [x] Year 40 - Printing Etiquette 2 - PR-92 ladder — RECORD PUBLISHED at 176 / 36

- **Live result (maintainer, 2026-08-15): rung 1 (size 176) completed at
  36 s — beats the 177/37 row on both axes and is PUBLISHED** to
  Solutions99+ with the README row updated (credit @commonnickname,
  upstream PR #92).
- Rung 2 (size 175, the data-dead calc deletion) REFUTED live: 38 s.  A
  frame-identical pure deletion still cost two seconds — dead code can be
  load-bearing timing (see the rejected ledger).
- File kept: [SolutionsToTry/Year 40 - Printing Etiquette 2 - PR-92 step deletion at size 176.txt](<SolutionsToTry/Year 40 - Printing Etiquette 2 - PR-92 step deletion at size 176.txt>)

### [ ] Year 15 - Shred Lines - event-gated size 5

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - event-gated size 5.txt](<SolutionsToTry/Year 15 - Shred Lines - event-gated size 5.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 60 - Understaffed Sorting - ordered-pickup size 8.txt](<SolutionsToTry/Year 60 - Understaffed Sorting - ordered-pickup size 8.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - deterministic size 7.txt](<SolutionsToTry/Year 15 - Shred Lines - deterministic size 7.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - size 5 speed tie-break.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - size 5 speed tie-break.txt>)
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

- **Paste-ready program:** [SolutionsToTry/Year 47 - Automated Pleasantries - speed tie-break at size 33.txt](<SolutionsToTry/Year 47 - Automated Pleasantries - speed tie-break at size 33.txt>)
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
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 36 - Seek and Destroy 2 - speed tie-break at size 211

- **Paste-ready program:** [SolutionsToTry/Year 36 - Seek and Destroy 2 - speed tie-break at size 211.txt](<SolutionsToTry/Year 36 - Seek and Destroy 2 - speed tie-break at size 211.txt>)
- Goal: retain the current displayed speed record of 46-47 while reducing the
  secondary size from 215 commands to 211.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 36 - Seek and Destroy 2 (speed).txt>)
  and make four counted reductions: remove `mem1 = set myitem` immediately after the
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
- Same-world emulator A/B evidence: at winning seed 23, incumbent and the
  size-211 candidate both complete in exactly 2,987 frames with 212 item actions
  and modelled speed 48.  Deleting a nearby `comment 1` is runtime-neutral but
  does not reduce editor size because comment commands are free.
- Expected editor size: **211**; expected displayed speed: **46-47**.
- Suggested live test: run the incumbent and candidate on fresh worlds until
  each completes, then confirm the candidate's editor size and displayed speed.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 37 - Dangerous Spreadsheeting - ordered-set speed tie-break at size 243

- **Paste-ready program:** [SolutionsToTry/Year 37 - Dangerous Spreadsheeting - ordered-set speed tie-break at size 243.txt](<SolutionsToTry/Year 37 - Dangerous Spreadsheeting - ordered-set speed tie-break at size 243.txt>)
- Goal: retain the current displayed speed record of 10-11 while reducing the
  secondary size from 246 commands to 243.
- Exact edits: in the late state that currently does `mem1 = set c`, tests
  `mem1 != datacube`, and conditionally does `mem1 = set e`, replace the whole
  four-line block with the single ordered assignment `mem1 = set c,e`.  In the
  earlier empty-true block, replace `if mem1 == 0: ... else: mem2 = set mem1`
  with `if mem1 != 0: mem2 = set mem1; endif`.
- Why it should be safe: the old block retains the center item when numeric and
  otherwise falls back to the east item.  Ordered multi-target `set c,e` makes
  the same first-success choice, and this state only reads the resulting value
  numerically; it never uses the saved location as an action target.  The
  inverted predicate has exactly the same two cases while removing the empty
  branch and its counted `else`.
- Deterministic emulator A/B evidence: candidate and incumbent both win in
  exactly 747 frames with modelled speed 12.  The candidate uses 68 modelled
  item actions versus 70 and canonical counting confirms 243 versus 246.
- Timing caveat: the fusion contracts the fallback path, so live choreography
  and counter timing still need an A/B run despite the exact emulator result.
- Expected editor size: **243**; expected displayed speed: **10-11**.
- Suggested live test: run the current speed program as a control, then the
  candidate; capture both completion panels and candidate editor size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 07 - Collation Station - speed tie-break at size 11

- **Paste-ready program:** [SolutionsToTry/Year 07 - Collation Station - speed tie-break at size 11.txt](<SolutionsToTry/Year 07 - Collation Station - speed tie-break at size 11.txt>)
- Goal: retain the current displayed speed record of 3 while reducing the
  secondary size from 12 commands to 11.
- Exact edit: replace the incumbent's `if w == datacube` / `else` pickup branch
  with `if w != datacube: pickup n,c,s; endif`, then one shared `step s`, then
  `if myitem == nothing: pickup s; endif`; leave the opening three south steps,
  final two south steps, and `drop` unchanged.
- Why it should work: only the x6 worker sees a west cube and skips the first
  pickup.  After the shared step it alone is empty and takes its south cube;
  the other six workers already hold theirs and skip the second pickup.  No
  failed item action is introduced.
- Deterministic emulator A/B evidence: candidate and incumbent both win with
  modelled speed 4 and 21 item actions; the candidate takes 195 frames versus
  194.  Canonical sizes are 11 and 12.
- Expected editor size: **11**; expected displayed speed: **3**.
- Suggested live test: one incumbent/candidate A/B should suffice; capture the
  completion panel and editor size.  All commands are editor-constructible.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

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

### [ ] Year 15 - Shred Lines - speed tie-break at size 42 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - speed tie-break at size 42.txt](<SolutionsToTry/Year 15 - Shred Lines - speed tie-break at size 42.txt>)
- Goal: retain the current displayed speed record of 11 while reducing the
  secondary size from 46 commands to 42.
- Source: [n05ucc4u and abfipes12's cross-block program](https://github.com/abfipes12/7-Billion-Humans-Solutions/blob/6663ed04d735d92525f6ee1fbb9263461889faa2/WithGliches/Speed/Year%2015%20-%20Shred%20Lines).
  Preserve both credits.
- Why it is smaller: label `a` moves inside the `else` immediately before the
  shared south/give/north/pickup tail, and the final `jump a` reuses that tail
  instead of duplicating it after the branch.
- Deterministic emulator A/B evidence: candidate and incumbent are exactly
  identical through the win: 622 frames, modelled speed 10, and 35 item
  actions.  Canonical sizes are 42 and 46.
- Construction caveat: the jump crosses into an `else` block, so this is a
  clipboard/glitch solution and must retain the marker.
- Suggested live test: one deterministic A/B should suffice; capture the
  completion panel and editor size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
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

### [ ] Year 42 - Important Email Organization - speed tie-break at size 143

- **Paste-ready program:** [SolutionsToTry/Year 42 - Important Email Organization - speed tie-break at size 143.txt](<SolutionsToTry/Year 42 - Important Email Organization - speed tie-break at size 143.txt>)
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
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 25 - My First Shredding Memory - guarded-loop speed candidate at size 6

- **Paste-ready program:** [SolutionsToTry/Year 25 - My First Shredding Memory - guarded-loop speed candidate at size 6.txt](<SolutionsToTry/Year 25 - My First Shredding Memory - guarded-loop speed candidate at size 6.txt>)
- Goal: retain or improve the current displayed speed record of 129 while
  reducing the secondary size from 9 commands to **6**.
- Program:

```text
mem1 = nearest shredder
a:
mem2 = nearest datacube
pickup mem2
if myitem == datacube:
    giveto mem1
endif
jump a
```

- Why it should be safe: a successful pickup feeds the same remembered
  shredder.  An empty pickup-race loser skips the give and retries; a worker
  still holding a cube retries the guarded give.  The persistent loop removes
  the incumbent's outer availability branch and terminal cleanup without
  changing cube or machine selection.
- Deterministic emulator A/B evidence: candidate and incumbent both win.  The
  candidate completes in 9,849 frames/modelled speed 158 with 152 item actions,
  versus 32,839/526/157 for the incumbent.  The model's absolute timing differs
  sharply from the known live score, but this paired improvement is much larger
  than the previous size-8 candidate's one-frame-equivalent gain.
- Expected editor size: **6**; expected displayed speed: at most **129**.
- Suggested live test: run the incumbent once as a control and the candidate
  once, capturing both completion panels and the candidate's editor size.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 29 - Biometric Access - speed tie-break at size 182

- **Paste-ready program:** [SolutionsToTry/Year 29 - Biometric Access - speed tie-break at size 182.txt](<SolutionsToTry/Year 29 - Biometric Access - speed tie-break at size 182.txt>)
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

### [ ] Year 66 - Decimal Counter - padding reduction at size 239 (safe fallback 240)

- **Paste-ready program:** [SolutionsToTry/Year 66 - Decimal Counter - padding reduction at size 239.txt](<SolutionsToTry/Year 66 - Decimal Counter - padding reduction at size 239.txt>)
- **Fallback (size 240):** [SolutionsToTry/Year 66 - Decimal Counter - safe fallback size 240.txt](<SolutionsToTry/Year 66 - Decimal Counter - safe fallback size 240.txt>)
- Goal: retain the current displayed speed record of 24 while reducing the
  secondary size from 254 commands to 239, with a frame-identical size-240
  fallback if the live score is sensitive to the final eight-frame shift.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 66 - Decimal Counter (speed).txt>)
  and delete three data-dead store blocks: the six consecutive
  `mem1 = set myitem` commands immediately after `listenfor ready`; the four
  such stores after `write 2; drop; step e`; and the three stores after
  `pickup w`.  Also delete one of the two consecutive `tell everyone hi`
  commands after `step w; step ne`.  This is the canonical size-240 fallback.
- Aggressive size-239 edit: additionally delete the downstream
  `mem1 = set c` after the retained duplicate-tell area and its following
  `step s; step n`; the value is never read anywhere in the program.
- Deterministic emulator A/B evidence: the size-240 fallback and incumbent both
  win in exactly 1,552 frames at modelled speed 25; canonical sizes are 240 and
  254.  Size 239 wins in 1,560 frames, still modelled speed 25.
- Boundary checks: deleting a different earlier singleton slows to 1,574
  frames/modelled speed 26, and deleting even one store from the later
  14-store cadence block fails.  Those reductions are not proposed.
- Expected editor size: **239** if it retains live speed 24; otherwise use the
  proven **240** fallback.
- Suggested live test: try size 239 first.  If its displayed speed regresses,
  restore that final singleton and capture the size-240 completion panel.
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

### [ ] Year 68 - Goodbye, Humans! - bypassed-wrapper speed tie-break at size 164

- **Paste-ready program:** [SolutionsToTry/Year 68 - Goodbye, Humans! - bypassed-wrapper speed tie-break at size 164.txt](<SolutionsToTry/Year 68 - Goodbye, Humans! - bypassed-wrapper speed tie-break at size 164.txt>)
- Goal: retain the current displayed speed record of 16 while reducing the
  secondary size from 172 commands to **164** without changing the executed
  instruction sequence.
- Exact structural edits: start from
  [the current speed program](<Solutions99+/Year 68 - Goodbye Humans (speed).txt>)
  and:
  - replace the opening three nested `foreachdir` wrappers with the straight
    `jump a; a: jump b; b:` path;
  - after `jump g`, remove the four nested `foreachdir` wrappers (`sw`, `s`,
    `se`, `e`) and their `endfor`s while retaining and dedenting the `g:` body;
  - after `jump m`, remove the `foreachdir n` wrapper and its `endfor` while
    retaining and dedenting the `m:` body; and
  - delete unreachable `comment 0` after the unconditional `jump ag`.
- Static reachability proof: entry jumps directly through `a` to `b`; every
  `g` path exits through `h/i/j/k/l` before its removed endfors; every `m` path
  exits through `n/o/p`; and no path enters `comment 0`.  The eight charged
  wrapper headers are therefore outside every executed path; `comment 0` is
  also unreachable but editor-size-free.
- Optional cadence ladder: after confirming size 164, delete one of the two
  consecutive top-level `tell everyone hi` commands for size 163, then both for
  size 162.  Stop if displayed speed regresses.
- Entry method: paste/reimport the dedented program; deleting a wrapper in the
  visual editor may also delete its retained body.
- Emulator caveat: the current model fails both the published incumbent and
  this candidate class, so only the live game can referee the pasted control
  flow and displayed timing.
- Expected editor size: **164**; expected displayed speed: **16**.
- Suggested live test: one incumbent loading/control run, then size 164.  Only
  after that succeeds should you try the optional size-163/162 tell ladder.
- Live-speed caution: frame-based evidence no longer establishes
  displayed speed (Years 39/40 regressed 36→41 live).  Run the
  incumbent as control first; discard on regression.
- Result: _not yet tested locally in the game_.

### [ ] Year 44 - Unique Fashion Party - speed 1 at size 8

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - speed 1 at size 8.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - speed 1 at size 8.txt>)
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
- Result: _not yet tested in the game_.

### [ ] Year 15 - Shred Lines - stochastic size 4

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - stochastic size 4.txt](<SolutionsToTry/Year 15 - Shred Lines - stochastic size 4.txt>)
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
