#!/usr/bin/env bash

CXX=clang++
STD="-std=c++17"
FLAGS="-Wall -Wextra -Wpedantic"
FLAGS="$FLAGS -Wno-unused-parameter"
DBG_FLAGS="-ggdb"

# LIBS="-lpthread"
LIBS=

SRCS=(
    src/main.cpp
    src/tui/terminal.cpp
    src/tui/screen.cpp
    src/input/input.cpp
    src/ui/frame.cpp
    src/ui/grid.cpp
    src/ui/menu.cpp
    src/ui/panel.cpp
    src/game/entity.cpp
    src/game/game.cpp
    src/game/level.cpp
)

mkdir -p build
$CXX $STD $FLAGS $DBG_FLAGS -I src "${SRCS[@]}" -o build/game-cpp $LIBS "$@"
