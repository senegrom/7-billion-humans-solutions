from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXTERNAL = ROOT / ".codex_external_abfipes"
YEARS = {26, *range(28, 45), 46, 47}

condition = re.compile(r"^(if|while)\b")
label = re.compile(r"^[A-Za-z_]\w*:$")
block_end = re.compile(r"^end(if|while|for)$")
comment = re.compile(r"^comment \d+$")
else_statement = re.compile(r"^else:?$")


def statements(path: Path) -> list[str]:
    result: list[str] = []
    pending = ""
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("--"):
            continue
        if line.startswith("DEFINE "):
            break
        pending = f"{pending} {line}" if pending else line
        if condition.match(pending) and not pending.endswith(":"):
            continue
        result.append(pending)
        pending = ""
    if pending:
        result.append(pending)
    return result


def size(path: Path) -> int:
    return sum(
        bool(else_statement.match(stmt))
        or not (label.match(stmt) or block_end.match(stmt) or comment.match(stmt))
        for stmt in statements(path)
    )


def year(path: Path) -> int | None:
    match = re.match(r"Year (\d+)", path.name)
    return int(match.group(1)) if match else None


for directory in (ROOT / "Solutions99+", ROOT / "Solutions50+"):
    for path in sorted(directory.glob("Year *")):
        if year(path) in YEARS:
            print(f"CURRENT\t{path.relative_to(ROOT)}\t{size(path)}")

for path in sorted(EXTERNAL.rglob("Year *")):
    if path.is_file() and year(path) in YEARS:
        print(f"EXTERNAL\t{path.relative_to(EXTERNAL)}\t{size(path)}")
