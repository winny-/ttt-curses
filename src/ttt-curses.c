#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <curses.h>
#include <limits.h>
#include <stdlib.h>

#include "log.h"
#include "game.h"

#define MAX(x,y) (x > y ? x : y)
#define MIN(x,y) (x < y ? x : y)
#define LOG_PATH "LOG"

ttt_game *game;
int last_action;
size_t focus_row, focus_column;

void draw() {
	clear();
	#define TITLE "Tic Tac Toe"
	mvaddstr(0, 0, TITLE);
	char* status = ttt_state2str(game->state);
	int mycols = COLS-strlen(status);
	mvaddstr(0, mycols < 0 ? 0 : mycols, status);

#define MARGIN 2  // Space around the board
#define CELL_PADDING 1 // Interior space around the cell's X/O/' '
#define CELL_SIZE (CELL_PADDING*2 + 1)  // Size of a cell minus its border
	for (size_t row = 0; row < game->rows; row++) {
		if (row != 0) {
			mvhline(MARGIN + CELL_SIZE * (row) + (row-1), MARGIN, '-', CELL_SIZE*game->rows + game->rows-1);
		}
		for (size_t column = 0; column < game->columns; column++) {
			if (row == 0 && column != 0) {
				mvvline(MARGIN, MARGIN + CELL_SIZE*column + (column-1), '|', CELL_SIZE*game->columns + game->columns-1);
			}
			mvaddch(
				MARGIN + row*CELL_SIZE + row + CELL_PADDING,
				MARGIN + column*CELL_SIZE + column + CELL_PADDING,
			    ttt_cell2ch(ttt_get_cell(game, row, column))
			    );
		}
	}
	mvaddch(MARGIN + focus_row*CELL_SIZE + focus_row + CELL_PADDING, MARGIN + focus_column*CELL_SIZE + focus_column + CELL_PADDING - 1, '[');
	mvaddch(MARGIN + focus_row*CELL_SIZE + focus_row + CELL_PADDING, MARGIN + focus_column*CELL_SIZE + focus_column + CELL_PADDING + 1, ']');
	if (last_action == 'r') {
		mvaddstr(LINES-2, 0, "(Randomized.)");
	} else if (last_action == 'c') {
		mvaddstr(LINES-2, 0, "(Cleared.)");
	}
	mvaddstr(LINES-1, 0, "Press q exit.  Press r to randomize board.  Press c to clear board.");
	refresh();
}

int main(int argc, char **argv) {
	int errors = 0;
	int v;
	setlocale(LC_ALL, "");
	srand(time(0));
	log_set_quiet(true);
	FILE* logfp = fopen("LOG", "a+");
	if (logfp) {
		log_add_fp(logfp, TTT_DEFAULT_LOG_LEVEL);
		char* rp = realpath(LOG_PATH, NULL);
		log_info("Logging to \"%s\"", rp ? rp : LOG_PATH);
		if (rp) {
			free(rp);
		}
	} else {
		errx(errno, "Could not open log file");
	}

	initscr();
	start_color();
	use_default_colors();
	game = ttt_game_alloc(TTT_DEFAULT_ROWS, TTT_DEFAULT_COLUMNS);

	cbreak(); // Turn off line buffering.
	noecho(); // Turn off key echoing.
	intrflush(stdscr, TRUE); // Prevent interrupt from messing up terminal.
	keypad(stdscr, TRUE); // Parse key sequences for us.

	log_info("Init completed.  Ncurses reports terminal dimensions of %dx%d (COLSxLINES)", COLS, LINES);

#define LOG_POS() log_debug("focused row: %d column: %d", focus_row, focus_column)
	for (;;) {
		draw();
		switch (last_action = getch()) {
		case ERR:
			break; // Continues the main loop.
		case 'q':
			goto cleanup;  // Exits the main loop.
		case 'r':
			ttt_randomize_board(game);
			break;
		case KEY_UP:
			focus_row = MAX(0, focus_row-1);
			LOG_POS();
			break;
		case KEY_DOWN:
			focus_row = MIN(game->rows - 1, focus_row+1);
			LOG_POS();
			break;
		case KEY_RIGHT:
			focus_column = MIN(game->columns - 1, focus_column+1);
			LOG_POS();
			break;
		case KEY_LEFT:
			focus_column = MAX(0, focus_column-1);
			LOG_POS();
			break;
		case 'c':
			ttt_reset(game);
			break;
		case ' ':
			ttt_play(game, focus_row, focus_column);
			break;
		}
	}

cleanup:
	if ((v = endwin()) != OK) {
		errors++;
		warnx("Call to endwin() failed with value %d\n", v);
	}
	if (logfp) {
		fclose(logfp);
		logfp = NULL;
	}
	ttt_game_free(game);
	return errors;
}
