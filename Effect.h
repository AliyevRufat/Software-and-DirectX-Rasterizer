#pragma once

class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& effectPath);
	//Rule of five
	~Effect();
	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	//Functions
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	//Getters and Setters
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechniques() const;
	ID3DX11EffectMatrixVariable* GetMatWorldViewProjVar()const;

	ID3DX11EffectMatrixVariable* GetMatWorldVar() const;
	ID3DX11EffectMatrixVariable* GetMatViewInvVar() const;
	void SetDiffuseNormalSpecularGlossinessMaps(ID3D11ShaderResourceView* pResourceViewDiffuse, ID3D11ShaderResourceView* pResourceViewNormal, ID3D11ShaderResourceView* pResourceViewSpecular, ID3D11ShaderResourceView* pResourceViewGlossiness);

private:
	//Datamembers
	ID3DX11Effect* m_pEffect;

	ID3DX11EffectTechnique* m_pTechniques;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	//
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
	//
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
	ID3DX11EffectMatrixVariable* m_pMatViewInvVariable;
};
