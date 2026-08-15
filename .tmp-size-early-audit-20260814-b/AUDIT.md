# Years 02-25 optimization audit

Checkout: root `master` at `412d9d1`; nested emulator `9521722`.
All emulator evidence uses the exact extracted level and the 87,500-frame
(1,400-second) hard cutoff.  No tracked file was changed.

## Candidate evidence

### Year 15 event-gated size 5 (root-owned novel lead)

Path: `.codex_root_candidates/y15_event_gated_size5.txt`

```text
a:
step n,s
if n == datacube or
 s == shredder:
    pickup n
    giveto s
endif
jump a
```

Canonical game size 5 versus current size 8.  Paste-only.  On emulator commit
9521722: 300/300 wins, average 290.3 seconds, range 85-756 seconds, maximum
47,214 frames.  The directional failed-pickup/give bubbles are modeled, but an
in-game paste and result remains the publication gate.  Active
`SOLUTIONS_TO_TRY` candidate.

### Year 15 item-state size 7 (novel deterministic fallback)

Path: `.tmp-size-early-audit-20260814-b/y15_item_state_size7.txt`

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

Canonical game size 7 (`else` counts) versus current size 8.  Paste-only
because `myitem` is not exposed this early.  On emulator commit 9521722: 1/1
deterministic win, 35 seconds, 2,167 frames.  Dominated by the size-5 lead but
worth retaining as a deterministic fallback until the real game accepts the
size-5 program.

### Year 15 public size 6 (+50% fallback)

Path: `.tmp-size-early-audit-20260814-b/y15_external_size6_50plus.txt`.
Normalized-exact copy of public source
`.codex_external_abfipes/WithGliches/Size/+50/Year 15 - Shred Lines`, credited
to `@abfipes12`.  Its header reports 9 failures in 25 trials, i.e. 16/25 or
64% success.  Canonical size 6 versus current +99% size 8.

Corrected emulator evidence: 96/100 wins; winning average 888.6 seconds,
range 432-1,346 seconds, 26,991-84,066 frames.  Four failures reached the hard
cutoff.  It belongs in `SOLUTIONS_TO_TRY` as a credited +50% fallback, but is
dominated if the novel size-5 +99% candidate is confirmed in game.

### Year 22 public size 4 (disputed/rejected +50% classification)

Path: `.tmp-size-early-audit-20260814-b/y22_external_size4_50plus.txt`.
Normalized-exact copy of public source
`.codex_external_abfipes/WithGliches/Size/+50/Year 22 - Number Royale`, credited
to `@abfipes12`.  Canonical size 4 versus current +99% size 5.

The public header says `failed 86 out of 150 => 57.3% successful`, but 86
failures imply only 64/150 = 42.7% success.  Corrected emulator evidence was
48/100 wins; winning average 402.0 seconds, range 110-920 seconds,
6,847-57,464 frames.  Fifty-two failures reached the hard cutoff.  It does not
currently qualify for `Solutions50+`; retain only as a rejected/disputed note,
not an active `SOLUTIONS_TO_TRY` entry.

### Year 24 unconditional size 3 (rejected)

Path: `.tmp-size-early-audit-20260814-b/y24_unconditional_transfer_size3.txt`.
The emulator lead was already disproved by real-game handoff behavior.  The
`nowalk` rule and `shredded_count 50` goal are implemented, but the relevant
worker handoff was a fidelity false positive.  Keep it in the rejected section
only; do not retest or promote it.

### Missing credited LowPercent imports

These public real-game programs are below 50% but each is strictly smaller
than the current +99% size record, with no +50% size row in between.  They
therefore qualify under `CONTRIBUTING.md`.  Their exact external bodies were
added as untracked solution files (README intentionally unchanged); they are
credited imports, not new discoveries:

* Y05 size 4, speed 21-22 seconds, 12/100 wins.  Exact scratch copy
  `y05_external_lowpercent_size4.txt`; public source
  `.codex_external_abfipes/Other/low%/Year 05 - An Important Decision`,
  credited to `@H-J-Granger` and `@abfipes12`.  The source explicitly says
  `WithGliches`, so the README row needs the paste marker.
* Y06 size 7, speed 6 seconds, 25% success (source does not state a trial
  count).  Exact scratch copy `y06_external_lowpercent_size7.txt`; public
  source `.codex_external_abfipes/Other/low%/Year 06 - Little Exterminator 1`,
  credited to `@H-J-Granger`.  The source explicitly says `WithGliches`, so
  the README row needs the paste marker.
* Y13 size 6, speed about 505 seconds, 71/175 wins (40.57%).  Exact scratch
  copy `y13_external_lowpercent_size6.txt`; public source
  `.codex_external_abfipes/Other/+35%/Year 13 - Injection Sites 2`, credited
  to `@H-J-Granger`.  Every command is present in the extracted Y13 palette
  and the source does not claim a paste glitch.

All recorded winning speeds are below the real 1,400-second cutoff.  These
are import candidates with existing public game evidence, not emulator-only
leads; no extra emulator run was needed.

## Complete level coverage

| Year | Current records (size/speed) | Audit result |
|---|---|---|
| 02 | +99 both 3/1 | Optimal: step, pickup and drop are each required. |
| 03 | +99 both 5/2 | Optimal for the extracted palette; the tempting size-4 loop needs `jump`, which is unavailable at this level. |
| 04 | +99 both 3/6 | Optimal loop: pickup plus repeated step/jump; community alternatives are not better tie-breaks. |
| 05 | +99 both 5/2 | No +50 lead. Import the missing public LowPercent size 4 at 12%. |
| 06 | +99 both 8/2 | Seven safe moves plus pickup are required for deterministic success. Import the missing public LowPercent size 7 at 25%. |
| 07 | +99 size 4/15; speed 12/3 | No smaller or faster lead; public snapshot matches current records. |
| 08 | no playable extracted level | No solution record to optimize. |
| 09 | +99 size 5/5; speed 15/3 | Public 15/3 speed tie-break is already imported; no lower structural lead. |
| 10 | +99 size 5/~155; speed 33/12 | Current programs are the strongest public paste/glitch records; no new lead. |
| 11 | +99 size 5/10; speed 16/5 | Current public bests; no new lead. |
| 12 | +99 size 5/118-157; speed 19/3 | Current public paste/glitch bests; a four-command random unzip has far below 50% joint placement probability. |
| 13 | +99 size 7/15; speed 20/5 | Import the missing public LowPercent size 6 at 40.57% (71/175); it remains below +50. |
| 14 | +99 size 4/4; speed 5/2 | Four required item/movement actions and two-second speed are lower bounds. |
| 15 | +99 size 8/23; speed 46/11 | Novel size 5 is the main lead; deterministic size 7 and public +50 size 6 are fallbacks. No speed improvement. |
| 16 | +99 size 6/26; speed 14/7 | Public 6/26 tie-break is already imported; no new lead. |
| 17 | +99 both 1/2 | Single pickup is the command lower bound. |
| 18 | +99 size 5/23; speed 13/6 | Public 5/23 tie-break already imported; unique-shredder rule is enforced. No new lead. |
| 19 | +99 both 3/26 | Take/give/jump perpetual loop is the size lower bound; no speed lead. |
| 20 | +99 size 9/11; speed 38/4 | Public 38/4 speed tie-break already imported; two variable-distance loops make size 9 structurally tight. |
| 21 | +99 size 5/21-26; speed 41/16-22; +50 speed 53/14-17 | Current files match the strongest public records; no new lead. |
| 22 | +99 size 5/8-10; speed 20/5-6; +50 speed 7/2 | Public size-4 file fails the +50 audit (48/100 and contradictory header); no qualifying new lead. |
| 23 | +99 size 6/23-25; speed 9/15-17 | Current imported public programs are strongest. Prior random size-4 inversion did not complete under the cap. |
| 24 | +99 size 5/70; speed 43/41 | No real-game lead. Size 3 is a known handoff-fidelity false positive; speed path is emulator-unfaithful/exhausted. |
| 25 | +99 size 5/139; speed 9/129 | Size 5 is the nearest/pickup/give loop lower bound. No reliable speed lead; nearest-motion timing remains a fidelity caveat. |

Every extracted goal for Years 02-25 is dispatched by emulator commit 9521722.
The early special rules used here (`alive_all`, `unique_shredder_use`, and
`nowalk`) are also enforced.  Goal coverage alone does not remove the Y24
handoff or Y25 movement/timing fidelity caveats described above.
