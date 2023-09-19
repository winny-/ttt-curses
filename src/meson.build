project('ttt-curses', 'c')
ncurses = dependency('ncurses')
src = [
	'ttt-curses.c',
	'game.c',
	'thirdparty/log.c/src/log.c',
]
incdir = include_directories('thirdparty/log.c/src')
executable('ttt-curses', src, dependencies : ncurses, include_directories : incdir)
