#include "textureManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;


void TextureManager::LoadTexture(string name,string path){
GLuint texID;
glGenTextures( 1, &texID );  // Get the texture object IDs.
    void* imgData;  // Pointer to image color data read from the file.
    int imgWidth;   // The width of the image that was read.
    int imgHeight;  // The height.
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(path.c_str());
    if (format == FIF_UNKNOWN) {
        cout<<"Unknown file type for texture image file "<<path<<endl;
        return;
    }
    FIBITMAP* bitmap = FreeImage_Load(format, path.c_str(), 0);  // Read image from file.
    if (!bitmap) {
    cout<<"Failed to load image "<<path<<endl;
        return;
    }
    FIBITMAP* bitmap2 = FreeImage_ConvertTo24Bits(bitmap);  // Convert to RGB or BGR format
    FreeImage_Unload(bitmap);
    imgData = FreeImage_GetBits(bitmap2);     // Grab the data we need from the bitmap.
    imgWidth = FreeImage_GetWidth(bitmap2);
    imgHeight = FreeImage_GetHeight(bitmap2);
    if (imgData) {
        cout<<"Texture "<<name<<" loaded from file"<<path<<", size "<<imgWidth<<"x"<<imgHeight<<endl;
        int format; // The format of the color data in memory, depends on platform.
        if ( FI_RGBA_RED == 0 )
            format = GL_RGB;
        else
            format = GL_BGR;
        glBindTexture( GL_TEXTURE_2D, texID );  // Will load image data into texture object #i
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, format,GL_UNSIGNED_BYTE, imgData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR	); // Required since there are no mipmaps.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR	);

        TextureManager::texture_lookup.insert(pair<string,int>(name,texID));
    }
    else {
        cout<<"Failed to get texture data from "<<path<<endl;
    }
}


unsigned int TextureManager::TextureFromFile(string path, string name)
{
    string filename = path;
    //filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        cout<<"nrComponents:  "<<nrComponents<<endl;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    TextureManager::texture_lookup.insert(pair<string,int>(name,textureID));
    cout<<"bullet tex ID:  "<<textureID<<endl;
    return textureID;
}


void TextureManager::loadCharacters(){

    FT_Library ft;

if(FT_Init_FreeType(&ft)) {
  fprintf(stderr, "Could not init freetype library\n");
  return;
}


FT_Face face;

if(FT_New_Face(ft, "./fonts/arial.ttf", 0, &face)) {
  fprintf(stderr, "Could not open font\n");
  return;
}

FT_Set_Char_Size(face,0,16*64,300,300 );


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
  
for (GLubyte c = 0; c < 128; c++)
{   
    // Load character glyph 
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        cout<<FT_Load_Char(face, c, FT_LOAD_RENDER)<<endl;
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        continue;
    }
    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use
    //cout<<"c: "<<c<<"  "<<texture<<endl;
    Character character = {
        texture, 
        Eigen::Vector2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        Eigen::Vector2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x
    };
    TextureManager::Characters.insert(std::pair<GLchar, Character>(c, character));
}
}


GLuint TextureManager::getTextureID(string name){
        return TextureManager::texture_lookup[name];
}



