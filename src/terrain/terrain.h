#pragma once

#include "geomip_grid.h"
#include <glfw/glfw3.h>
#include <utils/camera.h>
#include <utils/vec_2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/texture.h>



class Terrain
{
public:
	Terrain() { }

	~Terrain();

	void Destroy();

	void Init(float WorldScale);

	void Render(const glm::vec3& CameraPos);

	void LoadFromFile(const char* pFilename);

	float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }

	float GetWorldScale() const { return m_worldScale; }

    float GetMinHeight() const {return m_minHeight;}
    float GetMaxHeight() const {return m_maxHeight;}
	void SetMinMaxHeight(float MinHeight, float MaxHeight) {m_minHeight = MinHeight; m_maxHeight = MaxHeight;}
	int GetSize() const {return m_terrainSize;}
	void Finalize(); 

    protected:
	void LoadHeightMapFile(const char* pFilename);
	int m_terrainSize = 0;
	int m_patchSize = 0;
	float m_worldScale = 1.0f;
	Array2D<float> m_heightMap;
	GeomipGrid m_geomipGrid;
    float m_minHeight = 0.0f;
    float m_maxHeight = 0.0f;
};


