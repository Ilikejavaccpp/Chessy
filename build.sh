#/usr/bin/env bash
clang++ -std=c++17 -Ilib/raylib/src -Iinclude -Llib/raylib/src -Llib src/main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o bin/chessy
