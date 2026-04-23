/*

        Copyright 2014 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <iostream>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#ifndef _WIN64
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>


using namespace std;




#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0]) * a.size())

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifdef _WIN64
#define SNPRINTF _snprintf_s
#define VSNPRINTF vsnprintf_s
#define RANDOM rand
#define SRANDOM srand((unsigned)time(NULL))
#pragma warning (disable: 4566)
#else
#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf
#define RANDOM random
#define SRANDOM srandom(getpid())
#endif

#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_OGL_VALUE 0xffffffff

#define NUM_CUBE_MAP_FACES 6

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_CalcTangentSpace)


#define NOT_IMPLEMENTED printf("Not implemented case in %s:%d\n", __FILE__, __LINE__); exit(0);


#define MAX_BONES (200)

#define CLAMP(Val, Start, End) std::min(std::max((Val), (Start)), (End));

void OgldevError(const char* pFileName, uint line, const char* format, ...)
{
    char msg[1000];
    va_list args;
    va_start(args, format);
    VSNPRINTF(msg, sizeof(msg), format, args);
    va_end(args);

#ifdef _WIN32
    char msg2[1000];
    _snprintf_s(msg2, sizeof(msg2), "%s:%d: %s", pFileName, line, msg);
    MessageBoxA(NULL, msg2, NULL, 0);
#else
    fprintf(stderr, "%s:%d - %s", pFileName, line, msg);
#endif
}


void OgldevFileError(const char* pFileName, uint line, const char* pFileError)
{
#ifdef _WIN32
    char msg[1000];
    _snprintf_s(msg, sizeof(msg), "%s:%d: unable to open file `%s`", pFileName, line, pFileError);
    MessageBoxA(NULL, msg, NULL, 0);
#else
    fprintf(stderr, "%s:%d: unable to open file `%s`\n", pFileName, line, pFileError);
#endif
}

#define OGLDEV_ERROR0(msg) OgldevError(__FILE__, __LINE__, msg)
#define OGLDEV_ERROR(msg, ...) OgldevError(__FILE__, __LINE__, msg, __VA_ARGS__)
#define OGLDEV_FILE_ERROR(FileError) OgldevFileError(__FILE__, __LINE__, FileError);
bool ReadFile(const char* pFileName, string& outFile)
{
    ifstream f(pFileName);

    bool ret = false;

    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    }
    else {
        OGLDEV_FILE_ERROR(pFileName);
    }

    return ret;
}


#ifdef _WIN32
char* ReadBinaryFile(const char* pFilename, int& size)
{
    FILE* f = NULL;

    errno_t err = fopen_s(&f, pFilename, "rb");

    if (!f) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        OGLDEV_ERROR("Error opening '%s': %s\n", pFilename, buf);
        exit(0);
    }

    struct stat stat_buf;
    int error = stat(pFilename, &stat_buf);

    if (error) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        OGLDEV_ERROR("Error getting file stats: %s\n", buf);
        return NULL;
    }

    size = stat_buf.st_size;

    char* p = (char*)malloc(size);
    assert(p);

    size_t bytes_read = fread(p, 1, size, f);

    if (bytes_read != size) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        OGLDEV_ERROR("Read file error file: %s\n", buf);
        exit(0);
    }

    fclose(f);

    return p;
}

void WriteBinaryFile(const char* pFilename, const void* pData, int size)
{
    FILE* f = NULL;
    
    errno_t err = fopen_s(&f, pFilename, "wb"); 

    if (!f) {
        OGLDEV_ERROR("Error opening '%s'\n", pFilename);
        exit(0);
    }

    size_t bytes_written = fwrite(pData, 1, size, f);

    if (bytes_written != size) {
        OGLDEV_ERROR("Error write file\n");
        exit(0);
    }

    fclose(f);
}

#else
char* ReadBinaryFile(const char* pFilename, int& size)
{
    FILE* f = fopen(pFilename, "rb");

    if (!f) {
        OGLDEV_ERROR("Error opening '%s': %s\n", pFilename, strerror(errno));
        exit(0);
    }

    struct stat stat_buf;
    int error = stat(pFilename, &stat_buf);

    if (error) {
        OGLDEV_ERROR("Error getting file stats: %s\n", strerror(errno));
        return NULL;
    }

    size = stat_buf.st_size;

    char* p = (char*)malloc(size);
    assert(p);

    size_t bytes_read = fread(p, 1, size, f);

    if ((long uint)bytes_read != size) {
        OGLDEV_ERROR("Read file error file: %s\n", strerror(errno));
        exit(0);
    }

    fclose(f);

    return p;
}


void WriteBinaryFile(const char* pFilename, const void* pData, int size)
{
    FILE* f = fopen(pFilename, "wb");

    if (!f) {
        OGLDEV_ERROR("Error opening '%s': %s\n", pFilename, strerror(errno));
        exit(0);
    }

    int bytes_written = fwrite(pData, 1, size, f);

    if (bytes_written != size) {
        OGLDEV_ERROR("Error write file: %s\n", strerror(errno));
        exit(0);
    }

    fclose(f);

    /*    int f = open(pFilename, O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (f == -1) {
        OGLDEV_ERROR("Error opening '%s': %s\n", pFilename, strerror(errno));
        exit(0);
    }

    int write_len = write(f, pData, size);
    printf("%d\n", write_len);
    if (write_len != size) {
        OGLDEV_ERROR("Error write file: %s\n", strerror(errno));
        exit(0);
    }

    close(f);*/
}


#endif





long long GetCurrentTimeMillis()
{
#ifdef _WIN32
    return GetTickCount64();
#else
    timeval t;
    gettimeofday(&t, NULL);

    long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
    return ret;
#endif
}


string GetDirFromFilename(const string& Filename)
{
    // Extract the directory part from the file name
    string::size_type SlashIndex;

#ifdef _WIN64
    SlashIndex = Filename.find_last_of("\\");

    if (SlashIndex == -1) {
        SlashIndex = Filename.find_last_of("/");
    }
#else
    SlashIndex = Filename.find_last_of("/");
#endif

    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    return Dir;
}






