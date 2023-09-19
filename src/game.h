#ifndef _TTT_HAVE_GAME_H
#define _TTT_HAVE_GAME_H 1

#include <stddef.h>

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
	ttt_cell* board;
	ttt_state state;
	size_t rows, columns;
} ttt_game;

ttt_game* ttt_game_alloc(size_t, size_t);
void ttt_game_free(ttt_game*);


ttt_cell ttt_get_cell(ttt_game*, size_t, size_t);
void ttt_set_cell(ttt_game*, size_t, size_t, ttt_cell);
char ttt_cell2ch(ttt_cell);
void ttt_randomize_board(ttt_game*);
void ttt_reset(ttt_game*);

char* ttt_state2str(ttt_state);

#endif
