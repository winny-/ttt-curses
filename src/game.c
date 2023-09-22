#include "game.h"

#include <stdlib.h>
#include <err.h>
#include <string.h>

ttt_game* ttt_game_alloc(size_t rows, size_t columns) {
	ttt_game* g = malloc(sizeof(ttt_game));
        if (g == NULL) {
		return NULL;
	}
	g->board = calloc(rows * columns, sizeof(ttt_cell));
	if (g->board == NULL) {
		free(g);
		return NULL;
	}
	g->state = TTT_TURN_X;
	g->columns = columns;
	g->rows = rows;
	return g;
}

void ttt_game_free(ttt_game* game) {
	free(game->board);
	free(game);
}

#define BOARD_OFFSET(row, column) (game->board[row*game->rows + column])

ttt_cell ttt_get_cell(ttt_game* game, size_t row, size_t column) {
	return BOARD_OFFSET(row, column);
}

/* Unchecked routine to set a cell's value. */
void ttt_set_cell(ttt_game* game, size_t row, size_t column, ttt_cell value) {
	BOARD_OFFSET(row, column) = value;
}

char ttt_cell2ch(ttt_cell c) {
	switch (c) {
	case TTT_EMPTY:
		return ' ';
	case TTT_O:
		return 'O';
	case TTT_X:
		return 'X';
	default:
		warnx("Unknown cell type %d", c);
		abort();
	}
}

void ttt_randomize_board(ttt_game* game) {
	for (size_t row = 0; row < game->rows; row++) {
		for (size_t column = 0; column < game->columns; column++) {
			ttt_cell c = TTT_EMPTY;
			switch (rand() % 3) {
			case 0:
				c = TTT_O;
				break;
			case 1:
				c = TTT_X;
				break;
			}
			ttt_set_cell(game, row, column, c);
		}
	}
}

void ttt_reset(ttt_game* game) {
	for (size_t row = 0; row < game->rows; row++) {
		for (size_t column = 0; column < game->columns; column++) {
			ttt_set_cell(game, row, column, TTT_EMPTY);
		}
	}
	game->state = TTT_TURN_X;
}

char* ttt_state2str(ttt_state st) {
	switch (st) {
	case TTT_TURN_X:
		return "X's turn.";
	case TTT_TURN_O:
		return "O's turn.";
	case TTT_WIN_X:
		return "X - you win!  Hooray!";
	case TTT_WIN_O:
		return "O - you win!  Huzzah!";
	case TTT_TIE:
		return "You tied.  Better luck next time!";
	default:
		warnx("Unknown ttt_state value %d\n", st);
		abort();
	}
}

bool ttt_valid_move(ttt_game* g, size_t row, size_t column) {
	switch (g->state) {
	case TTT_TURN_X:
	case TTT_TURN_O:
		break;
	default:
		return false;
	}
	if (ttt_get_cell(g, row, column) != TTT_EMPTY) {
		return false;
	}
	return true;
}

