#!/usr/bin/env bash

build_c() {
    echo "Prevajam C..."
    pushd "c" &> /dev/null
    ./build.sh
    popd &> /dev/null
    echo "C preveden: c/build/game-c"
}

build_cpp() {
    echo "Prevajam C++..."
    pushd "cpp" &> /dev/null
    ./build.sh
    popd &> /dev/null
    echo "C++ preveden: cpp/build/game-cpp"
}

build_rust() {
    echo "Prevajam Rust..."
    pushd "rust" &> /dev/null
    cargo build --release
    popd &> /dev/null
    echo "Rust preveden: rust/target/release/game-rs"
}

build_java() {
    echo "Prevajam Java..."
    pushd "java" &> /dev/null
    ./build.sh
    popd &> /dev/null
    echo "Java prevedena: java/bin/"
}

build_all() {
    build_c
    build_cpp
    build_rust
    build_java
    echo ""
    echo "Vsi programi prevedeni."
}

usage() {
    echo "Uporaba: $0 <jezik>"
    echo ""
    echo "Jeziki:"
    echo "  c      Prevedi C verzijo"
    echo "  cpp    Prevedi C++ verzijo"
    echo "  rust   Prevedi Rust verzijo"
    echo "  java   Prevedi Java verzijo"
    echo "  all    Prevedi vse verzije"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

case "$1" in
    c)
        build_c
        ;;
    cpp|c++)
        build_cpp
        ;;
    rust|rs)
        build_rust
        ;;
    java)
        build_java
        ;;
    all)
        build_all
        ;;
    *)
        echo "Neznan jezik: $1"
        usage
        ;;
esac
