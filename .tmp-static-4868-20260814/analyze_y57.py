from collections import Counter
from pathlib import Path

cubes: list[tuple[int, int]] = []
for line in Path("tools/extract/out/levels/57_neighborly_sweeper.lvl").read_text().splitlines():
    fields = line.split()
    if len(fields) >= 4 and fields[:2] == ["ent", "cube"]:
        cubes.append((int(fields[2]), int(fields[3])))

cube_set = set(cubes)
degree = {
    cube: sum(
        (cube[0] + dx, cube[1] + dy) in cube_set
        for dx in (-1, 0, 1)
        for dy in (-1, 0, 1)
        if dx or dy
    )
    for cube in cubes
}
print("cubes", len(cubes), "degree counts", sorted(Counter(degree.values()).items()))
for value, count in Counter(degree.values()).most_common():
    print(value, count, sorted(cube for cube in cubes if degree[cube] != value))
