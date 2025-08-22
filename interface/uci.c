#include "uci.h"
#include "board.h"
#include "diagnostic_tools.h"
#include "fen.h"
#include "move.h"
#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHITESPACE " \f\n\r\t\v"

void uci() {
  printf("id name %s %s\n", ID_NAME, ID_VERSION);
  printf("id author %s\n", ID_AUTHOR);
  printf("uciok\n");
  fflush(stdout);
}

void isready() {
  printf("readyok\n");
  fflush(stdout);
}

void position(char *command, char *saveptr, Board *state) {}

void go(char *command, char *saveptr, Board *state) {
  // TOOD: handle other args
  Move best_move = lazy_search(state, 6);
  printf("bestmove %s\n", move_to_algebraic(best_move, state->to_move));
  fflush(stdout);
}

int uci_main() {
  char input[2048];
  int exit_code = UCI_EXIT_UNKNOWN;
  Board *state = NULL;

  while (1) {
    if (fgets(input, sizeof(input), stdin) == NULL) {
      continue;
    }

    char *saveptr;
    char *command = strtok_r(input, WHITESPACE, &saveptr);

    if (command == NULL) {
      continue;
    } else if (strcmp(command, "ucinewgame") == 0) {
      Board *temp = init_default_board();
      if (state != NULL) {
        free(state);
      }
      state = temp;

    } else if (strcmp(command, "uci") == 0) {
      uci();

    } else if (strcmp(command, "isready") == 0) {
      isready();

    } else if (strcmp(command, "position") == 0) {
      char *next_token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (strcmp(next_token, "startpos") == 0) {
        Board *temp = init_default_board(); // icky icky
        free(state);
        state = temp;

        next_token = strtok_r(NULL, WHITESPACE, &saveptr);
      } else if (strcmp(next_token, "fen") == 0) {
        char fen[100];
        fen[0] = '\0';

        next_token = strtok_r(NULL, WHITESPACE, &saveptr);
        while (next_token != NULL && strcmp(next_token, "moves") != 0) {
          strcat(fen, next_token);
          strcat(fen, " ");
          next_token = strtok_r(NULL, WHITESPACE, &saveptr);
        }
        fen[strlen(fen) - 1] = '\0';

        Board *temp = fen_to_board(fen); // icky icky
        free(state);
        state = temp;
      }
      printf("next token: %s\n", next_token);
      if (next_token != NULL && strcmp(next_token, "moves") == 0) {
        next_token = strtok_r(NULL, WHITESPACE, &saveptr);
        while (next_token != NULL) {
          Move move = algebraic_to_move(state, next_token);
          board_make_move(state, move);
          next_token = strtok_r(NULL, WHITESPACE, &saveptr);
        }
      }

    } else if (strcmp(command, "go") == 0) {
      go(command, saveptr, state);
    }

    else if (strcmp(command, "d") == 0) {
      if (state == NULL) {
        printf(
            "No board initialized. Use 'ucinewgame' or 'position' command.\n");
      } else {
        printf("%s\n", board_to_string(state));
      }
    }

    else if (strcmp(command, "quit") == 0) {
      exit_code = EXIT_SUCCESS;
      break;
    }
  }
  return exit_code;
}
