# Per-level command palettes

Which commands each level's editor offers, in the game's own order,
plus the level that introduces each command.  (Observed from the
game; the editor shows exactly this palette at the bottom of each
level's program panel.)

| # | Year | Level | Commands | Introduces |
|--:|-----:|-------|----------|------------|
| 0 | 2 | Welcome, New Employees | pickup drop step |  |
| 1 | 3 | Transport Squad | step pickup drop |  |
| 2 | 4 | Long Distance Delivery | step pickup jump | jump |
| 3 | 5 | An Important Decision | if pickup drop step jump | if |
| 4 | 6 | Little Exterminator 1 | step pickup jump if |  |
| 5 | 7 | Collation Station | step jump drop if pickup |  |
| 6 | 9 | Dynamic Angles | step jump pickup drop if |  |
| 7 | 10 | Emergency Escapades | step jump if |  |
| 8 | 12 | Unzip | step pickup drop jump if |  |
| 9 | 11 | Injection Sites 1 | step pickup drop jump if |  |
| 10 | 13 | Injection Sites 2 | step pickup drop jump if |  |
| 11 | 14 | Intro to Shredding | pickup step drop giveto jump if | giveto |
| 12 | 15 | Shred Lines | step jump if pickup giveto end | end |
| 13 | 16 | Little Exterminator 2 | if pickup drop step jump giveto end |  |
| 14 | 17 | Content Creators | pickup |  |
| 15 | 19 | Content Creators Bug Fix | step takefrom drop jump if pickup giveto | takefrom |
| 16 | 18 | Uniquely Disposed | step pickup if drop giveto jump end |  |
| 17 | 20 | Reverse Line | step jump pickup if drop end |  |
| 18 | 21 | Big Data | step pickup drop jump if giveto takefrom end |  |
| 19 | 22 | Number Royale | step jump pickup if end |  |
| 20 | 23 | Sorting Hall | step pickup drop jump if end |  |
| 21 | 24 | Budget Brigade 1 | takefrom giveto if jump |  |
| 22 | 26 | Budget Brigade 2 | jump takefrom giveto if |  |
| 23 | 25 | My First Shredding Memory | nearest giveto jump pickup end if | nearest |
| 24 | 28 | Neural Pathways | nearest giveto pickup jump |  |
| 25 | 29 | Biometric Access | step takefrom pickup if drop giveto jump end nearest |  |
| 26 | 30 | Fill the Floor | step nearest jump takefrom pickup drop if end |  |
| 27 | 31 | Checkerboard Organization | step nearest takefrom jump pickup drop if end |  |
| 28 | 32 | Creative Writhing | step pickup drop jump if nearest write end | write |
| 29 | 33 | Data Backup Day | step nearest set jump pickup drop if write end | set |
| 30 | 34 | Seek and Destroy 1 | step nearest jump giveto pickup drop if set end |  |
| 31 | 36 | Seek and Destroy 2 | step nearest jump giveto pickup drop if set end |  |
| 32 | 38 | Seek and Destroy 3 | step nearest jump giveto takefrom pickup drop if set end |  |
| 33 | 35 | Intro to Calc for Art Majors | set calc pickup drop write | calc |
| 34 | 37 | Dangerous Spreadsheeting | step pickup drop jump if set write calc end |  |
| 35 | 39 | Printing Etiquette 1 | step pickup drop jump if giveto takefrom nearest set write calc end |  |
| 36 | 40 | Printing Etiquette 2 | step pickup drop jump if giveto takefrom nearest set write calc end |  |
| 37 | 41 | Image Decrypter | if jump nearest set calc step pickup drop write |  |
| 38 | 42 | Important Email Organization | step pickup drop jump if giveto takefrom nearest set calc end |  |
| 39 | 43 | Multiplication Table | step pickup drop jump if giveto takefrom nearest set write calc end |  |
| 40 | 44 | Unique Fashion Party | step nearest jump pickup if end set calc |  |
| 41 | 46 | Compulsory Office Romance | tell listen if jump | tell, listen |
| 42 | 47 | Automated Pleasantries | tell listen if jump end |  |
| 43 | 48 | Community Training Day | step nearest jump takefrom giveto pickup drop if tell listen set end |  |
| 44 | 49 | Double Sided Destruction | step nearest jump giveto pickup drop if tell listen set end |  |
| 45 | 50 | Cubical Communication | step nearest jump takefrom giveto if tell listen set calc end |  |
| 46 | 51 | Identify Yourselves | pickup drop write jump if tell listen calc set end step |  |
| 47 | 52 | The Mode Code | step pickup drop jump if set write calc end tell listen nearest |  |
| 48 | 53 | 100 Cubes on the Floor | set calc step pickup drop write tell listen jump if end |  |
| 49 | 54 | Terrain Leveler | step pickup drop jump if set write calc end tell listen |  |
| 50 | 55 | Data Flowers | step set jump pickup drop if foreachdir write calc end | foreachdir |
| 51 | 56 | Local Maximums | step pickup drop jump if giveto nearest set write calc foreachdir end |  |
| 52 | 57 | Neighborly Sweeper | step pickup drop jump if set write calc tell listen foreachdir end |  |
| 53 | 58 | Good Neighbors | step pickup drop jump if set write calc foreachdir end nearest |  |
| 54 | 59 | Glory Hole | step set jump if foreachdir calc nearest end |  |
| 55 | 60 | Understaffed Sorting | step pickup drop jump if end giveto takefrom set tell listen foreachdir |  |
| 56 | 61 | Lazy Pathways | step nearest takefrom jump foreachdir set if drop calc end pickup write |  |
| 57 | 62 | The Sorting Floor | step pickup drop jump if set calc giveto tell listen nearest foreachdir |  |
| 58 | 63 | Defrag Disordered | step pickup drop jump if set nearest calc foreachdir end |  |
| 59 | 65 | Defrag Ordered | step pickup drop jump if set calc nearest foreachdir end |  |
| 60 | 64 | Binary Counter | step jump if tell listen calc set pickup drop end |  |
| 61 | 66 | Decimal Counter | step pickup drop jump if set write calc tell listen |  |
| 62 | 67 | Decimal Doubler | step pickup drop jump if set write calc tell listen |  |
| 63 | 68 | Goodbye, Humans! | step jump if set end tell listen foreachdir nearest |  |

## Condition-operand availability (editor)

The if-editor's operand choices are not per-level data; they follow
fixed rules:

- Directions and square tests are always offered.
- `nothing` is offered as a comparand wherever conditions exist.
- `num` (numeric literals) appears in condition and calc contexts.
- `everyone` exists only as a tell target.
- `mem1`-`mem4` appear once memory commands are introduced.
- **`myitem` unlocks as a condition subject at Number Royale
  (Year 22) and stays available afterwards.**  Before that level it
  cannot be typed -- but the game's paste importer accepts it on
  every level, which is how several published records on earlier
  levels exist (they carry the paste marker).

Practical rule for queue entries: a program using `myitem` in a
condition is editor-constructible only from Year 22 onward;
elsewhere mark it paste-only.
