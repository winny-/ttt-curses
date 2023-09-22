#include "game.h"

#include <stdlib.h>
#include <err.h>
#include <string.h>

bool any_empty(ttt_game*);

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


bool ttt_play(ttt_game* g, size_t row, size_t column) {
	if (!ttt_valid_move(g, row, column)) {
		return false;
	}
	ttt_set_cell(g, row, column, g->state == TTT_TURN_X ? TTT_X : TTT_O);
	ttt_score score = ttt_game_score(g);
	if (score.winner) {
		g->state = score.winner == TTT_X ? TTT_WIN_X : TTT_WIN_O;
	} else if (!any_empty(g)) {
		g->state = TTT_TIE;
	} else {
		g->state = g->state == TTT_TURN_X ? TTT_TURN_O : TTT_TURN_X;
	}

	return true;
}

bool any_empty(ttt_game* g) {
	for (size_t row = 0; row < g->rows; row++) {
		for (size_t column = 0; column < g->columns; column++) {
			if (ttt_get_cell(g, row, column) == TTT_EMPTY) {
				return true;
			}
		}
	}
	return false;
}

bool ttt_can_play(ttt_game* g) {
	switch (g->state) {
	case TTT_TURN_X:
	case TTT_TURN_O:
		break;
	default:
		return false;
	}
	ttt_score score = ttt_game_score(g);
	if (score.winner != TTT_EMPTY) {
		return false;
	}
	return any_empty(g);
}

// FIXME could this find "wins" in a sequence of TTT_EMPTY?
ttt_score ttt_game_score(ttt_game* g) {
	ttt_score score = { .winner = TTT_EMPTY };
	// Horizontals
	for (size_t row = 0; row < g->rows; row++) {
		ttt_cell rcell = ttt_get_cell(g, row, 0);
		for (size_t column = 1; column < g->columns; column++) {
			if (rcell != ttt_get_cell(g, row, column)) {
				goto next_row;
			}
		}
		score.winner = rcell;
		score.start_row = score.end_row = row;
		score.start_column = 0;
		score.end_column = g->columns - 1;
		return score;
	next_row:
	}
	// Verticals
	for (size_t column = 0; column < g->columns; column++) {
		ttt_cell ccell = ttt_get_cell(g, 0, column);
		for (size_t row = 1; row < g->rows; row++) {
			if (ccell != ttt_get_cell(g, row, column)) {
				goto next_column;
			}
		}
		score.winner = ccell;
		score.start_column = score.end_column = column;
		score.start_row = 0;
		score.end_row = g->rows - 1;
		return score;
	next_column:
	}
	// No diagonals on non-square board.
	if (g->rows != g->columns) {
		return score;
	}
	ttt_cell majorcell = ttt_get_cell(g, 0, 0);
	for (size_t i = 1; i < g->rows; i++) {
		if (majorcell != ttt_get_cell(g, i, i)) {
			goto test_minor;
		}
	}
	score.winner = majorcell;
	score.start_row = score.start_column = 0;
	score.end_row = score.end_column = g->rows - 1;
	return score;

test_minor:
	ttt_cell minorcell = ttt_get_cell(g, 0, g->columns - 1);
	for (size_t i = 0; i < g->rows - 1; i++) {
		if (minorcell != ttt_get_cell(g, i, g->columns - i - 1)) {
			return score;
		}
	}
	score.winner = minorcell;
	score.start_row = g->rows - 1;
	score.start_column = score.end_row = 0;
	score.end_column = g->columns - 1;
	return score;
}
