project('ttt-curses', 'c')
deps = [
	dependency('ncurses'),
	dependency('sqlite3'),
	dependency('libbsd'),
]
src = [
	'ttt-curses.c',
	'game.c',
	'config.c',
	'highscores.c',
	'thirdparty/log.c/src/log.c',
]
incdir = include_directories('thirdparty/log.c/src')
executable('ttt-curses', src, dependencies : deps, include_directories : incdir)
