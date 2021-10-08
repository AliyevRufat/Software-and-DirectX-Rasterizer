#include "pch.h"
#include "Effect.h"
#include <sstream>

Effect::Effect(ID3D11Device* pDevice, const std::wstring& effectPath)
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
	//
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pNormalMapVariable not found\n";
	}
	//
	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pSpecularMapVariable not found\n";
	}
	//
	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"Variable m_pGlossinessMapVariable not found\n";
	}
	//
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
	{
		std::wcout << L"Variable m_pMatWorldVariable not valid\n";
	}
	//
	m_pMatViewInvVariable = m_pEffect->GetVariableByName("gViewInversed")->AsMatrix();
	if (!m_pMatViewInvVariable->IsValid())
	{
		std::wcout << L"Variable m_pMatViewInvVariable not valid\n";
	}
}

Effect::~Effect()
{
	if (m_pMatViewInvVariable)
	{
		m_pMatViewInvVariable->Release();
	}
	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->Release();
	}
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->Release();
	}
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}
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

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
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

ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechniques() const
{
	return m_pTechniques;
}

ID3DX11EffectMatrixVariable* Effect::GetMatWorldViewProjVar() const
{
	return m_pMatWorldViewProjVariable;
}

ID3DX11EffectMatrixVariable* Effect::GetMatWorldVar() const
{
	return m_pMatWorldVariable;
}

ID3DX11EffectMatrixVariable* Effect::GetMatViewInvVar() const
{
	return m_pMatViewInvVariable;
}

void Effect::SetDiffuseNormalSpecularGlossinessMaps(ID3D11ShaderResourceView* pResourceViewDiffuse, ID3D11ShaderResourceView* pResourceViewNormal, ID3D11ShaderResourceView* pResourceViewSpecular, ID3D11ShaderResourceView* pResourceViewGlossiness)
{
	if (m_pDiffuseMapVariable->IsValid())
	{
		m_pDiffuseMapVariable->SetResource(pResourceViewDiffuse);
	}
	if (m_pNormalMapVariable->IsValid())
	{
		m_pNormalMapVariable->SetResource(pResourceViewNormal);
	}
	if (m_pSpecularMapVariable->IsValid())
	{
		m_pSpecularMapVariable->SetResource(pResourceViewSpecular);
	}
	if (m_pGlossinessMapVariable->IsValid())
	{
		m_pGlossinessMapVariable->SetResource(pResourceViewGlossiness);
	}
}