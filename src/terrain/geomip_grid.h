#pragma once

#include <glad/glad.h>
#include <vector>
// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "lod_manager.h"

class Terrain;

class GeomipGrid {
public:
	GeomipGrid();

	~GeomipGrid();

	void Destroy();

	void CreateGeomipGrid(int Width, int Depth, int PatchSize, const Terrain* pTerrain);

	void Render(const glm::vec3& CameraPos);

private:

	struct Vertex {
		glm::vec3 Pos;
		glm::vec2 Tex;
		glm::vec3 Normal = glm::vec3(0.0f, 0.0f, 0.0f);
		void InitVertex(const Terrain* pTerrain, int x, int z);
	};

	void CreateGLState();

	void PopulateBuffers(const Terrain* pTerrain);
	void InitVertices(const Terrain* pTerrain, std::vector<Vertex>& Vertices);

	void InitIndices(std::vector<uint>& Indices);
	int InitIndicesLOD(int Index, std::vector<uint>& Indices, int lod);
    int InitIndicesLODSingle(int Index, std::vector<uint>& Indices, int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom);

	void CalcNormals(std::vector<Vertex>& Vertices, std::vector<uint>& Indices);

	uint AddTriangle(uint Index, std::vector<uint>& Indices, uint v1, uint v2, uint v3);

	uint CreateTriangleFan(int Index, std::vector<uint>& Indices, int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom, int x, int z);

	int CalcNumIndices();

	int m_width = 0;
	int m_depth = 0;
	int m_patchSize = 0;
	int m_maxLOD = 0;
	GLuint m_vao;
	GLuint m_vb;
	GLuint m_ib;

	struct SingleLodInfo {
        int Start = 0;
        int Count = 0;
    };
	
    #define LEFTLOD   2
    #define RIGHTLOD  2
    #define TOPLOD    2
    #define BOTTOMLOD 2
	
    struct LodInfo {
        SingleLodInfo info[LEFTLOD][RIGHTLOD][TOPLOD][BOTTOMLOD];
    };
	
    std::vector<LodInfo> m_lodInfo;
    int m_numPatchesX = 0;
    int m_numPatchesZ = 0;
    LodManager m_lodManager;
};