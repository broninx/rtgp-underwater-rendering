#pragma once

#include <string>
#include <iostream>
#include <glad/glad.h>

using namespace std;

// load image from disk and create an OpenGL texture, returning the texture ID
GLint LoadTextureCube(string path);

// load any other texture TODO: try to use this function also for cube map sides
GLint LoadTexture(const char* path);