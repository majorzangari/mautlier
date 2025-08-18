#include "cli.h"
#include "board.h"
#include "misc.h"
#include "perft.h"

#include <stdlib.h>

DEFINE_STACK_TYPE(MoveHistory, Move, 2048)

int cli_main() {
  char input[256];
  int exit_code = 0;

  Board *board = init_default_board();
  MoveHistory move_history;
  MoveHistory_init(&move_history);

  while (1) {
    printf("mautlier> ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
      continue;
    }

    // Remove trailing newline character
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
      input[len - 1] = '\0';
    }

    // Process the input command
    char *saveptr;
    char *command = strtok_r(input, " ", &saveptr);

    if (or_strcmp(command, 3, "exit", "quit", "q")) {
      printf("Exiting CLI...\n");
      break;
    }

    else if (or_strcmp(command, 1, "help")) {
      printf("Available commands:\n");
      printf("  help - Show this help message\n");
      printf("  exit/quit - Exit the CLI\n");
    }

    else if (or_strcmp(command, 1, "perft")) {
      char *steps_str = strtok_r(NULL, " ", &saveptr);
      if (steps_str == NULL) {
        printf("Error: Missing number of steps for perft\n");
      } else {
        char *endptr;
        long steps = strtol(steps_str, &endptr, 10);
        if (*endptr != '\0' || steps < 1) {
          printf("Error: Invalid number of steps for perft\n");
        } else {
          printf("Running perft with %ld steps...\n", steps);
          perft_divide(board, steps);
        }
      }
    }

    else if (or_strcmp(command, 2, "position", "pos")) {
      char *arg = strtok_r(NULL, " ", &saveptr);
      if (arg == NULL) {
        printf("Error: Missing position arg\n");
      } else if (or_strcmp(arg, 2, "start_pos", "start")) {
        free(board);
        board = init_default_board();
      } else {
        printf("Unknown position command: '%s'\n", arg);
      }
    }

    else if (or_strcmp(command, 2, "move", "moves")) {
      char *arg = strtok_r(NULL, " ", &saveptr);
      if (arg == NULL) {
        printf("Error: Missing move string for move command\n");

      } else if (or_strcmp(arg, 2, "list", "l")) {
        Move moves[MAX_MOVES];
        int num_moves = generate_moves(board, moves);
        for (int i = 0; i < num_moves; i++) {
          printf("%d: %s\n", i + 1, move_to_string(moves[i]));
        }

      } else if (or_strcmp(arg, 2, "make", "m")) {
        Move moves[MAX_MOVES];
        int num_moves = generate_moves(board, moves);
        char *endptr;

        char *move_index_str = strtok_r(NULL, " ", &saveptr);
        long index = strtol(move_index_str, &endptr, 10);
        if (*endptr != '\0' || index < 1 || index > num_moves) {
          printf("Error: Invalid move index\n");
        } else {
          printf("Making move %ld: %s\n", index,
                 move_to_string(moves[index - 1]));
          board_make_move(board, moves[index - 1]);
        }

      } else if (or_strcmp(arg, 2, "unmake", "u")) {
        if (MoveHistory_is_empty(&move_history)) {
          printf("No moves to unmake\n");
          continue;
        } else {
          Move last_move = MoveHistory_pop(&move_history);
          board_unmake_move(board, last_move);
          printf("Unmade last move\n");
        }
      } else {
        printf("Unknown move command: '%s'\n", arg);
      }
    }

    else if (or_strcmp(command, 1, "print")) {
      char *board_str = board_to_string(board);
      printf("%s\n", board_str);
      free(board_str);
    }

    else {
      printf("Unknown command: '%s'\n", command);
    }
  }

  free(board);
  return exit_code;
}
