#include <sqlite3.h>
#include <limits.h>
#include <stdlib.h>

#include <bsd/string.h>

#include "highscores.h"
#include "config.h"
#include "log.h"
#include "game.h"

sqlite3* db = NULL;
sqlite3_stmt* iter;
char path[PATH_MAX];

#define INITSQL ("CREATE TABLE IF NOT EXISTS scores (" \
		 "  date DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL," \
		 "  turns INT NOT NULL," \
                 "  tie INT DEFAULT FALSE NOT NULL," \
		 "  winner TEXT DEFAULT NULL," \
		 "  start_row INT DEFAULT NULL," \
		 "  end_row   INT DEFAULT NULL," \
		 "  start_column INT DEFAULT NULL," \
		 "  end_column INT DEFAULT NULL," \
		 "  m INT NOT NULL," \
		 "  n INT NOT NULL," \
		 "  k INT NOT NULL" \
		 ");")

#define INSERTSQL ("INSERT INTO scores " \
	     "(turns, tie, winner, start_row, end_row, start_column, end_column, m, n, k) VALUES " \
	     "(    ?,   ?,      ?,         ?,       ?,            ?,          ?, ?, ?, ?);")

#define SELECTSQL ("SELECT turns, tie, winner, start_row, end_row, start_column, end_column, m, n, k " \
	 "FROM scores " \
	 "ORDER BY rowid DESC " \
	 "LIMIT ?;")


bool highscores_iter_begin(size_t limit) {
	if (sqlite3_prepare_v2(db, SELECTSQL, strlen(SELECTSQL), &iter, NULL)) {
		log_error("Unable to initialize highscores iterator: %s", sqlite3_errmsg(db));
		return false;
	}
	if (sqlite3_bind_int(iter, 1, limit)) {
		log_error("Unable to initialize highscores iterator: %s", sqlite3_errmsg(db));
		return false;
	}
	return true;
}


ttt_score* row2ttt_score() {
	ttt_score* score = malloc(sizeof(ttt_score));
	score->turns = sqlite3_column_int(iter, 0);
	char* buf = sqlite3_column_text(iter, 2);
	score->winner = ttt_str2cell(buf);
	score->start_row = sqlite3_column_int(iter, 3);
	score->end_row = sqlite3_column_int(iter, 4);
	score->start_column = sqlite3_column_int(iter, 5);
	score->end_column = sqlite3_column_int(iter, 6);
	return score;
}


ttt_score* highscores_iter_next() {
	switch (sqlite3_step(iter)) {
	case SQLITE_ROW:
		return row2ttt_score();
	case SQLITE_DONE:
		return NULL;
	default:
		log_error("Unable to iterate on high scores: %s", sqlite3_errmsg(db));
		return NULL;
	}
	return NULL;
}


void highscores_record(ttt_game* game, ttt_score* score) {
	sqlite3_stmt *stmt;
	if (sqlite3_prepare_v2(db, INSERTSQL, strlen(INSERTSQL), &stmt, NULL)) {
		log_error("Unable to record highscore: %s", sqlite3_errmsg(db));
		return;
	}
	#define BIND_NULL(index) if (sqlite3_bind_null(stmt, index)) { log_error("Unable to record highscore: %s", sqlite3_errmsg(db)); return; }
	#define BIND(type, index, expr) if(sqlite3_bind_##type(stmt, index, expr)) { log_error("Unable to record highscore: %s", sqlite3_errmsg(db)); return; }
	#define BIND_S(index, expr) if(sqlite3_bind_text(stmt, index, expr, strlen(expr), SQLITE_STATIC)) { log_error("Unable to record highscore: %s", sqlite3_errmsg(db)); return; }
	// It appears the binding indicies start at 1, not 0.
	BIND(int, 1, score->turns);

	BIND(int, 2, score->winner ? 0 : 1);
	if (score->winner) {
		char buf[] = { ttt_cell2ch(score->winner), '\0' };
		BIND_S(3, buf);
		BIND(int, 4, score->start_row);
		BIND(int, 5, score->end_row);
		BIND(int, 6, score->start_column);
		BIND(int, 7, score->end_column);
	} else {
		for (size_t i = 3; i < 4; i++) {
			BIND_NULL(i);
		}
	}
	BIND(int, 8, game->rows);
	BIND(int, 9, game->columns);
	BIND(int, 10, game->rows);
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		log_error("Unable to record highscore: %s", sqlite3_errmsg(db));
		return;
	}
}


bool highscores_init() {
	iter = NULL;
	char* state_dir = get_state_dir();
	if (!state_dir) {
		log_warn("Could not determine a XDG state directory.  Disabling high scores.");
		return false;
	}
	strlcpy(path, state_dir, PATH_MAX);
	free(state_dir);
	strlcat(path, "/highscores.db", PATH_MAX);

	if (sqlite3_open(path, &db)) {
		sqlite3_close(db);
		log_error("Unable to open %s: %s", path, sqlite3_errmsg(db));
		return false;
	}
	sqlite3_stmt *stmt;
	if (sqlite3_prepare_v2(db, INITSQL, strlen(INITSQL), &stmt, NULL)) {
		log_error("Unable to create schema in %s: %s", path, sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		log_error("Unable to create schema in %s: %s (%d)", path, sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}
	return true;
}

void highscores_cleanup() {
	if (db) {
		if (sqlite3_close(db)) {
			log_error("Unable to close %s: %s", path, sqlite3_errmsg(db));
		}
	}
}
