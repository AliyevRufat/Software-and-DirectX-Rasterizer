#include "pch.h"
#include "Scenegraph.h"
#include <fstream>
#include <iostream>

Scenegraph* Scenegraph::m_pInstance{ nullptr };

void Scenegraph::ResetInstance()
{
	delete m_pInstance;
	m_pInstance = nullptr;
}

std::vector<Vertex_Input> Scenegraph::GetVertices() const
{
	return m_Vertices;
}

std::vector<uint32_t> Scenegraph::GetIndices() const
{
	return m_Indices;
}

void Scenegraph::ReadOBJFile(const std::string& fileName)
{
	//clear the lists before reading new obj file
	m_Vertices.clear();
	m_Indices.clear();
	//
	std::vector<Elite::FPoint4> vertexPos;
	std::vector<Elite::FVector2> vertexUv;
	std::vector<Elite::FVector3> vertexNormal;
	std::vector<uint32_t> iDList = {}; // id
	std::vector<uint32_t> iDListUV = {}; // id
	std::vector<uint32_t> iDListNormals = {}; // id
	//
	std::ifstream input;
	std::string oneLine;
	std::string catchString;
	std::vector<float> catchFloat;

	input.open(fileName, std::ios::in | std::ios::binary);
	if (input.is_open())
	{
		while (std::getline(input, oneLine))
		{
			if (oneLine[0] == 'v' && oneLine[1] == ' ') // reading positions
			{
				for (size_t i = 2; i < oneLine.size(); i++)
				{
					if (oneLine[i] == ' ')
					{
						for (size_t j = i + 1; j < oneLine.size(); j++)
						{
							while (j < oneLine.size() && oneLine[j] != ' ')
							{
								catchString += oneLine[j];
								j++;
							}
							catchFloat.push_back(std::stof(catchString));
							catchString.clear();
						}
						if (catchFloat.size() == 3)
						{
							vertexPos.push_back(Elite::FPoint3{ catchFloat[0],catchFloat[1],catchFloat[2] });
							catchFloat.clear();
						}
						break;
					}
				}
				catchString = ' ';
			}
			else if (oneLine[0] == 'v' && oneLine[1] == 't') // reading uvs
			{
				for (size_t i = 0; i < oneLine.size(); i++)
				{
					if (oneLine[i] == '.')
					{
						for (size_t j = i - 1; j < i + 5; j++)
						{
							catchString += oneLine[j];
						}
						catchFloat.push_back(std::stof(catchString));
						catchString = ' ';

						if (catchFloat.size() == 3)
						{
							vertexUv.push_back(Elite::FVector2{ catchFloat[0],catchFloat[1] });//,catchFloat[2] });
							catchFloat.clear();
						}
					}
				}
				catchString = ' ';
			}
			else if (oneLine[0] == 'v' && oneLine[1] == 'n') // reading normals
			{
				for (size_t i = 2; i < oneLine.size(); i++)
				{
					if (oneLine[i] == ' ')
					{
						for (size_t j = i + 1; j < oneLine.size(); j++)
						{
							while (j < oneLine.size() && oneLine[j] != ' ')
							{
								catchString += oneLine[j];
								j++;
							}
							catchFloat.push_back(std::stof(catchString));
							catchString.clear();
						}
						if (catchFloat.size() == 3)
						{
							vertexNormal.push_back(Elite::FVector3{ catchFloat[0],catchFloat[1],catchFloat[2] });
							catchFloat.clear();
						}
						break;
					}
				}
				catchString = ' ';
			}
			if (oneLine[0] == 'f') // reading id's
			{
				for (size_t i = 1; i < oneLine.size(); i++)
				{
					std::string tempString;
					std::string tempString2;
					int nrPos = -1;
					int nrUv = -1;
					int nrNormal = -1;
					bool saveNormal = false;
					char tempChar = oneLine[i];
					//for pos
					if (std::isdigit(tempChar) && oneLine[i - 1] == ' ')
					{
						int j = 0;
						while (oneLine[i + j] != '/')
						{
							tempString += oneLine[i + j];
							j++;
						}
					}
					if (tempString != "")
					{
						nrPos = std::stoi(tempString);
						iDList.push_back(nrPos - 1);
					}
					//for uvs
					if (std::isdigit(tempChar) && oneLine[i - 1] == '/')
					{
						int k = 0;
						while (oneLine[i + k] != ' ')
						{
							if (oneLine[i + k] == '/')
							{
								int j = 0;
								while (oneLine[i + j] != '/')
								{
									tempString2 += oneLine[i + j];
									j++;
								}
							}
							k++;
						}
					}
					if (tempString2 != "")
					{
						nrUv = std::stoi(tempString2);
						iDListUV.push_back(nrUv - 1);
					}
					//for normals
					if (std::isdigit(tempChar) && oneLine[i - 1] == '/')
					{
						int k = 0;
						while (i + k < oneLine.size())
						{
							if (oneLine[i + k] == ' ')
							{
								saveNormal = true;
								break;
							}
							else if (oneLine[i + k] == '/')
							{
								saveNormal = false;
								break;
							}
							tempString2 += oneLine[i + k];
							k++;
						}
						if (!saveNormal)
						{
							tempString2.clear();
						}
					}
					if (tempString2 != "")
					{
						nrNormal = std::stoi(tempString2);
						iDListNormals.push_back(nrNormal - 1);
					}
				}
			}
		}
		input.close();
		//for normals
	}
	////duplicates optimization------------------------------------------------------------------------------------
	for (size_t i = 0; i < iDList.size(); i++)
	{
		Elite::FPoint4 pos = vertexPos[iDList[i]];
		Elite::FVector3 normal = vertexNormal[iDListNormals[i]];
		//by default do inversed and change back in renderer constructor for rasterizer
		//invert z value pos
		pos.z = -pos.z;
		//invert z value normal
		normal.z = -normal.z;

		Vertex_Input vertex = { Elite::FPoint3(pos)	,	Elite::RGBColor(0, 0, 0), Elite::FVector2(vertexUv[iDListUV[i]]), normal,Elite::FVector3(0,0,0) };

		auto it = std::find_if(m_Vertices.begin(), m_Vertices.end(),
			[&](const Vertex_Input& a) -> bool
		{
			return (
				vertex.Position == a.Position &&
				vertex.TexCoord == a.TexCoord
				);
		}
		);

		if (it != m_Vertices.end())
		{
			int index = std::distance(m_Vertices.begin(), it);
			m_Indices.push_back(index);
		}
		else
		{
			m_Vertices.push_back(vertex);
			m_Indices.push_back(m_Vertices.size() - 1);
		}
	}
	////Get the tangents----------------------------------------------------------------------------------------------
	for (uint32_t i = 0; i < m_Indices.size(); i += 3)
	{
		uint32_t index0 = m_Indices[i];
		uint32_t index1 = m_Indices[i + 1];
		uint32_t index2 = m_Indices[i + 2];
		const Elite::FPoint3& p0 = Elite::FPoint3(m_Vertices[index0].Position);
		const Elite::FPoint3& p1 = Elite::FPoint3(m_Vertices[index1].Position);
		const Elite::FPoint3& p2 = Elite::FPoint3(m_Vertices[index2].Position);
		const Elite::FVector2& uv0 = m_Vertices[index0].TexCoord;
		const Elite::FVector2& uv1 = m_Vertices[index1].TexCoord;
		const Elite::FVector2& uv2 = m_Vertices[index2].TexCoord;
		const Elite::FVector3 edge0 = p1 - p0;
		const Elite::FVector3 edge1 = p2 - p0;
		const Elite::FVector2 diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const Elite::FVector2 diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Cross(diffX, diffY);
		Elite::FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
		m_Vertices[index0].Tangent += tangent;
		m_Vertices[index1].Tangent += tangent;
		m_Vertices[index2].Tangent += tangent;
	}
	//Create the tangents (reject vector) + fix the tangents per vertex-------------------------------------------------
	for (size_t i = 0; i < m_Vertices.size(); i++)
	{
		m_Vertices[i].Tangent = (Elite::GetNormalized(Elite::Reject(m_Vertices[i].Tangent, m_Vertices[i].Normal)));
		//invert z value tangent
		m_Vertices[i].Tangent.z = -m_Vertices[i].Tangent.z;
	}
}

Scenegraph* Scenegraph::GetInstance()
{
	if (!m_pInstance)
	{
		m_pInstance = new Scenegraph();
	}
	return m_pInstance;
}