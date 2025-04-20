#include "state/board.h"
#include "state/fen.h"

int main(void) {
  char str[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Board *board = fen_to_board(str);

  return 0;
}
