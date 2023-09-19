#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <curses.h>

#include "log.h"
#include "game.h"

ttt_game *game;
char last_action;

void draw() {
	clear();
	#define TITLE "Tic Tac Toe"
	mvaddstr(0, 0, TITLE);
	char* status = ttt_state2str(game->state);
	int mycols = COLS-strlen(status);
	mvaddstr(0, mycols < 0 ? 0 : mycols, status);
	
	#define PADDING 2
	#define SCALE 2
	for (size_t row = 0; row < game->rows; row++) {
		for (size_t column = 0; column < game->columns; column++) {
			mvaddch(
			    row*SCALE + PADDING,
			    column*SCALE + PADDING,
			    ttt_cell2ch(ttt_get_cell(game, row, column))
			    );
		}
	}

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
	initscr();
	start_color();
	use_default_colors();
	game = ttt_game_alloc(TTT_DEFAULT_ROWS, TTT_DEFAULT_COLUMNS);

	cbreak(); // Turn off line buffering.
	noecho(); // Turn off key echoing.
	intrflush(stdscr, TRUE); // Prevent interrupt from messing up terminal.
	keypad(stdscr, TRUE); // Parse key sequences for us.


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
		case 'c':
			ttt_reset(game);
			break;
		}
	}

cleanup:
	if ((v = endwin()) != OK) {
		errors++;
		warnx("Call to endwin() failed with value %d\n", v);
	}
	ttt_game_free(game);
	return errors;
}