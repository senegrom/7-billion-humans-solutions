from __future__ import annotations

import random
import re
import sys
from pathlib import Path


LEVEL = Path(__file__).resolve().parents[1] / "tools/extract/out/levels/31_checkerboard_organization.lvl"
DIAGS = ((-1, -1), (-1, 1), (1, -1), (1, 1))


def load() -> tuple[int, int, set[tuple[int, int]], set[tuple[int, int]], tuple[int, int], list[tuple[int, int]], set[tuple[int, int]]]:
    width = height = 0
    walls: set[tuple[int, int]] = set()
    holes: set[tuple[int, int]] = set()
    printer = (-1, -1)
    workers: list[tuple[int, int]] = []
    goals: set[tuple[int, int]] = set()
    cubes: set[tuple[int, int]] = set()
    for line in LEVEL.read_text().splitlines():
        fields = line.split()
        if fields[:1] == ["dim"]:
            width, height = map(int, fields[1:3])
        elif fields[:2] == ["ent", "wall"]:
            walls.add(tuple(map(int, fields[2:4])))
        elif fields[:2] == ["ent", "hole"]:
            holes.add(tuple(map(int, fields[2:4])))
        elif fields[:2] == ["ent", "printer"]:
            printer = tuple(map(int, fields[2:4]))
        elif fields[:2] == ["ent", "worker"]:
            workers.append(tuple(map(int, fields[2:4])))
        elif fields[:2] == ["ent", "goal"]:
            goals.add(tuple(map(int, fields[2:4])))
        elif fields[:2] == ["ent", "cube"]:
            cubes.add(tuple(map(int, fields[2:4])))
    productive = [position for position in workers if (sum(position) - sum(printer)) % 2 == 0]
    return width, height, walls, holes, printer, productive, cubes


def main() -> None:
    trials = int(sys.argv[1]) if len(sys.argv) > 1 else 100_000
    cap = int(sys.argv[2]) if len(sys.argv) > 2 else 2_000
    width, height, walls, holes, printer, starts, initial_cubes = load()
    goals = {
        tuple(map(int, match.groups()))
        for line in LEVEL.read_text().splitlines()
        if (match := re.match(r"ent goal (\d+) (\d+)", line))
    }
    wins = 0
    win_turns: list[int] = []
    for seed in range(1, trials + 1):
        rng = random.Random(seed)
        workers = [[x, y, False, True] for x, y in starts]
        cubes = set(initial_cubes)
        for turn in range(1, cap + 1):
            order = list(range(len(workers)))
            rng.shuffle(order)
            printer_used = False
            for index in order:
                worker = workers[index]
                x, y, holding, alive = worker
                if not alive:
                    continue
                waiting_for_printer = False
                if not holding:
                    for dx, dy in DIAGS:  # exact pickup order: nw, sw, ne, se
                        target = x + dx, y + dy
                        if target in cubes:
                            cubes.remove(target)
                            holding = True
                            break
                        if target == printer:
                            if printer_used:
                                waiting_for_printer = True
                            else:
                                holding = True
                                printer_used = True
                            break
                if waiting_for_printer:
                    worker[:] = [x, y, holding, True]
                    continue
                options: list[tuple[int, int]] = []
                for dx, dy in DIAGS:
                    target = x + dx, y + dy
                    if not (0 <= target[0] < width and 0 <= target[1] < height):
                        continue
                    if target in walls or target == printer:
                        continue
                    options.append(target)
                if options:
                    x, y = rng.choice(options)
                if (x, y) in holes:
                    worker[:] = [x, y, False, False]
                    continue
                if holding and (x, y) not in cubes:
                    cubes.add((x, y))
                    holding = False
                worker[:] = [x, y, holding, True]
            if goals <= cubes:
                wins += 1
                win_turns.append(turn)
                break
            if not any(worker[3] for worker in workers):
                break
    rate = wins / trials
    print(f"trials={trials} wins={wins} rate={rate:.8f}")
    if win_turns:
        win_turns.sort()
        print(
            f"turns min={win_turns[0]} median={win_turns[len(win_turns)//2]} "
            f"max={win_turns[-1]}"
        )
        for percentile in (90, 95, 99):
            index = min(
                len(win_turns) - 1,
                (len(win_turns) * percentile + 99) // 100 - 1,
            )
            print(f"p{percentile}={win_turns[index]}")


if __name__ == "__main__":
    main()
