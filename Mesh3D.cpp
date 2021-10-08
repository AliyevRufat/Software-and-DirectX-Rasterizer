#include "pch.h"
#include "Mesh3D.h"

Mesh3D::Mesh3D(SDL_Window* pWindow, ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertices, const std::vector<uint32_t>& indices, MeshType meshType)
	:m_pEffect{ new Effect(pDevice,L"Resources/PosCol3DVehicle.fx") }
	, m_pEffectTransparency{ new EffectTransparency(pDevice,L"Resources/PosCol3DFire.fx") }
	, m_pTextureDiffuse(new Texture(pWindow, pDevice, "Resources/vehicle_diffuse.png"))
	, m_pTextureDiffuseFire(new Texture(pWindow, pDevice, "Resources/fireFX_diffuse.png"))
	, m_pTextureNormal(new Texture(pWindow, pDevice, "Resources/vehicle_normal.png"))
	, m_pTextureSpecular(new Texture(pWindow, pDevice, "Resources/vehicle_specular.png"))
	, m_pTextureGlossiness(new Texture(pWindow, pDevice, "Resources/vehicle_gloss.png"))
	, m_pVertexLayout{}
	, m_pVertexBuffer{}
	, m_pIndexBuffer{}
	, m_AmountIndices{}
	, m_MeshType{ meshType }
{
	//Create Vertex Layout
	HRESULT result = S_OK;

	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	if (meshType == MeshType::vehicle)
	{
		// Create the input layout
		D3DX11_PASS_DESC passDesc;
		m_pEffect->GetTechniques()->GetPassByIndex(0)->GetDesc(&passDesc);
		result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pVertexLayout);
		if (FAILED(result))
			return;
	}
	else
	{
		// Create the input layout for fire
		D3DX11_PASS_DESC passDesc;
		m_pEffectTransparency->GetTechniques()->GetPassByIndex(0)->GetDesc(&passDesc);
		result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pVertexLayout);
		if (FAILED(result))
			return;
	}

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_Input) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	// Create index buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}

Mesh3D::~Mesh3D()
{
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
	}
	delete m_pTextureGlossiness;
	delete m_pTextureSpecular;
	delete m_pTextureNormal;
	delete m_pTextureDiffuse;
	delete m_pTextureDiffuseFire;
	delete m_pEffect;
	delete m_pEffectTransparency;
}

void Mesh3D::Render(ID3D11DeviceContext* pDeviceContext, const Elite::FMatrix4& worldViewProjMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewInverseMatrix, Elite::Renderer::TextureFilteringTechnique textureFiltering, bool isTransparencyOn, Elite::Renderer::CullMode cullMode)
{
	UINT stride = sizeof(Vertex_Input);
	UINT offset = 0;

	if (m_MeshType == MeshType::vehicle)
	{
		//Set to WorldViewProj
		m_pEffect->GetMatWorldViewProjVar()->SetMatrix(&worldViewProjMatrix[0].x);

		//Set to World
		m_pEffect->GetMatWorldVar()->SetMatrix(&worldMatrix[0].x);

		//Set to World
		m_pEffect->GetMatViewInvVar()->SetMatrix(&viewInverseMatrix[0].x);

		//Set ShaderResourceView
		m_pEffect->SetDiffuseNormalSpecularGlossinessMaps(m_pTextureDiffuse->GetShaderResourceView(), m_pTextureNormal->GetShaderResourceView(), m_pTextureSpecular->GetShaderResourceView(), m_pTextureGlossiness->GetShaderResourceView());
	}
	else
	{
		//Set to WorldViewProj
		m_pEffectTransparency->GetMatWorldViewProjVar()->SetMatrix(&worldViewProjMatrix[0].x);

		//Set ShaderResourceView
		m_pEffectTransparency->SetDiffuseMap(m_pTextureDiffuseFire->GetShaderResourceView());
	}

	//Set vertex buffer
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0); // ? offset instead of 0 ?

	//Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (m_MeshType == MeshType::vehicle)//-----------------------------------------------------------------VEHIVLE------------------------------------------------------------------------
	{
		//Render a triangle based on the chosen Technique (can be changed by pressing 'F')
		D3DX11_TECHNIQUE_DESC techDesc;
		m_pEffect->GetTechniques()->GetDesc(&techDesc);
		//
		switch (cullMode)
		{
		case Elite::Renderer::CullMode::backFace:
			switch (textureFiltering)
			{
			case Elite::Renderer::TextureFilteringTechnique::point:
				m_pEffect->GetTechniques()->GetPassByIndex(0)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::linear:
				m_pEffect->GetTechniques()->GetPassByIndex(1)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::anisotropic:
				m_pEffect->GetTechniques()->GetPassByIndex(2)->Apply(0, pDeviceContext);

				break;
			default:
				break;
			}
			break;
		case Elite::Renderer::CullMode::frontFace:
			switch (textureFiltering)
			{
			case Elite::Renderer::TextureFilteringTechnique::point:
				m_pEffect->GetTechniques()->GetPassByIndex(3)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::linear:
				m_pEffect->GetTechniques()->GetPassByIndex(4)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::anisotropic:
				m_pEffect->GetTechniques()->GetPassByIndex(5)->Apply(0, pDeviceContext);

				break;
			default:
				break;
			}
			break;
		case Elite::Renderer::CullMode::none:
			switch (textureFiltering)
			{
			case Elite::Renderer::TextureFilteringTechnique::point:
				m_pEffect->GetTechniques()->GetPassByIndex(6)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::linear:
				m_pEffect->GetTechniques()->GetPassByIndex(7)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::anisotropic:
				m_pEffect->GetTechniques()->GetPassByIndex(8)->Apply(0, pDeviceContext);

				break;
			default:
				break;
			}
			break;
		}
	}
	else if (m_MeshType == MeshType::fireParticle)//-----------------------------------------------------------------FIREPARTICLE------------------------------------------------------------------------
	{
		D3DX11_TECHNIQUE_DESC techDescFire;
		m_pEffectTransparency->GetTechniques()->GetDesc(&techDescFire);
		//

		if (isTransparencyOn)//if the transparency is on
		{
			switch (textureFiltering)
			{
			case Elite::Renderer::TextureFilteringTechnique::point:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(0)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::linear:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(1)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::anisotropic:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(2)->Apply(0, pDeviceContext);

				break;
			default:
				break;
			}
		}
		else//if the transparency is off
		{
			switch (textureFiltering)
			{
			case Elite::Renderer::TextureFilteringTechnique::point:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(3)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::linear:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(4)->Apply(0, pDeviceContext);

				break;
			case Elite::Renderer::TextureFilteringTechnique::anisotropic:
				m_pEffectTransparency->GetTechniques()->GetPassByIndex(5)->Apply(0, pDeviceContext);

				break;
			default:
				break;
			}
		}
	}
	pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
}