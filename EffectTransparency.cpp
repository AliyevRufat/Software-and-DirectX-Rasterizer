#include "pch.h"
#include "EffectTransparency.h"
#include <sstream>

EffectTransparency::EffectTransparency(ID3D11Device* pDevice, const std::wstring& effectPath)
{
	//get the effect from the loadeffect function
	m_pEffect = LoadEffect(pDevice, effectPath);
	//
	m_pTechniques = m_pEffect->GetTechniqueByName("Techniques");
	if (!m_pTechniques->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}
	//
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
	}
	//
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pDiffuseMapVariable not found\n";
	}
}

EffectTransparency::~EffectTransparency()
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}
	if (m_pTechniques)
	{
		m_pTechniques->Release();
	}
	if (m_pEffect)
	{
		m_pEffect->Release();
	}
}

ID3DX11Effect* EffectTransparency::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect = nullptr;

	DWORD shaderFlags = 0;
	//#if defined( DEBUG ) || defined (_DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	//#endif
	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			char* pErrors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}

ID3DX11Effect* EffectTransparency::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* EffectTransparency::GetTechniques() const
{
	return m_pTechniques;
}

ID3DX11EffectMatrixVariable* EffectTransparency::GetMatWorldViewProjVar() const
{
	return m_pMatWorldViewProjVariable;
}

void EffectTransparency::SetDiffuseMap(ID3D11ShaderResourceView* pResourceViewDiffuse)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pResourceViewDiffuse);
	}
}