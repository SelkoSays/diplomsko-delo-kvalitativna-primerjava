#!/usr/bin/env bash
set -e

CC=clang
STD="-std=c17"
FLAGS="-Wall -Wextra -Wpedantic"
LIBS=" -lpthread"

SRCS=(
    src/main.c
    src/tui/terminal.c
    src/tui/screen.c
    src/input/input.c
    src/ui/frame.c
    src/ui/grid.c
    src/ui/menu.c
    src/ui/panel.c
    src/game/entity.c
    src/game/game.c
    src/game/level.c
)

cd "$(dirname "$0")"

mkdir -p ./build

$CC $STD $FLAGS -I src "${SRCS[@]}" -o ./build/game-c $LIBS "$@"
