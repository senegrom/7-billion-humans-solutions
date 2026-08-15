from __future__ import annotations

MASK = 0xFFFFFFFF
POWER_ON = 0xABAB1981
SEED_MUL = 2654435761
SEED_ADD = 12345


def xorshift32(x: int) -> int:
    x ^= (x << 13) & MASK
    x ^= x >> 17
    x ^= (x << 5) & MASK
    return x & MASK


def transition_bit_mask(draw: int, bit: int) -> int:
    result = 0
    for source_bit in range(32):
        x = 1 << source_bit
        for _ in range(draw):
            x = xorshift32(x)
        if (x >> bit) & 1:
            result |= 1 << source_bit
    return result


def nullspace(equations: list[int]) -> tuple[int, list[int]]:
    rows = [row for row in equations if row]
    pivot_cols: list[int] = []
    rank = 0
    for col in range(32):
        selected = next((i for i in range(rank, len(rows)) if (rows[i] >> col) & 1), None)
        if selected is None:
            continue
        rows[rank], rows[selected] = rows[selected], rows[rank]
        for i in range(len(rows)):
            if i != rank and ((rows[i] >> col) & 1):
                rows[i] ^= rows[rank]
        pivot_cols.append(col)
        rank += 1
        if rank == len(rows):
            break
    rows = rows[:rank]
    free_cols = [col for col in range(32) if col not in pivot_cols]
    basis: list[int] = []
    for free in free_cols:
        value = 1 << free
        for row, pivot in zip(rows, pivot_cols):
            if (row >> free) & 1:
                value |= 1 << pivot
        basis.append(value)
    return rank, basis


def enumerate_span(basis: list[int]):
    value = 0
    previous_gray = 0
    yield value
    for index in range(1, 1 << len(basis)):
        gray = index ^ (index >> 1)
        changed = gray ^ previous_gray
        basis_index = changed.bit_length() - 1
        value ^= basis[basis_index]
        previous_gray = gray
        yield value


def draw_values(initial: int, first_draw: int, count: int, modulus: int) -> list[int]:
    x = POWER_ON if initial == 0 else initial
    values: list[int] = []
    for draw in range(1, first_draw + count):
        x = xorshift32(x)
        if draw >= first_draw:
            values.append(x % modulus)
    return values


def y33_candidates() -> list[int]:
    equations: list[int] = []
    for pair in range(8):
        left_draw = 33 + 2 * pair
        right_draw = left_draw + 1
        for bit in range(2):  # Equality modulo 100 implies equality modulo 4.
            equations.append(
                transition_bit_mask(left_draw, bit)
                ^ transition_bit_mask(right_draw, bit)
            )
    rank, basis = nullspace(equations)
    witnesses: list[int] = []
    for state in enumerate_span(basis):
        values = draw_values(state, 33, 16, 100)
        if all(values[i] == values[i + 1] for i in range(0, 16, 2)):
            witnesses.append(state)
    print(f"Y33 mod4 rank={rank}, nullity={len(basis)}, candidates_checked={1 << len(basis)}")
    return witnesses


def emulator_seeds_for_state(state: int) -> list[int]:
    if not (state & 1):
        return []
    inverse = pow(SEED_MUL, -1, 1 << 32)
    seeds: list[int] = []
    for raw in (state, state - 1):
        seed = ((raw - SEED_ADD) * inverse) & MASK
        actual = ((seed * SEED_MUL + SEED_ADD) & MASK) | 1
        if seed != 1 and actual == state:
            seeds.append(seed)
    if state == POWER_ON:
        seeds.append(1)
    return sorted(set(seeds))


def y37_parity_space() -> tuple[int, list[int]]:
    # 36 randu cubes consume 72 constructor rolls; values are draws 73..108.
    equations = [transition_bit_mask(draw, 0) for draw in range(73, 109)]
    return nullspace(equations)


y33 = y33_candidates()
print(f"Y33 full witnesses={len(y33)} probability={len(y33)}/2^32")
for state in y33[:20]:
    values = draw_values(state, 33, 16, 100)
    seeds = emulator_seeds_for_state(state)
    print(f"  state=0x{state:08X} values={values} emulator_seeds={seeds}")
all_trial_seeds = sorted(seed for state in y33 for seed in emulator_seeds_for_state(state))
print(f"Y33 first emulator trial seed={all_trial_seeds[0] if all_trial_seeds else 'NONE'}")

y37_rank, y37_basis = y37_parity_space()
print(f"Y37 parity rank={y37_rank}, nullity={len(y37_basis)}, parity_candidates={1 << len(y37_basis)}")
y37 = [
    state
    for state in enumerate_span(y37_basis)
    if all(value == 0 for value in draw_values(state, 73, 36, 10))
]
print(f"Y37 full witnesses={len(y37)} probability={len(y37)}/2^32")
for state in y37[:20]:
    print(f"  state=0x{state:08X} emulator_seeds={emulator_seeds_for_state(state)}")
