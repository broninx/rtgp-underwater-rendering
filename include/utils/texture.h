#pragma once


#include <string>
#include "bitmap.h"

class Texture
{
    protected: 
        std::string m_fileName;
        GLenum m_target;
        GLuint m_obj;

    public:
        Texture() {}
        Texture(GLenum TextureTarget, const std::string& FileName) 
        {
            m_target = TextureTarget;
            m_fileName = FileName;
        }
    
        void Load();

        void Bind(GLenum TextureUnit);

};

