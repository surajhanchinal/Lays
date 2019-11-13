#g++ -g parser.cpp -o Out
g++ -g textureManager.cpp Run.cpp libs/libfreetype.so libs/libIrrKlang.so  -I. -pthread -lGL -lGLEW -lGLU -lglut -lfreeimage -lassimp -o test