/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include <vector>

class Texture;
class Mesh3D;

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	struct Vertex
	{
		Elite::FPoint4	Position = {};
		Elite::RGBColor Color = {};
		Elite::FVector2 TexCoord = {};
		Elite::FVector3 Normal = {};
		Elite::FVector3 Tangent = {};
		Elite::FVector3 ViewDirection = {};
	};

	class Renderer final
	{
	public:

		//Enum class to switch the culling modes in rasterizer and directX
		enum class CullMode
		{
			backFace,
			frontFace,
			none
		};

		//Enum class to change the Texture Filtering Technique
		enum class TextureFilteringTechnique
		{
			point,
			linear,
			anisotropic
		};

		//Enum class to switch between rasterizer and directX
		enum class RendererApplication
		{
			rasterizer,
			directX
		};

		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Elite::FMatrix4& worldViewProjMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewInverseMatrix,
			const Elite::FPoint3& cameraPos, TextureFilteringTechnique textureFiltering,
			RendererApplication rendererApp, bool isTransparencyOn, CullMode cullMode);

	private:
		//Methods DirectX-------------------------------
		HRESULT InitializeDirectX();
		//Methods Rasterize-----------------------------
		Elite::FVector3 TransformToWorldSpaceAndNormalize(const Elite::FVector3& originalVectorValues, const Elite::FMatrix4& worldMatrix) const;
		float InsideOutsideTest(const Elite::FPoint2& pixelPoint, const Elite::FVector2& firstVector, const Elite::FVector2& secondVector) const;
		void VertexTransformationFunction(Elite::FMatrix4& worldViewProjectionMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FPoint3& cameraPos);
		Elite::RGBColor PixelShading(const Vertex& outputV) const;
		Elite::RGBColor GetPhongColor(const Vertex& outputV, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection) const;

		//Datamembers DirectX----------------------------
		SDL_Window* m_pWindow = nullptr;
		uint32_t m_Width;
		uint32_t m_Height;
		//
		bool m_IsInitialized;
		//
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGIFactory* m_pDXGIFactory;
		//
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
		//Meshes
		Mesh3D* m_pVehicleMesh;
		Mesh3D* m_pFireMesh;

		//Datamembers Rasterizer-------------------------
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		//
		std::vector<Vertex> m_OriginalVertices;
		std::vector<Vertex> m_Vertices;

		Texture* m_pUvMap;
		Texture* m_pNormalMap;
		Texture* m_pGlossinessMap;
		Texture* m_pSpecularMap;

		std::vector<uint32_t> m_IndexBuffer;
		//bool vector for vertices to check if they are in the ndc coordinates
		std::vector<bool> m_IsVertexInScreen;
	};
}

#endif