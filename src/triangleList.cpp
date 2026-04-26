#include <stdio.h>
#include <vector>

#include "triangleList.h"
#include "terrain.h"

using namespace glm;


TriangleList::TriangleList()
{
}


void TriangleList::CreateTriangleList(int Width, int Depth, const Terrain* pTerrain)
{
	m_width = Width;
	m_depth = Depth;

	CreateGLState();

	PopulateBuffers(pTerrain);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TriangleList::CreateGLState()
{
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vb);

	glBindBuffer(GL_ARRAY_BUFFER, m_vb);

	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

	int POS_LOC = 0;
	int TEX_LOC = 1;
	int NORMAL_LOC = 2;

	size_t NumFloats = 0;
	
	glEnableVertexAttribArray(POS_LOC);
	glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
	NumFloats += 3;

	glEnableVertexAttribArray(TEX_LOC);
	glVertexAttribPointer(TEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
	NumFloats += 2;

	glEnableVertexAttribArray(NORMAL_LOC);
	glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
	NumFloats += 3;
}

void TriangleList::PopulateBuffers(const Terrain* pTerrain)
{
	std::vector<Vertex> Vertices;
	Vertices.resize(m_width * m_depth);

	InitVertices(pTerrain, Vertices);

	std::vector<uint> Indices;
	int NumQuads = (m_width - 1) * (m_depth - 1);
	Indices.resize(NumQuads * 6);
	InitIndices(Indices);

	CalcNormals(Vertices, Indices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}



void TriangleList::Vertex::InitVertex(const Terrain* pTerrain, int x, int z)
{
	float y = pTerrain->GetHeight(x, z);

	float WorldScale = pTerrain->GetWorldScale();

	Pos = vec3(x * WorldScale, y, z * WorldScale);

	float Size = (float) pTerrain->GetSize();
	Tex = vec2(x / Size, y / Size);
}

void TriangleList::InitVertices(const Terrain* pTerrain, std::vector<Vertex>& Vertices)
{
	int Index = 0;

	for (int z = 0; z < m_depth; z++) {
		for (int x = 0; x < m_width; x++) {
			assert(Index < (int)Vertices.size());
			Vertices[Index].InitVertex(pTerrain, x, z);
			Index++;
		}
	}

    assert(Index == (int)Vertices.size());
}

void TriangleList::InitIndices(std::vector<uint>& Indices)
{
	int Index = 0;

	for (int z = 0; z < m_depth - 1; z++) {
		for (int x = 0; x < m_width - 1; x++) {
			uint IndexBottomLeft = z * m_width + x;
			uint IndexTopLeft = (z + 1) * m_width + x;
			uint IndexTopRight = (z + 1) * m_width + x + 1;
			uint IndexBottomRight = z * m_width + x + 1;

			// Add top left triangle
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexBottomLeft;
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexTopLeft;
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexTopRight;

			// Add bottom right triangle
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexBottomLeft;
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexTopRight;
            assert(Index < (int)Indices.size());
			Indices[Index++] = IndexBottomRight;
		}
	}

    assert(Index == (int)Indices.size());
}

void TriangleList::CalcNormals(std::vector<Vertex>& Vertices, std::vector<uint>& Indices)
{
	    // unsigned int Index = 0;

    // Accumulate each triangle normal into each of the triangle vertices
    for (unsigned int i = 0 ; i < Indices.size() ; i += 3) {
        unsigned int Index0 = Indices[i];
        unsigned int Index1 = Indices[i + 1];
        unsigned int Index2 = Indices[i + 2];
        glm::vec3 v1 = Vertices[Index1].Pos - Vertices[Index0].Pos;
        glm::vec3 v2 = Vertices[Index2].Pos - Vertices[Index0].Pos;
        glm::vec3 Normal = glm::cross(v1, v2);
        Normal = glm::normalize(Normal);

        Vertices[Index0].Normal += Normal;
        Vertices[Index1].Normal += Normal;
        Vertices[Index2].Normal += Normal;
    }

    // Normalize all the vertex normals
    for (unsigned int i = 0 ; i < Vertices.size() ; i++) {
        Vertices[i].Normal = glm::normalize(Vertices[i].Normal);
    }
}

void TriangleList::Render()
{
	glBindVertexArray(m_vao);

	glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
}

