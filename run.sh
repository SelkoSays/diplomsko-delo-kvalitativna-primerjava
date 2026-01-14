#!/usr/bin/env sh

check_and_build_c() {
    if [ ! -f "./c/build/game-c" ]; then
        echo "C verzija ni prevedena. Prevajam..."
        "./build.sh" c
        echo ""
    fi
}

check_and_build_cpp() {
    if [ ! -f "./cpp/build/game-cpp" ]; then
        echo "C++ verzija ni prevedena. Prevajam..."
        "./build.sh" cpp
        echo ""
    fi
}

check_and_build_rust() {
    if [ ! -f "./rust/target/release/game-rs" ]; then
        echo "Rust verzija ni prevedena. Prevajam..."
        "./build.sh" rust
        echo ""
    fi
}

check_and_build_java() {
    if [ ! -d "./java/bin" ] || [ -z "$(ls -A "./java/bin" 2>/dev/null)" ]; then
        echo "Java verzija ni prevedena. Prevajam..."
        "./build.sh" java
        echo ""
    fi
}

run_c() {
    check_and_build_c
    "./c/build/game-c"
}

run_cpp() {
    check_and_build_cpp
    "./cpp/build/game-cpp"
}

run_rust() {
    check_and_build_rust
    "./rust/target/release/game-rs"
}

run_java() {
    check_and_build_java
    cd "./java"
    java --module-path lib/jna:bin -m si/si.Main
}

usage() {
    echo "Uporaba: $0 <jezik>"
    echo ""
    echo "Jeziki:"
    echo "  c      Zaženi C verzijo"
    echo "  cpp    Zaženi C++ verzijo"
    echo "  rust   Zaženi Rust verzijo"
    echo "  java   Zaženi Java verzijo"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

case "$1" in
    c)
        run_c
        ;;
    cpp|c++)
        run_cpp
        ;;
    rust|rs)
        run_rust
        ;;
    java)
        run_java
        ;;
    *)
        echo "Neznan jezik: $1"
        usage
        ;;
esac
