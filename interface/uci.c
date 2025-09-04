#include "uci.h"
#include "board.h"
#include "diagnostic_tools.h"
#include "fen.h"
#include "move.h"
#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define WHITESPACE " \f\n\r\t\v"

FILE *book = NULL;

int init_book(const char *path) {
  book = fopen(path, "rb");
  if (!book) {
    fprintf(stderr, "Failed to open book file: %s\n", path);
    return 0;
  }
  return 1;
}

void close_book() {
  if (book)
    fclose(book);
  book = NULL;
}

static UCIOptions uci_options = {
    .transposition_table = true,
    .quiescence_search = true,
    .book = true,
    .null_move_pruning = true,
    .move_ordering = true,
    .lmr = true,
};

void uci() {
  printf("id name %s %s\n", ID_NAME, ID_VERSION);
  printf("id author %s\n", ID_AUTHOR);
  printf("option name TranspositionTable type check default true\n");
  printf("option name QuiescenceSearch type check default true\n");
  printf("option name Book type check default true\n");
  printf("option name NullMovePruning type check default true\n");
  printf("option name MoveOrdering type check default true\n");
  printf("option name LMR type check default true\n");
  printf("uciok\n");
  fflush(stdout);
}

void isready() {
  printf("readyok\n");
  fflush(stdout);
}

static inline long get_time_ms() { // TODO: remove copy paste
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec * 1000) + (t.tv_usec / 1000);
}

void go(char *command, char *saveptr, Board *state) {
  int wtime = -1;
  int btime = -1;
  int winc = 0;
  int binc = 0;
  int movestogo = 0;
  int movetime = -1;
  int depth = -1;
  int infinite = 0;

  char *token = strtok_r(NULL, WHITESPACE, &saveptr);
  while (token != NULL) {
    if (strcmp(token, "wtime") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        wtime = atoi(token);
    } else if (strcmp(token, "btime") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        btime = atoi(token);
    } else if (strcmp(token, "winc") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        winc = atoi(token);
    } else if (strcmp(token, "binc") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        binc = atoi(token);
    } else if (strcmp(token, "movestogo") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        movestogo = atoi(token);
    } else if (strcmp(token, "movetime") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL)
        movetime = atoi(token);
    } else if (strcmp(token, "depth") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        depth = atoi(token);
      }
    } else if (strcmp(token, "infinite") == 0) {
      infinite = 1;
    }
    token = strtok_r(NULL, WHITESPACE, &saveptr);
  }

  SearchRequestInfo info = {0};
  if (movetime != -1) {
    info.max_duration_ms = movetime;
  } else if (infinite) {
    info.infinite = 1;
  } else if (wtime != -1 && btime != -1) {
    int time = (state->to_move == WHITE) ? wtime : btime;
    int inc = (state->to_move == WHITE) ? winc : binc;
    if (movestogo != 0) {
      time = time / movestogo;
    } else {
      time = time / 30; // assume 30 moves to end of game
    }
    time += inc;
    time = time * 90 / 100;
    if (time < 40) {
      time = 40; // minimum time
    }
    info.max_duration_ms = time;
  } else if (depth != -1) {
    info.max_depth = depth;
  } else {
    info.max_duration_ms = 5000; // default to 5 seconds
  }

  search_position(state, &info, &uci_options, book);
}

void setoption(char *command, char *saveptr) {
  char *token = strtok_r(NULL, WHITESPACE, &saveptr);
  if (token == NULL || strcmp(token, "name") != 0) {
    return;
  }

  // TODO: yucky yucky logic
  token = strtok_r(NULL, WHITESPACE, &saveptr);
  if (strcmp(token, "TranspositionTable") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.transposition_table = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.transposition_table = false;
        }
      }
    }
  } else if (strcmp(token, "QuiescenceSearch") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.quiescence_search = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.quiescence_search = false;
        }
      }
    }
  } else if (strcmp(token, "Book") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.book = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.book = false;
        }
      }
    }
  } else if (strcmp(token, "NullMovePruning") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.null_move_pruning = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.null_move_pruning = false;
        }
      }
    }
  } else if (strcmp(token, "MoveOrdering") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.move_ordering = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.move_ordering = false;
        }
      }
    }
  } else if (strcmp(token, "LMR") == 0) {
    token = strtok_r(NULL, WHITESPACE, &saveptr);
    if (token != NULL && strcmp(token, "value") == 0) {
      token = strtok_r(NULL, WHITESPACE, &saveptr);
      if (token != NULL) {
        if (strcmp(token, "true") == 0) {
          uci_options.lmr = true;
        } else if (strcmp(token, "false") == 0) {
          uci_options.lmr = false;
        }
      }
    }
  }
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
      if (book == NULL) {
        init_book("book/Cerebellum3Merge.bin"); // TODO: make uci argument
      }
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
        printf("No board initialized. Use 'ucinewgame' or 'position' "
               "command.\n");
      } else {
        printf("%s\n", board_to_debug_string(state));
      }
    }

    else if (strcmp(command, "setoption") == 0) {
      setoption(command, saveptr);
    }

    else if (strcmp(command, "quit") == 0) {
      if (book != NULL) {
        close_book();
      }
      exit_code = EXIT_SUCCESS;
      break;
    }
  }
  return exit_code;
}
