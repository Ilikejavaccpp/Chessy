#/usr/bin/env bash

# SmolInt driver...
# Not necessary if you want the `.hpp` version
clang -std=c11 -O3 -Iinclude/backend -c include/backend/smolInt.c -o bin/smolInt_driver.o
echo "[INFO] : VENDOR -> `smolInt`: Finished compiling the library `smolInt` -> SUCCESS"
echo "[INFO] : Waiting to compile chessy... -> WAIT"
sleep 1.4

clang -std=c11 -O2 -Iinclude/backend -c include/backend/file_io.c -o bin/chessy_fiob_driver.o
clang -std=c11 -O2 -Iinclude -Ilib/clay -c src/clay_impl.c -o bin/chessy_clayb_driver.o

clang++ -std=c++23 -Ilib/raylib/src -Iinclude -Llib/raylib/src -Llib bin/chessy_clayb_driver.o bin/chessy_fiob_driver.o bin/smolInt_driver.o src/main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o bin/chessy -Wno-c++20-designator -Wno-initializer-overrides -Ilib/clay -fsanitize=address,leak,undefined 
