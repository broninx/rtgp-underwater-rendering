
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <string.h>
#include <utils/fileLoad.h>

#include "terrain.h"

Terrain::~Terrain()
{
    Destroy();
}


void Terrain::Destroy()
{
    m_heightMap.Destroy();
    m_geomipGrid.Destroy();
}


void Terrain::Init(float WorldScale){
    m_worldScale = WorldScale;
}

void Terrain::LoadFromFile(const char* pFilename){
    LoadHeightMapFile(pFilename);
    m_geomipGrid.CreateGeomipGrid(m_terrainSize, m_terrainSize, m_patchSize, this);
}

void Terrain::LoadHeightMapFile(const char* pFilename)
{
    int FileSize = 0;
    unsigned char* p = (unsigned char*)ReadBinaryFile(pFilename, FileSize);

    if (FileSize % sizeof(float) != 0) {
        printf("%s:%d - '%s' does not contain an whole number of floats (size %d)\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_terrainSize = (int)sqrtf((float)FileSize / (float)sizeof(float));

    printf("Terrain size %d\n", m_terrainSize);

    if ((m_terrainSize * m_terrainSize) != (FileSize / sizeof(float))) {
        printf("%s:%d - '%s' does not contain a square height map - size %d\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_heightMap.InitArray2D(m_terrainSize, m_terrainSize, (float*)p);
}

float Terrain::GetWorldHeight(float x, float z)
{
    float HeightMapX = x / m_worldScale;
    float HeightMapZ = z / m_worldScale;

    return GetHeightInterpolated(HeightMapX, HeightMapZ);
}

float Terrain::GetHeightInterpolated(float x, float z)
{
    float X0Z0Height = GetHeight((int)x, (int)z);

    if (((int)x + 1 >= m_terrainSize) ||  ((int)z + 1 >= m_terrainSize)) {
        return X0Z0Height;
    }

    float X1Z0Height = GetHeight((int)x + 1, (int)z);
    float X0Z1Height = GetHeight((int)x, (int)z + 1);
    float X1Z1Height = GetHeight((int)x + 1, (int)z + 1);

    float FactorX = x - floorf(x);

    float InterpolatedBottom = (X1Z0Height - X0Z0Height) * FactorX + X0Z0Height;
    float InterpolatedTop    = (X1Z1Height - X0Z1Height) * FactorX + X0Z1Height;

    float FactorZ = z - floorf(z);

    float FinalHeight = (InterpolatedTop - InterpolatedBottom) * FactorZ + InterpolatedBottom;

    return FinalHeight;
}
void Terrain::Finalize() 
{
    m_geomipGrid.CreateGeomipGrid(m_terrainSize, m_terrainSize, m_patchSize, this);
}

void Terrain::Render(const glm::vec3& CameraPos)
{
    m_geomipGrid.Render(CameraPos);
}
