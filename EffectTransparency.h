#pragma once
class EffectTransparency final
{
public:
	EffectTransparency(ID3D11Device* pDevice, const std::wstring& effectPath);
	//Rule of five
	~EffectTransparency();
	EffectTransparency(const EffectTransparency&) = delete;
	EffectTransparency(EffectTransparency&&) noexcept = delete;
	EffectTransparency& operator=(const EffectTransparency&) = delete;
	EffectTransparency& operator=(EffectTransparency&&) noexcept = delete;

	//Functions
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	//Getters and Setters
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechniques() const;
	ID3DX11EffectMatrixVariable* GetMatWorldViewProjVar()const;

	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceViewDiffuse);

private:
	//Datamembers
	ID3DX11Effect* m_pEffect;

	ID3DX11EffectTechnique* m_pTechniques;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	//
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
};
