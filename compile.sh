#g++ -g parser.cpp -o Out
g++ -g textureManager.cpp Run.cpp libs/libIrrKlang.so -pthread -lGL -lGLEW -lGLU -lglut -lfreeimage -lassimp -o test