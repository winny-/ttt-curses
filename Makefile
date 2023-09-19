build/ttt-curses: src/* build
	ninja -C build

build: src/meson.build
	meson setup build src

run: build/ttt-curses
	./build/ttt-curses

clean:
	rm -rf build

.PHONY: clean run
