#pragma once
#include "Mesh3D.h"
#include <vector>
#include <string>
#include "Mesh3D.h"

class Scenegraph final
{
public:
	static Scenegraph* GetInstance();
	void ResetInstance();
	static Scenegraph* m_pInstance;
	//Functions
	std::vector<Vertex_Input> GetVertices() const;
	std::vector<uint32_t> GetIndices() const;
	void ReadOBJFile(const std::string& fileName);
private:
	Scenegraph() = default;
	//Datamembers
	std::vector<Vertex_Input> m_Vertices;
	std::vector<uint32_t> m_Indices;
};
