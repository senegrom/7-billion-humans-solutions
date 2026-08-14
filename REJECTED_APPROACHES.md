# Rejected and Superseded Approaches

These experiments are kept so failed ideas are not rediscovered and mistaken
for records.  Candidates that still need live-game verification remain in
[SOLUTIONS_TO_TRY.md](SOLUTIONS_TO_TRY.md).

- Y24 size 3: emulator false positive; real handoff semantics do not sustain it.
- Y15 public size 6: 96/100 capped emulator wins, but superseded in both size
  and reliability evidence by the retained novel size-5 candidate.
- Y15 size-5 give-first ordering: 300/300, but average and worst-case timing
  were slightly worse than the retained pickup-first ordering on the same
  seeds (191.4 vs 190.9 average; 387 vs 378 worst).
- Y15 symmetric size-5 gate `(shredder and holding) or north-cube`: 300/300,
  but much slower than the retained empty-worker gate (265.5 average, 727
  worst).  This exhausts the only other non-dominated one-IF predicate family.
- Y16 size-4 random `step s,e` route: a finite three-cube path exists, but two
  buffered witness batches timed out without yielding a result and the static
  route probability is too low for a practical live test.  Retain as theory,
  not as a queue entry.
- Y10 random-walk size 2: 0/100 capped emulator wins; any winning path is too
  rare to justify live-game queue space without stronger evidence.
- Y10 speed size-32 branch fusion: it still wins, but takes 867 model frames
  versus the incumbent's 790 and is therefore not a speed-record tie-break.
- Y07 deterministic size-9 pickup linearization: it wins, but adds a 93-frame
  invalid-pickup bubble and slows modelled speed from 4 to 5.  The retained
  error-free size-11 candidate is the correct secondary-size lead.
- Y12 speed size-16 random final-direction fusion: 0/100 while the incumbent
  was 100/100 in 197 frames; endpoint motion changes later branch entrants.
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
- Y31 diagonal size-4 families: `takefrom` failed 0/10 because workers steal
  sheets from adjacent hands and break the parity invariant.  Replacing it with
  `pickup` preserves parity, but a corrected printer/floor-relay model was
  0/1,000 because workers repeatedly remove already-filled goal cubes.
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
- Y41 random-exit size 6: a finite all-west-while-carrying then north-to-hole
  route exists, but even the relaxed geometry estimate is only about 1e-88;
  there is no practical witness search or live test.
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
- Y64 speed size-99 deletion of one initial `mem1 = set myitem` cadence pad:
  0/1 at the real cutoff while the untouched size-100 incumbent won in 561
  frames.  Even one apparently dead store is phase-critical on this level.
- Y65 size 13 rewrite: 0/52.
- Y66 further padding cuts: deleting one store from the later 14-store block
  fails; deleting the earlier singleton after its first shuffle wins but slows
  from 1,552 to 1,574 frames/modelled speed 26.  Size 240 is the exact-timing
  fallback and size 239 in the live-game queue is the only retained aggressive
  boundary.
- Y68 unconditional size 5: 0/20; removing the side-hole guard did not produce
  an observable low-percent win.
