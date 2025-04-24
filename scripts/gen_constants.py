def generate_knight_bitboards():
    knight_moves = [
        (2, 1),
        (2, -1),
        (-2, 1),
        (-2, -1),
        (1, 2),
        (1, -2),
        (-1, 2),
        (-1, -2),
    ]
    bitboards = []

    for i in range(64):
        bitboard = 0
        row, col = divmod(i, 8)
        for dx, dy in knight_moves:
            new_row, new_col = row + dx, col + dy
            if 0 <= new_row < 8 and 0 <= new_col < 8:
                new_index = new_row * 8 + new_col
                bitboard |= 1 << new_index
        bitboards.append(bitboard)

    return bitboards


def generate_king_bitboards():
    king_moves = [
        (-1, -1),
        (-1, 0),
        (-1, 1),
        (0, -1),
        (0, 1),
        (1, -1),
        (1, 0),
        (1, 1),
    ]
    bitboards = []

    for i in range(64):
        bitboard = 0
        row, col = divmod(i, 8)
        for dx, dy in king_moves:
            new_row, new_col = row + dx, col + dy
            if 0 <= new_row < 8 and 0 <= new_col < 8:
                new_index = new_row * 8 + new_col
                bitboard |= 1 << new_index
        bitboards.append(bitboard)
    return bitboards


def generate_header_file(file_path):
    knight_bitboards = generate_knight_bitboards()
    king_bitboards = generate_king_bitboards()

    with open(file_path, "w") as file:
        file.write("#ifndef MAUTLIER_CONSTANTS_H\n")
        file.write("#define MAUTLIER_CONSTANTS_H\n\n")
        file.write('#include "board.h"\n\n')

        file.write("static const Bitboard knight_moves[64] = {\n")
        for bb in knight_bitboards:
            file.write(f"    0x{bb:016x},\n")
        file.write("};\n\n")

        file.write("static const Bitboard king_moves[64] = {\n")
        for bb in king_bitboards:
            file.write(f"   0x{bb:016x},\n")
        file.write("};\n\n")
        file.write("#endif // MAUTLIER_CONSTANTS_H\n")


file_path = "../state/constants.h"
generate_header_file(file_path)
