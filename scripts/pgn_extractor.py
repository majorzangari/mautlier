import chess.pgn
import random

# Input PGN file
pgn_file = "../book/games.pgn"
# Output file
output_file = "../book/random_positions.txt"

with (
    open(pgn_file, "r", encoding="utf-8") as in_f,
    open(output_file, "w", encoding="utf-8") as out_f,
):
    game_count = 0
    position_count = 0

    while True:
        game = chess.pgn.read_game(in_f)
        if game is None:
            break  # End of file

        game_count += 1
        board = game.board()

        # Reservoir sampling variables
        selected_fen = None
        count = 0

        for move in game.mainline_moves():
            board.push(move)
            fullmove = board.fullmove_number
            if 10 <= fullmove <= 40:
                count += 1
                if random.randint(1, count) == 1:
                    selected_fen = board.fen()

        if selected_fen:
            result = game.headers.get("Result", "*")
            out_f.write(f'{selected_fen}; result "{result}"\n')
            position_count += 1

        # Optional: print progress every 1000 games
        if game_count % 1000 == 0:
            print(f"Processed {game_count} games, collected {position_count} positions")

print(
    f"Finished processing {game_count} games, extracted {position_count} random positions"
)
