#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <map>
#include <string>
#include "FreeImage.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

struct Character {
    GLuint     TextureID;  // ID handle of the glyph texture
    Eigen::Vector2i Size;       // Size of glyph
    Eigen::Vector2i Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
};

using namespace std;
class TextureManager{
    public:
    TextureManager(){

    }
    void LoadTexture(string name,string path);
    void loadCharacters();
    GLuint getTextureID(string name); 
    unsigned int TextureFromFile(string path, string name);
    std::map<GLchar, Character> Characters;

    private:
        map<string, int> texture_lookup;

};