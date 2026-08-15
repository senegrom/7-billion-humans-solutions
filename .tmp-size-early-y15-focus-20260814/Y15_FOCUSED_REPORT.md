# Year 15 focused optimization report

Level: `tools/extract/out/levels/15_shred_lines.lvl` (`goal shred_all`)

Emulator: nested commit `9521722`; every reported batch used `EMU_CAP=87500`.
All candidate files and this report are untracked scratch artifacts.

## Best candidate

Path: `y15_size5_empty_gate.txt`

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

Canonical size: **5**.  Conditions fold left-to-right, so its predicate is
exactly `(north-is-cube AND hands-empty) OR south-is-shredder`.

Validation: **300/300 WIN**, average 190.9 game seconds, range 85..378;
frames 5,310..23,590, average 11,900.3; no 87,500-frame cap hits.

Comparison: the previous event gate `(north-is-cube OR south-is-shredder)`
was 300/300 but averaged 290.3 seconds, ranged 85..756, and reached 47,214
frames.  The new hand-state clause prevents a holder wandering below another
uncollected cube from paying a full-hands pickup failure plus a failed give.

This is still a real-game candidate, not a confirmed record: `myitem` is a
paste-only/future sensor on Year 15 and must be accepted and scored by the owned
game.  All commands themselves are in the extracted Year 15 palette and the
emulator implements this level's exact `shred_all` goal.

## Size lower bounds

Size <=3 cannot solve the level:

1. A loop is necessary because each fixed-column worker must process three
   cubes, and programs do not wrap, so `jump` is required.
2. A movement command must reach cubes to the north and return to shredders to
   the south.
3. `pickup` and `giveto` are distinct required side effects.

Thus `step`, `pickup`, `giveto`, and `jump` give a hard lower bound of size 4.
A conditionally gated loop needs one additional `if`, so the natural gated
lower bound is size 5.

The canonical size-4 loop

```text
a:
step n,s
pickup n
giveto s
jump a
```

won 99/100 capped trials: winning runs averaged 655.0 seconds, ranged
320..1183, and used 19,940..73,900 frames; one run hit the hard cap.  Adding
current to the pickup mask (`pickup c,n`) produced metric-identical 99/100
results.  Current is geometrically redundant: an empty worker encounters the
square immediately south of the next cube before it can stand on that cube.
The give-first size-4 ordering remains untested, but it cannot eliminate the
two speculative item errors paid by ordinary blank random-walk loops.

## Size-5 predicate lower bound

Let `A` mean north is a cube, `S` south is a shredder, `E` empty hands, and
`H` holding a cube.  The exact state gate wanted is `A&E OR S&H`.  On event
tiles, A/S are exclusive and E/H are complements, so this is XNOR.  It cannot
be expressed by one unparenthesized left-fold condition.  In particular,
`A and E or S and H` evaluates as `(((A&E)|S)&H)` and collapses to `S&H`,
never picking up.  Reversing the pairs collapses to `A&E`, never shredding.

Any safe one-condition superset that admits both required states must admit at
least one cross-state.  The two minimal choices are:

* Tested best: `(A&E) OR S`, which also fires for empty-at-shredder but excludes
  held-at-cube.
* Symmetric gate: `(S&H) OR A`, expressible as
  `s == shredder and myitem == datacube or n == datacube`; it excludes
  empty-at-shredder but still fires for held-at-cube.

Both cross-states are genuinely reachable.  The symmetric gate's pickup-first
form was tested for 300 capped trials: 300/300 wins, average 265.5 seconds,
range 95..727, frames 5,902..45,382, average 16,562.6.  It is decisively worse
than the tested best gate (by 4,662.3 average frames and 21,792 maximum frames),
so its give-first ordering was not tested under the predeclared stop rule.
Predicates accepting additional non-event states are dominated; the exact full
gate requires a second `if` and canonical size at least 6.

## Action-order comparison

Path: `y15_size5_empty_gate_give_first.txt`

This reverses the tested body's two actions to `giveto s; pickup n`.  It also
won **300/300**, but averaged 191.4 seconds, ranged 78..387, and used
4,854..24,170 frames (average 11,933.9).  Against pickup-first on the identical
seed set 1..300, it improved the best case by 456 frames but worsened the
average by 33.6 and the worst case by 580.  It is rejected as a record; global
worker/RNG interleaving outweighed its favorable local delivery ordering.

No further emulator runs were launched after the symmetric pickup-first batch.
