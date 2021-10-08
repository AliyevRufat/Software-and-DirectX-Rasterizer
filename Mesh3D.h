#pragma once
#include "Effect.h"
#include "EffectTransparency.h"
#include <vector>
#include "Texture.h"
#include "ERenderer.h"

struct Vertex_Input
{
	Elite::FPoint3 Position;
	Elite::RGBColor Color;
	Elite::FVector2 TexCoord;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
};

class Mesh3D final
{
public:
	enum class MeshType
	{
		vehicle,
		fireParticle
	};

	Mesh3D(SDL_Window* pWindow, ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexList, const std::vector<uint32_t>& indexList, MeshType meshType);
	//Rule of five
	~Mesh3D();
	Mesh3D(const Mesh3D&) = delete;
	Mesh3D(Mesh3D&&) noexcept = delete;
	Mesh3D& operator=(const Mesh3D&) = delete;
	Mesh3D& operator=(Mesh3D&&) noexcept = delete;

	//Functions
	void Render(ID3D11DeviceContext* pDeviceContext, const Elite::FMatrix4& worldViewProjMatrix,
		const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewInverseMatrix,
		Elite::Renderer::TextureFilteringTechnique textureFiltering, bool isTransparencyOn, Elite::Renderer::CullMode cullMode);

private:
	//Datamembers
	EffectTransparency* m_pEffectTransparency;
	Effect* m_pEffect;
	Texture* m_pTextureDiffuseFire;
	Texture* m_pTextureDiffuse;
	Texture* m_pTextureNormal;
	Texture* m_pTextureSpecular;
	Texture* m_pTextureGlossiness;
	//
	ID3D11InputLayout* m_pVertexLayout;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	//
	int m_AmountIndices;
	//
	const MeshType m_MeshType;
};