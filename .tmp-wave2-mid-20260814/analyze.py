from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from check_readme import is_free, read_statements, solution_size


ROOT = Path(__file__).resolve().parents[1]
YEARS = set(range(26, 48))
REGISTER_RE = re.compile(r"\b(mem[1-4])\b")
ASSIGN_RE = re.compile(r"^(mem[1-4])\s*=\s*(.+)$")
CONTROL_RE = re.compile(r"^(?:if\b|else:?$|endif$|while\b|endwhile$|jump\b|end$|[A-Za-z_]\w*:$)")


def year(path: Path) -> int | None:
    match = re.match(r"Year (\d+)", path.name)
    return int(match.group(1)) if match else None


paths = sorted(
    path
    for folder in ("Solutions99+", "Solutions50+", "SolutionsLowPercent")
    for path in (ROOT / folder).glob("Year *.txt")
    if year(path) in YEARS
)

for path in paths:
    statements = read_statements(path)
    print(f"\n=== {path.relative_to(ROOT)} size={solution_size(path)} ===")

    # Immediate duplicates, including action timing pads.
    for index in range(len(statements) - 1):
        if statements[index] == statements[index + 1] and not is_free(statements[index]):
            print(f"DUP @{index + 1}: {statements[index]}")

    # A register assignment with no intervening read before a linear overwrite.
    for index, statement in enumerate(statements):
        match = ASSIGN_RE.match(statement)
        if not match:
            continue
        register = match.group(1)
        for next_index in range(index + 1, len(statements)):
            following = statements[next_index]
            if CONTROL_RE.match(following):
                break
            following_assignment = ASSIGN_RE.match(following)
            registers = REGISTER_RE.findall(following)
            if register not in registers:
                continue
            if following_assignment and following_assignment.group(1) == register:
                # A self-reference on the RHS is a read, not dead.
                rhs_registers = REGISTER_RE.findall(following_assignment.group(2))
                if register not in rhs_registers:
                    print(
                        f"DEAD-LINEAR @{index + 1}->{next_index + 1}: "
                        f"{statement!r} overwritten by {following!r}"
                    )
                break
            break

    # Save-and-test patterns that may fold to a direct sensor check.
    for index in range(len(statements) - 1):
        match = ASSIGN_RE.match(statements[index])
        if not match or not match.group(2).startswith("set "):
            continue
        register = match.group(1)
        if re.search(rf"\b{register}\b", statements[index + 1]) and statements[index + 1].startswith(("if ", "while ")):
            print(f"FOLD? @{index + 1}: {statements[index]!r} THEN {statements[index + 1]!r}")

    # Tell channels that nobody listens for in the whole program.
    listens = set(re.findall(r"^listenfor\s+(\w+)$", "\n".join(statements), re.MULTILINE))
    for index, statement in enumerate(statements):
        match = re.match(r"^tell\s+\S+\s+(\w+)$", statement)
        if match and match.group(1) not in listens:
            print(f"UNLISTENED-TELL @{index + 1}: {statement}")

    # Whole-program register liveness over if/else/jump control flow.
    labels = {
        statement[:-1]: index
        for index, statement in enumerate(statements)
        if re.match(r"^[A-Za-z_]\w*:$", statement)
    }
    if_else: dict[int, int | None] = {}
    if_end: dict[int, int] = {}
    else_end: dict[int, int] = {}
    stack: list[tuple[int, int | None]] = []
    for index, statement in enumerate(statements):
        if statement.startswith("if "):
            stack.append((index, None))
        elif statement in ("else", "else:"):
            start, _ = stack.pop()
            stack.append((start, index))
        elif statement == "endif":
            start, alternative = stack.pop()
            if_else[start] = alternative
            if_end[start] = index
            if alternative is not None:
                else_end[alternative] = index

    successors: list[set[int]] = [set() for _ in statements]
    for index, statement in enumerate(statements):
        next_index = index + 1
        if statement.startswith("jump "):
            label = statement.split(maxsplit=1)[1]
            if label in labels:
                successors[index].add(labels[label])
        elif statement == "end":
            pass
        elif statement.startswith("if ") and index in if_end:
            if next_index < len(statements):
                successors[index].add(next_index)
            alternative = if_else[index]
            false_index = (alternative + 1) if alternative is not None else (if_end[index] + 1)
            if false_index < len(statements):
                successors[index].add(false_index)
        elif statement in ("else", "else:") and index in else_end:
            after = else_end[index] + 1
            if after < len(statements):
                successors[index].add(after)
        elif next_index < len(statements):
            successors[index].add(next_index)

    uses: list[set[str]] = []
    defs: list[set[str]] = []
    for statement in statements:
        match = ASSIGN_RE.match(statement)
        if match:
            defs.append({match.group(1)})
            uses.append(set(REGISTER_RE.findall(match.group(2))))
        else:
            defs.append(set())
            uses.append(set(REGISTER_RE.findall(statement)))
    live_in: list[set[str]] = [set() for _ in statements]
    live_out: list[set[str]] = [set() for _ in statements]
    changed = True
    while changed:
        changed = False
        for index in range(len(statements) - 1, -1, -1):
            new_out = set().union(*(live_in[s] for s in successors[index])) if successors[index] else set()
            new_in = uses[index] | (new_out - defs[index])
            if new_out != live_out[index] or new_in != live_in[index]:
                live_out[index] = new_out
                live_in[index] = new_in
                changed = True
    for index, statement in enumerate(statements):
        match = ASSIGN_RE.match(statement)
        if match and match.group(1) not in live_out[index] and " / 0" not in statement:
            print(f"DEAD-CFG @{index + 1}: {statement}")
