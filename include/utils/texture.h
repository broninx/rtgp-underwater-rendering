#pragma once


#include <glad/glad.h>
#include <string>


class Texture
{
    private: 
        std::string m_fileName;
        GLenum m_target;
        GLuint m_obj;

    public:
        Texture(GLenum TextureTarget, const std::string& FileName)
        {
            m_target = TextureTarget;
            m_fileName = FileName;
        }

        
        void Load();

        void Bind(GLenum TextureUnit);
};