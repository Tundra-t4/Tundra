clang++ -v -std=c++20 -fpermissive -g ../polaroid/polaroid.o -o tundrac tundrac.cpp -I./include/ -I../polaroid/ -lLLVM -I./modules/HParser ./modules/HParser/Hparser.o  -L/usr/lib/llvm/lib \
-I /opt/homebrew/Cellar/llvm/21.1.7/include -L/opt/homebrew/Cellar/llvm/21.1.7/lib -g
