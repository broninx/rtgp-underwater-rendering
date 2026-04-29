
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <iostream>
#include <glad/glad.h>
#include <string>
#include <vector>
#include <utils/util_func.h>
#include "utils/texture.h"

using namespace std;


//should be called once to load the texture
void Texture::Load()
{
    int w, h, channels;
    unsigned char* image;

    stbi_set_flip_vertically_on_load(0); // do not flip the image loaded, because for the cube map we need a specific orientation of the images
    image = stbi_load(m_fileName.c_str(), &w, &h, &channels, 0);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &m_obj);
    glBindTexture(m_target, m_obj);
    if(m_target == GL_TEXTURE_2D)
    {
        if (channels==3)
            glTexImage2D(m_target, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels==4)
            glTexImage2D(m_target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
        // 3 channels = RGB ; 4 channel = RGBA
        
        glGenerateMipmap(m_target);
        // we set how to consider UVs outside [0,1] range
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // we set the filtering for minification and magnification
        glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // we free the memory once we have created an OpenGL texture
        stbi_image_free(image);
    } 

    // we set the binding to 0 once we have finished
    glBindTexture(m_target, 0);
}

//must be called  at least once for the specific texture unit
void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_target, m_obj);
}
