#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <limits.h>
#include <stdbool.h>

#include <curses.h>

#include "log.h"
#include "game.h"
#include "highscores.h"

#define MAX(x,y) (x > y ? x : y)
#define MIN(x,y) (x < y ? x : y)
#define LOG_PATH "LOG"

ttt_game *game;
int last_action;
size_t focus_row, focus_column;
bool highscores_enabled;

void draw() {
	clear();
	#define TITLE "Tic Tac Toe"
	mvaddstr(0, 0, TITLE);
	char* status = ttt_state2str(game->state);
	int mycols = COLS-strlen(status);
	mvaddstr(0, MAX(mycols, 0), status);
	#define TBUFSZ 200
	char turn[TBUFSZ];
	turn[0] = '\0';
	snprintf(turn, TBUFSZ, "Turns: %d", game->turn);
	mycols = COLS-strlen(turn);
	mvaddstr(1, MAX(mycols, 0), turn);

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
	if (last_action == 'n') {
		mvaddstr(LINES-2, 0, "(New game started.)");
	}
	mvaddstr(LINES-1, 0, "Press q exit.  Press n for a new game.  Press s to view scores.");
	refresh();
}

void show_score() {
	WINDOW* w = newwin(20, 60, 20, 20);
	box(w, '|', '-');
	mvwaddstr(w, 1, 2, "*** High scores ***");
#define ENTRIES 5
	if (!highscores_iter_begin(ENTRIES)) {
		log_warn("Unable to start high scores iterator");
		mvwaddstr(w, 4, 2, "(Unable to read high scores.)");
		goto pressanykeytoclose;
	}
	for (size_t i = 0; i < ENTRIES; i++) {
		ttt_score* score = highscores_iter_next();
		if (!score) {
			break;
		}
		mvwprintw(w, i+4, 2, "%2d %c %d,%d,%d", score->turns, ttt_cell2ch(score->winner), score->m, score->n, score->k);

	}
pressanykeytoclose:
	mvwaddstr(w, 18, 2, "Press any key to close.");
	wrefresh(w);
	getch();
	delwin(w);
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
	highscores_enabled = highscores_init();

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
		case 'q':
			goto cleanup;  // Exits the main loop.
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
		case 'n':
			ttt_reset(game);
			break;
		case 's':
			show_score();
			break;
		case ' ':
			ttt_state prevstate = game->state;
			ttt_play(game, focus_row, focus_column);
			if (prevstate != game->state && highscores_enabled) {
				switch (game->state) {
				case TTT_TIE:
				case TTT_WIN_X:
				case TTT_WIN_O:
					ttt_score score = ttt_game_score(game);
					highscores_record(game, &score);
					break;
				default:
					break;
				}
			}
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
	highscores_cleanup();
	return errors;
}
