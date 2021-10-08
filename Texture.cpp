#include "pch.h"
#include "Texture.h"

Texture::Texture(SDL_Window* pWindow, const std::string& pathToFile)
	:m_pSurface{ IMG_Load(pathToFile.c_str()) }
	, m_pTexture{}
	, m_pTextureResourceView{}
{
	//Rasterizer
	m_pSurfacePixels = (uint32_t*)m_pSurface->pixels;
}

Texture::Texture(SDL_Window* pWindow, ID3D11Device* pDevice, const std::string& pathToFile)
	: m_pTexture{}
	, m_pTextureResourceView{}
{
	//DiretcX
	m_pSurface = { IMG_Load(pathToFile.c_str()) };

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	if (m_pTexture)
	{
		hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pTextureResourceView);
	}
}

Texture::~Texture()
{
	SDL_FreeSurface(m_pSurface);
	if (m_pTexture)
	{
		m_pTexture->Release();
	}
	if (m_pTextureResourceView)
	{
		m_pTextureResourceView->Release();
	}
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
	return m_pTextureResourceView;
}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	SDL_LockSurface(m_pSurface);
	Uint8 r{};
	Uint8 g{};
	Uint8 b{};
	const Uint32 pixel{ static_cast<Uint32>(Uint32(uv.x * m_pSurface->w) + Uint32(uv.y * m_pSurface->h) * m_pSurface->w) };

	SDL_GetRGB(m_pSurfacePixels[pixel], m_pSurface->format, &r, &g, &b);

	return Elite::RGBColor(r / 255.f, g / 255.f, b / 255.f);
}