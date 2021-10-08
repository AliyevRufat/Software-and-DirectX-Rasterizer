#pragma once
#include <string>
#include <SDL_image.h>
#include "EMath.h"
#include "EVector2.h"
#include "ERGBColor.h"

class Texture
{
public:
	Texture(SDL_Window* pWindow, ID3D11Device* pDevice, const std::string& pathToFile);
	Texture(SDL_Window* pWindow, const std::string& pathToFile);
	//Rule of Five
	~Texture();
	Texture(const Texture&) = delete;
	Texture(Texture&&) noexcept = delete;
	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&) noexcept = delete;

	//Functions DirectX---------------------------------------
	ID3D11ShaderResourceView* GetShaderResourceView() const;

	//Functions Rasterizer------------------------------------
	Elite::RGBColor Sample(const Elite::FVector2& uv) const;
private:
	//Datamembers DirectX-------------------------------------
	ID3D11ShaderResourceView* m_pTextureResourceView;
	ID3D11Texture2D* m_pTexture;

	//Datamembers Rasterizer----------------------------------
	SDL_Surface* m_pSurface{};
	uint32_t* m_pSurfacePixels = nullptr;
};
