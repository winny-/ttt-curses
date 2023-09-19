#!/usr/bin/env bash
set -eu -o pipefail

if [[ $# -eq 0 ]]; then
	command=build
else
	command="$1"
fi

cd "${0%%/*}"

case $command in
build)
  meson setup build src
  ninja -C build
  ;;
run)
  ./compile.bash build
  ./build/ttt-curses
  ;;
clean)
  rm -rf build
esac
