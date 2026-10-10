#!/usr/bin/env python3
"""Check that the README tables agree with the solution files, and the live queue with its programs."""

import functools
import io
import re
import sys
import urllib.parse
from pathlib import Path
from typing import Dict, List, NamedTuple, Optional, Set

REPO_ROOT = Path(__file__).parent
README = REPO_ROOT / "README.md"
COMMANDS = REPO_ROOT / "COMMANDS.md"
QUEUE = REPO_ROOT / "SOLUTIONS_TO_TRY.md"
QUEUE_DIRECTORY = REPO_ROOT / "SolutionsToTry"
BLOB_PREFIX = "https://github.com/senegrom/7-billion-humans-solutions/blob/master/"

# Maps a README section heading to the folder its rows must link to.
SECTION_DIRECTORIES: Dict[str, str] = {
    "+99% Solutions": "Solutions99+",
    "+50% Solutions": "Solutions50+",
    "<50% Solutions": "SolutionsLowPercent",
}

SECTION_PATTERN = re.compile(r"^###### (.*)$")
LINK_PATTERN = re.compile(r"\[([^\]]*)\]\((\S*\.txt)\)")
FILE_TYPE_PATTERN = re.compile(r"\((speed|size|both)\)\.txt$", re.IGNORECASE)
FILE_YEAR_PATTERN = re.compile(r"^(Year \d{2}) - ")
MARKERS = "❌✔➕➖📋"

# Statements that the game does not count towards a solution's size.
BLOCK_END_PATTERN = re.compile(r"^end(if|for)$")
LABEL_PATTERN = re.compile(r"^[A-Za-z_]\w*:$")
COMMENT_COMMAND_PATTERN = re.compile(r"^comment \d+$")
CONDITION_PATTERN = re.compile(r"^if\b")
ELSE_PATTERN = re.compile(r"^else:?$")
JUMP_PATTERN = re.compile(r"^jump(?:\s+(\w+))?$")

# What the game's editor will not build at a level, so a solution using it has to be pasted in and carries the
# paste marker (the rules are in COMMANDS.md).
PASTE_MARKER = "📋"
ONE_DIRECTION_COMMAND_LIST_PATTERN = re.compile(
    r"^(pickup|giveto|takefrom|\w+\s*=\s*set)\s+\w+(,\w+)+$"
)
STEP_DIRECTION_LIST_PATTERN = re.compile(r"^step\s+\w+(,\w+)+$")
MYITEM_PATTERN = re.compile(r"\bmyitem\b")
ASSIGNMENT_PATTERN = re.compile(r"^mem\d\s*=\s*(\w+)")
PALETTE_ROW_PATTERN = re.compile(r"^\|\s*\d+\s*\|\s*(\d+)\s*\|[^|]*\|([^|]*)\|")
FIRST_YEAR_WITH_STEP_DIRECTION_LISTS = 30
FIRST_YEAR_WITH_MYITEM = 21

# The live queue: an entry heading, its links, and the size a program's file name promises.
QUEUE_HEADING_PATTERN = re.compile(r"^### ")
QUEUE_LINK_PATTERN = re.compile(r"\]\(<([^>]+)>\)")
QUEUE_FILE_SIZE_PATTERN = re.compile(r"size (\d+)\.txt$")


class Row(NamedTuple):
    """One solution row of one of the README tables."""

    line_number: int
    section: str
    year: str
    name: str
    path: Optional[Path]
    url: str
    size: str
    speed: str
    pasted: bool


def read_statements(path: Path) -> List[str]:
    """
    Splits a solution file into the statements the game counts as its size.

    Blank lines, `-- comment --` headers and the trailing `DEFINE COMMENT` payloads are dropped, and a condition
    spread over several lines is joined back into a single statement.  A byte-order mark at the start is ignored.

    Args:
        path: The solution file to read.

    Returns:
        A list of statements, one per line of code as the game's editor shows it.
    """
    statements: List[str] = []
    pending = ""
    for raw_line in path.read_text(encoding="utf-8-sig", errors="replace").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("--"):
            continue
        if line.startswith("DEFINE "):
            break  # the comment/label payloads at the end of a file are not code
        pending = f"{pending} {line}" if pending else line
        if CONDITION_PATTERN.match(pending) and not pending.endswith(":"):
            continue  # a condition continues on the next line
        statements.append(pending)
        pending = ""
    if pending:
        statements.append(pending)
    return statements


def is_free(statement: str) -> bool:
    """
    Reports whether a statement is one the game does not charge a command for.

    Args:
        statement: A single statement of a solution.

    Returns:
        True if the statement does not count towards the solution's size.
    """
    if ELSE_PATTERN.match(statement):
        return False  # `else` looks like a label but does cost a command
    return bool(
        LABEL_PATTERN.match(statement)
        or BLOCK_END_PATTERN.match(statement)
        or COMMENT_COMMAND_PATTERN.match(statement)
    )


def solution_size(path: Path) -> int:
    """
    Counts the commands in a solution, the way the game's size counter does.

    Labels, block ends (`endif`, `endfor`) and `comment` commands are free; everything else, including
    `else` and `end`, costs one command.

    Args:
        path: The solution file to measure.

    Returns:
        The number of commands in the solution.
    """
    return sum(1 for statement in read_statements(path) if not is_free(statement))


def label_problems(statements: List[str]) -> List[str]:
    """
    Finds labels and jumps that do not pair up.

    In the game a label is only ever the far end of a jump arrow, so it refuses to paste a program with a label that
    nothing jumps to.  A jump with no label, or a label defined twice, leaves a jump without one place to go.  Labels
    are free in the size count, which is why this check and not the Size check has to catch a lost or copied one.

    Args:
        statements: The solution's statements.

    Returns:
        A description of each problem, in a fixed order.
    """
    labels = [
        s[:-1]
        for s in statements
        if LABEL_PATTERN.match(s) and not ELSE_PATTERN.match(s)
    ]
    jumps = [match.group(1) for match in map(JUMP_PATTERN.match, statements) if match]
    problems = [
        f"no jump leads to the label {label}:, and the game refuses such a paste"
        for label in dict.fromkeys(labels)
        if label not in jumps
    ]
    problems += [
        f"the label {label}: is defined more than once"
        for label in dict.fromkeys(labels)
        if labels.count(label) > 1
    ]
    problems += [
        f"`jump {target}` has no label {target}: to lead to"
        for target in dict.fromkeys(jumps)
        if target is not None and target not in labels
    ]
    if None in jumps:
        problems.append("a `jump` names no label")
    return problems


def cell_value(cell: str) -> Optional[int]:
    """
    Reads the number out of a Size or Speed table cell.

    Args:
        cell: The cell's text, which may be bold and may carry a marker, a `~` or a range such as `10-11`.

    Returns:
        The number, taking the lowest value of a range, or None if the cell holds no plain number.
    """
    text = cell.replace("**", "")
    for marker in MARKERS:
        text = text.replace(marker, "")
    text = text.strip().lstrip("~").strip()
    match = re.match(r"^(\d+)(-\d+)?$", text)
    return int(match.group(1)) if match else None


def is_bold(cell: str) -> bool:
    """
    Reports whether a table cell is bold, which marks the value a solution is optimised for.

    Args:
        cell: The cell's text.

    Returns:
        True if the cell is bold.
    """
    return "**" in cell


def url_to_path(url: str) -> Optional[Path]:
    """
    Turns a README link into the path of the file it points at.

    Args:
        url: The link's target.

    Returns:
        The path the link points at, or None if the link is not a link into this repository.
    """
    if not url.startswith(BLOB_PREFIX):
        return None
    return REPO_ROOT / urllib.parse.unquote(url[len(BLOB_PREFIX) :])


@functools.lru_cache(maxsize=None)
def names_in(directory: Path) -> Set[str]:
    """
    Lists the exact names in a folder, so a link's letter case can be checked even where the file system ignores it.

    Args:
        directory: The folder to list.

    Returns:
        The names of the entries in the folder.
    """
    return {entry.name for entry in directory.iterdir()}


def exists_exactly(path: Path) -> bool:
    """
    Reports whether a file exists with exactly this spelling, as GitHub requires.

    Args:
        path: The file to look for.

    Returns:
        True if the file exists and its name matches letter for letter.
    """
    return path.is_file() and path.name in names_in(path.parent)


def parse_readme() -> List[Row]:
    """
    Reads every solution row out of the README's tables.

    Returns:
        A list of rows, in the order they appear in the README.
    """
    rows: List[Row] = []
    section = None
    for line_number, line in enumerate(
        README.read_text(encoding="utf-8-sig").splitlines(), 1
    ):
        heading = SECTION_PATTERN.match(line)
        if heading:
            section = (
                heading.group(1) if heading.group(1) in SECTION_DIRECTORIES else None
            )
            continue
        if section is None or not line.startswith("|"):
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) < 5 or cells[0] == "Year" or set(cells[0]) <= set(":- "):
            continue  # heading or separator row
        link = LINK_PATTERN.search(cells[1])
        rows.append(
            Row(
                line_number=line_number,
                section=section,
                year=cells[0],
                name=link.group(1) if link else cells[1],
                path=url_to_path(link.group(2)) if link else None,
                url=link.group(2) if link else "",
                size=cells[3],
                speed=cells[4],
                pasted=PASTE_MARKER in cells[1],
            )
        )
    return rows


def check_rows(rows: List[Row]) -> List[str]:
    """
    Checks every README row against the file it links to.

    Args:
        rows: The rows read from the README.

    Returns:
        A list of problem descriptions, empty if every row is correct.
    """
    problems: List[str] = []
    for row in rows:
        where = f'README.md:{row.line_number} "{row.year} {row.name}"'
        if not row.url:
            problems.append(f"{where}: the Name cell does not link to a solution file")
            continue
        if row.path is None:
            problems.append(
                f"{where}: link does not point into this repository: {row.url}"
            )
            continue
        if not exists_exactly(row.path):
            problems.append(
                f"{where}: link points at a file that does not exist (letter case counts): {row.url}"
            )
            continue
        expected_directory = SECTION_DIRECTORIES[row.section]
        if row.path.parent.name != expected_directory:
            problems.append(
                f"{where}: is listed under {row.section} but links to {row.path.parent.name}, "
                f"expected {expected_directory}"
            )
            continue
        file_year = FILE_YEAR_PATTERN.match(row.path.name)
        if file_year and row.year != file_year.group(1):
            problems.append(
                f"{where}: Year column says {row.year} but the solution is for {file_year.group(1)}"
            )
        problems.extend(
            f"{where}: {problem}"
            for problem in label_problems(read_statements(row.path))
        )
        size = cell_value(row.size)
        actual_size = solution_size(row.path)
        if size is None:
            problems.append(f"{where}: Size column is not a number: {row.size}")
        elif size != actual_size:
            problems.append(
                f"{where}: Size column says {size} but the solution has {actual_size} commands"
            )
        if is_bold(row.speed) and cell_value(row.speed) is None:
            problems.append(f"{where}: Speed column is not a number: {row.speed}")
        file_type_match = FILE_TYPE_PATTERN.search(row.path.name)
        if not file_type_match:
            continue  # check_names.py reports the bad file name
        file_type = file_type_match.group(1).lower()
        expected_bold = {
            "size": (True, False),
            "speed": (False, True),
            "both": (True, True),
        }[file_type]
        if (is_bold(row.size), is_bold(row.speed)) != expected_bold:
            problems.append(
                f"{where}: is a ({file_type}) solution, so only the "
                f"{'Size and Speed columns' if file_type == 'both' else file_type.capitalize() + ' column'} "
                "should be bold"
            )
    return problems


def check_files_are_listed(rows: List[Row]) -> List[str]:
    """
    Checks that every solution file is listed in the README exactly once.

    Args:
        rows: The rows read from the README.

    Returns:
        A list of problem descriptions, empty if every file is listed exactly once.
    """
    problems: List[str] = []
    listed: Set[Path] = set()
    for row in rows:
        if row.path is None:
            continue
        if row.path in listed:
            problems.append(
                f"README.md:{row.line_number}: {row.path.name} is listed more than once"
            )
        listed.add(row.path)
    for directory in SECTION_DIRECTORIES.values():
        for path in sorted((REPO_ROOT / directory).glob("*.txt")):
            if path not in listed:
                problems.append(f'"{directory}/{path.name}" is not listed in README.md')
    return problems


@functools.lru_cache(maxsize=None)
def palettes() -> Dict[int, Set[str]]:
    """
    Reads each level's command palette from the table in COMMANDS.md.

    Returns:
        The commands each Year's editor offers, keyed by Year number.
    """
    found: Dict[int, Set[str]] = {}
    for line in COMMANDS.read_text(encoding="utf-8-sig").splitlines():
        match = PALETTE_ROW_PATTERN.match(line)
        if match:
            found[int(match.group(1))] = set(match.group(2).split())
    return found


def statement_command(statement: str) -> Optional[str]:
    """
    Names the palette command a statement needs.

    Args:
        statement: A single statement of a solution.

    Returns:
        The command's name as the palette table spells it, or None for a statement that rides along with another
        (labels, block ends, comments).
    """
    if ELSE_PATTERN.match(statement):
        return "if"
    if is_free(statement):
        return None
    assignment = ASSIGNMENT_PATTERN.match(statement)
    word = assignment.group(1) if assignment else statement.split()[0]
    return "listen" if word == "listenfor" else word


def paste_only_reason(
    year: int, statements: List[str], palette: Optional[Set[str]] = None
) -> Optional[str]:
    """
    Finds the first statement of a solution that the game's editor will not build at the solution's level.

    Args:
        year: The level's Year number.
        statements: The solution's statements.
        palette: The commands the level's editor offers, if known.

    Returns:
        Why that statement has to be pasted in, or None if the whole solution can be typed in the editor.
    """
    for statement in statements:
        command = statement_command(statement)
        if (
            palette is not None
            and command is not None
            and command not in palette
            and not (command in ("set", "calc") and palette & {"set", "calc"})
        ):
            return f"`{statement}` uses {command}, which the Year {year} editor does not offer"
        if ONE_DIRECTION_COMMAND_LIST_PATTERN.match(statement):
            return f"`{statement}` names several directions, which the editor only allows on step and foreachdir"
        if (
            year < FIRST_YEAR_WITH_STEP_DIRECTION_LISTS
            and STEP_DIRECTION_LIST_PATTERN.match(statement)
        ):
            return (
                f"`{statement}` names several directions, which the editor only allows on step from "
                f"Year {FIRST_YEAR_WITH_STEP_DIRECTION_LISTS}"
            )
        if year < FIRST_YEAR_WITH_MYITEM and MYITEM_PATTERN.search(statement):
            return f"`{statement}` uses myitem, which the editor only offers from Year {FIRST_YEAR_WITH_MYITEM}"
    return None


def check_paste_markers(rows: List[Row]) -> List[str]:
    """
    Checks that a row carries the paste marker exactly when the editor cannot build its solution.

    Args:
        rows: The rows read from the README.

    Returns:
        A list of problem descriptions, empty if every marker is right.
    """
    problems: List[str] = []
    for row in rows:
        year = re.match(r"^Year (\d+)$", row.year)
        if row.path is None or not exists_exactly(row.path) or not year:
            continue  # check_rows reports the broken link or Year cell
        where = f'README.md:{row.line_number} "{row.year} {row.name}"'
        palette = palettes().get(int(year.group(1)))
        if palette is None:
            problems.append(
                f"{where}: COMMANDS.md has no command palette for {row.year}"
            )
        reason = paste_only_reason(
            int(year.group(1)), read_statements(row.path), palette
        )
        if reason and not row.pasted:
            problems.append(
                f"{where}: {reason}, so the row needs the {PASTE_MARKER} marker"
            )
        elif row.pasted and not reason:
            problems.append(
                f"{where}: carries the {PASTE_MARKER} marker but the editor can build all of it"
            )
    return problems


def check_lower_percent_rows_are_better(rows: List[Row]) -> List[str]:
    """
    Checks that a less reliable solution is only listed if it beats the more reliable one.

    Args:
        rows: The rows read from the README.

    Returns:
        A list of problem descriptions, empty if every row earns its place.
    """
    problems: List[str] = []
    best: Dict[str, Dict[str, int]] = {}
    for row in rows:
        columns = {
            column: cell_value(cell)
            for column, cell in (("Size", row.size), ("Speed", row.speed))
            if is_bold(cell)
        }
        year_best = best.setdefault(row.year, {})
        comparable = {
            column: (value, year_best[column])
            for column, value in columns.items()
            if value is not None and column in year_best
        }
        if (
            row.section != "+99% Solutions"
            and comparable
            and not any(value < previous for value, previous in comparable.values())
        ):
            beaten = ", ".join(
                f"{column} {value} against {previous}"
                for column, (value, previous) in sorted(comparable.items())
            )
            problems.append(
                f'README.md:{row.line_number} "{row.year} {row.name}": does not beat the more reliable solution '
                f"already listed ({beaten}), so it does not belong in {row.section}"
            )
        for column, value in columns.items():
            if value is not None and value < year_best.get(column, value + 1):
                year_best[column] = value
    return problems


def check_queue() -> List[str]:
    """
    Checks the live queue in SOLUTIONS_TO_TRY.md against its programs in SolutionsToTry/.

    Every link must resolve and every queue program must be linked; a program's labels must pair up with its jumps;
    a "size N" in a program's file name must be its size; and an entry's heading carries the paste marker exactly
    when one of its programs has to be pasted in.  A checkout without the queue has nothing to check.

    Returns:
        A list of problem descriptions, empty if the queue is consistent.
    """
    if not QUEUE.is_file():
        return []
    problems: List[str] = []
    linked: Set[Path] = set()
    heading: Optional[str] = None
    heading_line = 0
    entries: Dict[int, List[Path]] = {}
    marks: Dict[int, str] = {}
    for line_number, line in enumerate(
        QUEUE.read_text(encoding="utf-8-sig").splitlines(), 1
    ):
        if QUEUE_HEADING_PATTERN.match(line):
            heading, heading_line = line, line_number
            continue
        for target in QUEUE_LINK_PATTERN.findall(line):
            path = REPO_ROOT / target
            if not exists_exactly(path):
                problems.append(
                    f"SOLUTIONS_TO_TRY.md:{line_number}: link points at a file that does not exist: {target}"
                )
                continue
            if path.parent == QUEUE_DIRECTORY:
                linked.add(path)
                if heading is not None:
                    entries.setdefault(heading_line, []).append(path)
                    marks[heading_line] = heading
    for path in sorted(QUEUE_DIRECTORY.glob("*.txt")):
        shown = f"SolutionsToTry/{path.name}"
        if path not in linked:
            problems.append(f'"{shown}" is not linked from SOLUTIONS_TO_TRY.md')
        statements = read_statements(path)
        problems.extend(
            f'"{shown}": {problem}' for problem in label_problems(statements)
        )
        promised = QUEUE_FILE_SIZE_PATTERN.search(path.name)
        if promised and int(promised.group(1)) != solution_size(path):
            problems.append(
                f'"{shown}": the file name promises size {promised.group(1)} but the program has '
                f"{solution_size(path)} commands"
            )
    for line_number, paths in entries.items():
        year = FILE_YEAR_PATTERN.match(paths[0].name)
        if not year:
            continue
        number = int(year.group(1).split()[1])
        reasons = [
            paste_only_reason(number, read_statements(path), palettes().get(number))
            for path in paths
        ]
        reason = next((r for r in reasons if r), None)
        pasted = PASTE_MARKER in marks[line_number]
        if reason and not pasted:
            problems.append(
                f"SOLUTIONS_TO_TRY.md:{line_number}: {reason}, so the heading needs the {PASTE_MARKER} marker"
            )
        elif pasted and not reason:
            problems.append(
                f"SOLUTIONS_TO_TRY.md:{line_number}: the heading carries the {PASTE_MARKER} marker but the editor "
                "can build every program it links"
            )
    return problems


def main() -> None:
    """
    Checks the README and the live queue against their files and exits non-zero if anything is wrong.
    """
    # The markers are emoji: escape them on a console that cannot print them rather than crash.
    if isinstance(sys.stdout, io.TextIOWrapper):
        sys.stdout.reconfigure(errors="backslashreplace")
    rows = parse_readme()
    problems = (
        check_rows(rows)
        + check_files_are_listed(rows)
        + check_lower_percent_rows_are_better(rows)
        + check_paste_markers(rows)
        + check_queue()
    )
    if not problems:
        print(f"Finished! Checked {len(rows)} solutions and there are no issues :)")
        return
    print("Issues found:\n")
    for problem in problems:
        print(f"- {problem}")
    sys.exit(1)


if __name__ == "__main__":
    main()
