#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <map>
#include <string>
#include "FreeImage.h"

using namespace std;
class TextureManager{
    public:
    TextureManager(){

    }
    void LoadTexture(string name,string path);
    GLuint getTextureID(string name); 
    unsigned int TextureFromFile(string path, string name);

    private:
        map<string, int> texture_lookup;

};