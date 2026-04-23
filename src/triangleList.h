#pragma once

#include <glad/glad.h>
#include <vector>
// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

class Terrain;

class TriangleList {
public:
	TriangleList();

	void CreateTriangleList(int Width, int Depth, const Terrain* pTerrain);

	void Render();

private:

	struct Vertex {
		glm::vec3 Pos;

		void InitVertex(const Terrain* pTerrain, int x, int z);
	};

	void CreateGLState();

	void PopulateBuffers(const Terrain* pTerrain);
	void InitVertices(const Terrain* pTerrain, std::vector<Vertex>& Vertices);
	void InitIndices(std::vector<uint>& Indices);

	int m_width = 0;
	int m_depth = 0;
	GLuint m_vao;
	GLuint m_vb;
	GLuint m_ib;
};