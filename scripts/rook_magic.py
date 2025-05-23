import random as rand
import numpy as np

RANK_1: np.uint64 = np.uint64(0x00000000000000FF)
FILE_A: np.uint64 = np.uint64(0x8080808080808080)
RANK_8: np.uint64 = np.uint64(0xFF00000000000000)
FILE_H: np.uint64 = np.uint64(0x0101010101010101)


def get_blocker_mask(square: int) -> np.uint64:
    rank, file = divmod(square, 8)
    rank_bb: np.uint64 = RANK_1 << (np.uint64(8) * np.uint64(rank))
    file_bb: np.uint64 = FILE_H << np.uint64(file)
    attacking_pieces = rank_bb | file_bb

    blockers = attacking_pieces
    if file != 7:
        blockers &= ~FILE_A
    if file != 0:
        blockers &= ~FILE_H
    if rank != 7:
        blockers &= ~RANK_8
    if rank != 0:
        blockers &= ~RANK_1
    blockers &= ~(np.uint64(1) << np.uint64(square))

    return blockers


def bitset_subsets(bitset: np.uint64) -> list[np.uint64]:
    subsets = []
    subset = bitset
    while True:
        subsets.append(subset)
        if subset == 0:
            break
        subset = (subset - 1) & bitset
    return subsets


def get_rook_attacks(blockers: np.uint64, square: int) -> np.uint64:
    attacks = np.uint64(0)
    rank, file = divmod(square, 8)
    for r in range(rank + 1, 8):
        sq = r * 8 + file
        attacks |= np.uint64(1) << np.uint64(sq)
        if blockers & (np.uint64(1) << np.uint64(sq)):
            break

    for r in range(rank - 1, -1, -1):
        sq = r * 8 + file
        attacks |= np.uint64(1) << np.uint64(sq)
        if blockers & (np.uint64(1) << np.uint64(sq)):
            break

    for f in range(file + 1, 8):
        sq = rank * 8 + f
        attacks |= np.uint64(1) << np.uint64(sq)
        if blockers & (np.uint64(1) << np.uint64(sq)):
            break

    for f in range(file - 1, -1, -1):
        sq = rank * 8 + f
        attacks |= np.uint64(1) << np.uint64(sq)
        if blockers & (np.uint64(1) << np.uint64(sq)):
            break
    return attacks


def get_all_rook_attacks(square: int) -> dict[np.uint64, np.uint64]:
    blocker_mask = get_blocker_mask(square)
    subsets = bitset_subsets(blocker_mask)
    return {blocker: get_rook_attacks(blocker, square) for blocker in subsets}


def get_index_bits(square: int) -> int:
    out = 10
    rank, file = divmod(square, 8)
    if rank == 0 or rank == 7:
        out += 1
    if file == 0 or file == 7:
        out += 1
    return out


def find_magic(square: int) -> np.uint64:
    attack_map = get_all_rook_attacks(square)
    index_bits = get_index_bits(square)
    blocker_mask = get_blocker_mask(square)

    while True:
        fail = False
        magic = (
            np.uint64(rand.getrandbits(64))
            & np.uint64(rand.getrandbits(64))
            & np.uint64(rand.getrandbits(64))
        )
        map = {}
        for blocker in bitset_subsets(blocker_mask):
            index = (blocker * magic) >> (64 - index_bits)
            if index not in map:
                map[index] = attack_map[blocker]
            elif map[index] != attack_map[blocker]:
                fail = True
                break
        if not fail:
            return magic


if __name__ == "__main__":
    for i in range(64):
        magic = find_magic(i)
        print(hex(magic))
