from collections import Counter, deque
from pathlib import Path

level = Path("tools/extract/out/levels/61_lazy_pathways.lvl")
width, height = 17, 13
walls: set[tuple[int, int]] = set()
cubes: list[tuple[int, int, int]] = []
door: tuple[int, int] | None = None

for line in level.read_text().splitlines():
    fields = line.split()
    if len(fields) < 4 or fields[0] != "ent":
        continue
    kind, x, y = fields[1], int(fields[2]), int(fields[3])
    if kind == "wall":
        walls.add((x, y))
    elif kind == "cube":
        cubes.append((x, y, int(fields[4])))
    elif kind == "door":
        door = (x, y)

assert door is not None
corners = [door, (door[0] - 1, door[1]), (door[0], door[1] - 1), (door[0] - 1, door[1] - 1)]
corner_distances: list[dict[tuple[int, int], int]] = []
for start in corners:
    distances = {start: 0}
    queue = deque([start])
    while queue:
        x, y = queue.popleft()
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                nxt = (x + dx, y + dy)
                if not (dx or dy):
                    continue
                if not (0 <= nxt[0] < width and 0 <= nxt[1] < height):
                    continue
                if nxt in walls or nxt in corners or nxt in distances:
                    continue
                distances[nxt] = distances[(x, y)] + 1
                queue.append(nxt)
    corner_distances.append(distances)


def target(position: tuple[int, int]) -> int:
    corner = min(
        range(4),
        key=lambda i: (corners[i][0] - position[0]) ** 2 + (corners[i][1] - position[1]) ** 2,
    )
    return corner_distances[corner].get(position, -1)


all_shells = Counter(target((x, y)) for x, y, _ in cubes)
wrong_shells = Counter(target((x, y)) for x, y, value in cubes if value != target((x, y)))
print("cubes", len(cubes))
print("initial correct", len(cubes) - sum(wrong_shells.values()))
print("all shells", sorted(all_shells.items()))
print("initial-wrong shells", sorted(wrong_shells.items()))
