#ifndef _TTT_HIGHSCORES_H
#define _TTT_HIGHSCORES_H
#include <stdbool.h>
#include "game.h"

bool highscores_init();
void highscores_cleanup();
void highscores_record(ttt_game*, ttt_score*);
ttt_score* highscores_iter_next();
bool highscores_iter_begin(size_t);

#endif
