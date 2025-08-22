g++ -fwrapv -fno-strict-aliasing -g -O3 -std=c++17 main.cpp -lSDL2 -o cmodel.elf
emcc -fwrapv -fno-strict-aliasing -O3 -std=c++17 main.cpp -lSDL2 -o cmodel.html --preload-file roto.bin -sUSE_SDL=2
# g++ -g -O3 gen.cpp -o gen.elf
