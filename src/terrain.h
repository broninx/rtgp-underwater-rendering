#pragma once

#include <glfw/glfw3.h>
#include <utils/camera.h>
#include <utils/vec_2d.h>
#include <utils/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "triangleList.h"

class Terrain
{
public:
	Terrain() {}

	void Init(float WorldScale);

	void Render();

	void LoadFromFile(const char* pFilename);

	float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }

	float GetWorldScale() const { return m_worldScale; }

protected:
	void LoadHeightMapFile(const char* pFilename);

	int m_terrainSize = 0;
	float m_worldScale = 1.0f;
	Array2D<float> m_heightMap;
	TriangleList m_triangleList;
};


