#ifndef _TTT_HAVE_GAME_H
#define _TTT_HAVE_GAME_H 1

#include <stddef.h>
#include <stdbool.h>

#define TTT_DEFAULT_LOG_LEVEL LOG_TRACE

#define TTT_DEFAULT_ROWS 3
#define TTT_DEFAULT_COLUMNS 3

typedef enum {
	TTT_TURN_X,
	TTT_TURN_O,
	TTT_WIN_X,
	TTT_WIN_O,
	TTT_TIE
} ttt_state;

typedef enum {
	TTT_EMPTY = 0,
	TTT_O,
	TTT_X,
} ttt_cell;

typedef struct {
	size_t turn;
	ttt_cell* board;
	ttt_state state;
	size_t rows, columns;
} ttt_game;

/* FIXME this data feels like it overlaps with the above */
typedef struct {
	size_t turns;
	ttt_cell winner;
	// The following fields contain valid data when ttt_cell != TTT_EMPTY.
	size_t start_row, start_column, end_row, end_column;
} ttt_score;

/* TODO add a dedicated highscore data structure */

ttt_game* ttt_game_alloc(size_t, size_t);
void ttt_game_free(ttt_game*);

ttt_cell ttt_get_cell(ttt_game*, size_t, size_t);
void ttt_set_cell(ttt_game*, size_t, size_t, ttt_cell);
char ttt_cell2ch(ttt_cell);
ttt_cell ttt_str2cell(char*);
void ttt_reset(ttt_game*);

char* ttt_state2str(ttt_state);

bool ttt_valid_move(ttt_game*, size_t, size_t);
bool ttt_play(ttt_game*, size_t, size_t);
ttt_score ttt_game_score(ttt_game*);
bool ttt_can_play(ttt_game*);

#endif
