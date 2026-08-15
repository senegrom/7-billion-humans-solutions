from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
for path in sorted((ROOT / "tools/extract/out/levels").glob("*.lvl")):
    m = re.match(r"(\d\d)_", path.name)
    if not m or not 48 <= int(m.group(1)) <= 68:
        continue
    lines = path.read_text(encoding="utf-8").splitlines()
    width = height = 0
    rows: list[list[str]] = []
    cubes: list[str] = []
    workers: list[tuple[int, int]] = []
    for line in lines:
        if line.startswith("dim "):
            width, height = map(int, line.split()[1:])
            rows = [["." for _ in range(width)] for _ in range(height)]
        elif line.startswith("row "):
            y = next((i for i, row in enumerate(rows) if all(c == "." for c in row)), 0)
            rows[y] = list(line[4:].ljust(width, ".")[:width])
        elif line.startswith("ent "):
            parts = line.split()
            kind, x, y = parts[1], int(parts[2]), int(parts[3])
            ch = {"wall":"#", "door":"D", "hole":"O", "shredder":"S", "printer":"P",
                  "switch":"X", "button":"B", "worker":"@", "cube":"c", "goal":"G",
                  "goalpad":"G"}.get(kind, "?")
            if kind == "cube":
                cubes.append(f"({x},{y})={parts[4]}")
            if kind == "worker":
                workers.append((x, y))
            if 0 <= x < width and 0 <= y < height:
                rows[y][x] = ch if rows[y][x] == "." else rows[y][x] + ch
    print(f"===== {path.name} =====")
    for line in lines:
        if line.startswith(("name ", "commands ", "goal ", "rule ", "randmax ")):
            print(line)
    print("workers", workers)
    print("cubes", " ".join(cubes))
    for y, row in enumerate(rows):
        print(f"{y:02} " + "".join(cell[-1] for cell in row))
