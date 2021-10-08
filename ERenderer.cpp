#include "pch.h"
//External includes
#include "SDL.h"
#include "SDL_surface.h"
//Project includes
#include "ERenderer.h"
#include "Scenegraph.h"

Elite::Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
	, m_pUvMap{ new Texture(pWindow, "Resources/vehicle_diffuse.png") }
	, m_pNormalMap{ new Texture(pWindow, "Resources/vehicle_normal.png") }
	, m_pGlossinessMap{ new Texture(pWindow, "Resources/vehicle_gloss.png") }
	, m_pSpecularMap{ new Texture(pWindow, "Resources/vehicle_specular.png") }
	, m_IsVertexInScreen{}

{
	//Read the obj file (parser) for the mesh
	Scenegraph::GetInstance()->ReadOBJFile("Resources/vehicle.obj");
	//Create data for our mesh //directx vertices
	std::vector<Vertex_Input>vertices = Scenegraph::GetInstance()->GetVertices();
	std::vector<uint32_t> indices = Scenegraph::GetInstance()->GetIndices();
	//raster vertices
	m_OriginalVertices.resize(vertices.size());
	m_IsVertexInScreen.resize(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		m_OriginalVertices[i] = (Vertex{ Elite::FPoint3(vertices[i].Position.x, vertices[i].Position.y, -vertices[i].Position.z)
			,vertices[i].Color,vertices[i].TexCoord, Elite::FVector3(vertices[i].Normal.x, vertices[i].Normal.y, -vertices[i].Normal.z),
			Elite::FVector3(vertices[i].Tangent.x, vertices[i].Tangent.y, -vertices[i].Tangent.z), Elite::FVector3(0,0,0) });
	}
	m_Vertices = m_OriginalVertices;
	m_IndexBuffer = indices;
	////Read the obj file(parser) for fire particle
	Scenegraph::GetInstance()->ReadOBJFile("Resources/fireFX.obj");
	std::vector<Vertex_Input>verticesFireParticle = Scenegraph::GetInstance()->GetVertices();
	std::vector<uint32_t> indicesFireParticle = Scenegraph::GetInstance()->GetIndices();

	//Initialize (for rasterizer)
	m_pWindow = pWindow;
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize (for DirectX)
	//Initialize DirectX pipeline
	//...
	InitializeDirectX();
	m_IsInitialized = true;
	std::cout << "DirectX is ready\n";

	//meshInitialize
	m_pVehicleMesh = (new Mesh3D(pWindow, m_pDevice, vertices, indices, Mesh3D::MeshType::vehicle));
	m_pFireMesh = (new Mesh3D(pWindow, m_pDevice, verticesFireParticle, indicesFireParticle, Mesh3D::MeshType::fireParticle));
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);

	if (FAILED(result))
	{
		return result;
	}

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));

	if (FAILED(result))
	{
		return result;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;
	//Create SwapChain and hook it into the handle of te SDL window
	m_pSwapChain = {};
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

	if (FAILED(result))
	{
		return result;
	}

	//Crate the Depth/Stencil
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	//Create the resource view for our Depth/Stencil Buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	//Create the actual resource and the 'matching' resource view
	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);

	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

	if (FAILED(result))
	{
		return result;
	}

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);

	if (FAILED(result))
	{
		return result;
	}

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);
}

Elite::Renderer::~Renderer()
{
	//Rasterizer------------------------------------
	delete m_pSpecularMap;
	delete m_pGlossinessMap;
	delete m_pNormalMap;
	delete m_pUvMap;
	//DirectX---------------------------------------
	delete m_pFireMesh;
	delete m_pVehicleMesh;

	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}
	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
	}
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
	}
}

void Elite::Renderer::Render(Elite::FMatrix4& worldViewProjMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewInverseMatrix, const Elite::FPoint3& cameraPos, TextureFilteringTechnique textureFiltering, RendererApplication rendererApp, bool isTransparencyOn, CullMode cullMode)
{
	//Clear Buffers
	RGBColor clearColor = RGBColor(0.35f, 0.35f, 0.35f);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH || D3D11_CLEAR_STENCIL, 1.0f, 0);
	//
	if (rendererApp == RendererApplication::directX)//-----------------------------------------DIRECTX-------------------------------------------
	{
		if (!m_IsInitialized)
			return;

		//Render
		//...
		m_pVehicleMesh->Render(m_pDeviceContext, worldViewProjMatrix, worldMatrix, viewInverseMatrix, textureFiltering, false, cullMode);
		m_pFireMesh->Render(m_pDeviceContext, worldViewProjMatrix, worldMatrix, viewInverseMatrix, textureFiltering, isTransparencyOn, cullMode);
		//Present
		m_pSwapChain->Present(0, 0);
	}
	else//------------------------------------------------------------------------------------RASTERIZER--------------------------------------------
	{
		SDL_LockSurface(m_pBackBuffer);
		//set sizes of vectors and the values
		std::vector<Elite::RGBColor> coloredPixels(m_Height * m_Width);
		std::vector<float> depthBufer(m_Height * m_Width);
		//gray background
		for (size_t i = 0; i < coloredPixels.size(); i++)
		{
			coloredPixels[i] = { 175.f, 175.f, 175.f };
		}
		//buffer clean
		for (size_t i = 0; i < depthBufer.size(); i++)
		{
			depthBufer[i] = FLT_MAX;
		}
		//
		VertexTransformationFunction(worldViewProjMatrix, worldMatrix, cameraPos);
		//
		for (size_t i = 0; i < m_IndexBuffer.size() - 2; i += 3)
		{
			//check if the "triangle" is inside of the screen
			if (m_IsVertexInScreen[m_IndexBuffer[i]] && m_IsVertexInScreen[m_IndexBuffer[i + 1]] && m_IsVertexInScreen[m_IndexBuffer[i + 2]])
			{
				bool switchWeightAndPointWs = false;
				Elite::FPoint4 p0 = {};
				Elite::FPoint4 p1 = {};
				Elite::FPoint4 p2 = {};
				switch (cullMode)
				{
				case Elite::Renderer::CullMode::backFace:
					p0 = m_Vertices[m_IndexBuffer[i]].Position;
					p1 = m_Vertices[m_IndexBuffer[i + 1]].Position;
					p2 = m_Vertices[m_IndexBuffer[i + 2]].Position;
					break;
				case Elite::Renderer::CullMode::frontFace:
					p0 = m_Vertices[m_IndexBuffer[i]].Position;
					p1 = m_Vertices[m_IndexBuffer[i + 2]].Position;
					p2 = m_Vertices[m_IndexBuffer[i + 1]].Position;
					break;
				case Elite::Renderer::CullMode::none:
					p0 = m_Vertices[m_IndexBuffer[i]].Position;
					p1 = m_Vertices[m_IndexBuffer[i + 1]].Position;
					p2 = m_Vertices[m_IndexBuffer[i + 2]].Position;

					if (Cross(Elite::FVector2(p1.x, p1.y) - Elite::FVector2(p0.x, p0.y), Elite::FVector2(p2.x, p2.y) - Elite::FVector2(p1.x, p1.y)) > 0)//check if we are looking at the back or front of this triangle
					{
						switchWeightAndPointWs = true;
						p0 = m_Vertices[m_IndexBuffer[i]].Position;
						p1 = m_Vertices[m_IndexBuffer[i + 2]].Position;
						p2 = m_Vertices[m_IndexBuffer[i + 1]].Position;
					}
					break;
				}

				Elite::FPoint2 topLeft{}, bottomRight{};

				topLeft.x = std::floor(std::min(p0.x, p1.x));
				topLeft.x = std::floor(std::min(float(topLeft.x), p2.x));

				topLeft.y = std::floor(std::min(p0.y, p1.y));
				topLeft.y = std::floor(std::min(float(topLeft.y), p2.y));

				bottomRight.x = std::ceil(std::max(p0.x, p1.x));
				bottomRight.x = std::ceil(std::max(float(bottomRight.x), p2.x));

				bottomRight.y = std::ceil(std::max(p0.y, p1.y));
				bottomRight.y = std::ceil(std::max(float(bottomRight.y), p2.y));

				bottomRight.x = (float)Elite::Clamp(int(bottomRight.x), 0, int(m_Width - 1));
				bottomRight.y = (float)Elite::Clamp(int(bottomRight.y), 0, int(m_Height - 1));
				topLeft.x = (float)Elite::Clamp(int(topLeft.x), 0, int(m_Width - 1));
				topLeft.y = (float)Elite::Clamp(int(topLeft.y), 0, int(m_Height - 1));

				if (topLeft.x <= (m_Width - 1) && bottomRight.x <= (m_Width - 1) && topLeft.x > 0 && bottomRight.x > 0 && topLeft.y <= (m_Height - 1)
					&& bottomRight.y <= (m_Height - 1) && topLeft.y > 0 && bottomRight.y > 0)
				{
					for (uint32_t r = (uint32_t)(topLeft.y); r < (uint32_t)(bottomRight.y); ++r)
					{
						for (uint32_t c = (uint32_t)(topLeft.x); c < (uint32_t)(bottomRight.x); ++c)
						{
							//InsideOutsideTests
							Elite::FPoint2 pixelPoint = Elite::FPoint2((float)c, (float)r);

							float w0 = InsideOutsideTest(pixelPoint, Elite::FVector2(p1.x, p1.y), Elite::FVector2(p2.x, p2.y));
							float w1 = InsideOutsideTest(pixelPoint, Elite::FVector2(p2.x, p2.y), Elite::FVector2(p0.x, p0.y));
							float w2 = InsideOutsideTest(pixelPoint, Elite::FVector2(p0.x, p0.y), Elite::FVector2(p1.x, p1.y));
							//
							bool canRender = w0 >= 0.f && w1 >= 0.f && w2 >= 0.f;

							if (canRender)
							{
								//get the right weights
								const float totalArea = abs(Elite::Cross(Elite::FVector2(Elite::FVector2(p0.x, p0.y) - Elite::FVector2(p1.x, p1.y)), Elite::FVector2(Elite::FVector2(p0.x, p0.y) - Elite::FVector2(p2.x, p2.y))));
								//
								w0 /= totalArea;	w1 /= totalArea;	w2 /= totalArea;

								//calculate Z buffer
								float zBufferValue = 1.f / ((1.f / p0.z * w0) + (1.f / p1.z * w1) + (1.f / p2.z * w2));
								//calculate W interpolated
								const float wInterpolated = 1.f / (1.f / p0.w * w0 + 1.f / p1.w * w1 + 1.f / p2.w * w2);

								if (zBufferValue < depthBufer[c + (r * m_Width)])
								{
									//weight and point cache vars
									float tempW1 = {};
									Elite::FPoint4 tempP1 = {};
									//change the weights and w component of p's if front culling or none culling
									if (cullMode == Elite::Renderer::CullMode::frontFace)
									{
										tempW1 = w1; // save w1
										//switch them
										w1 = w2;
										w2 = tempW1;

										tempP1.w = p1.w; // save p1.w
										//switch them
										p1.w = p2.w;
										p2.w = tempP1.w;
									}
									else if (cullMode == Elite::Renderer::CullMode::none)
									{
										if (switchWeightAndPointWs)
										{
											tempW1 = w1; // save w1
											//switch them
											w1 = w2;
											w2 = tempW1;

											tempP1.w = p1.w; // save p1.w
											//switch them
											p1.w = p2.w;
											p2.w = tempP1.w;
										}
									}
									//write depth buffer value
									depthBufer[c + (r * m_Width)] = zBufferValue;
									//uv interpolation
									FVector2 uv = m_Vertices[m_IndexBuffer[i]].TexCoord / p0.w * w0 + m_Vertices[m_IndexBuffer[i + 1]].TexCoord / p1.w * w1 + m_Vertices[m_IndexBuffer[i + 2]].TexCoord / p2.w * w2;
									uv *= wInterpolated;
									uv = Elite::FVector2(uv.x, 1 - uv.y);
									//normal interpolation
									Elite::FVector3 normal = m_Vertices[m_IndexBuffer[i]].Normal / p0.w * w0 + m_Vertices[m_IndexBuffer[i + 1]].Normal / p1.w * w1 + m_Vertices[m_IndexBuffer[i + 2]].Normal / p2.w * w2;
									normal *= wInterpolated;
									Normalize(normal);
									//tangent interpolation
									Elite::FVector3 tangent = m_Vertices[m_IndexBuffer[i]].Tangent / p0.w * w0 + m_Vertices[m_IndexBuffer[i + 1]].Tangent / p1.w * w1 + m_Vertices[m_IndexBuffer[i + 2]].Tangent / p2.w * w2;
									tangent *= wInterpolated;
									Normalize(tangent);
									//viewDirection interpolation
									Elite::FVector3 viewDirection = m_Vertices[m_IndexBuffer[i]].ViewDirection / p0.w * w0 + m_Vertices[m_IndexBuffer[i + 1]].ViewDirection / p1.w * w1 + m_Vertices[m_IndexBuffer[i + 2]].ViewDirection / p2.w * w2;
									viewDirection *= wInterpolated;
									Normalize(viewDirection);

									////pixelShading
									Vertex vertexOutput = { Elite::FPoint4(0,0,0,0),Elite::RGBColor(0,0,0), uv, normal,  tangent , viewDirection };
									coloredPixels[c + (r * m_Width)] = PixelShading(vertexOutput);
								}
							}
						}
					}
				}
			}
		}
		//coloring every pixel
		for (size_t i = 0; i < coloredPixels.size(); i++)
		{
			m_pBackBufferPixels[i] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(coloredPixels[i].r * 255),
				static_cast<uint8_t>(coloredPixels[i].g * 255),
				static_cast<uint8_t>(coloredPixels[i].b * 255));
		}

		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
}

Elite::FVector3 Elite::Renderer::TransformToWorldSpaceAndNormalize(const Elite::FVector3& originalVectorValues, const Elite::FMatrix4& worldMatrix) const
{
	const Elite::FVector4 tempNormal = worldMatrix * Elite::FVector4(originalVectorValues);
	Elite::FVector3 transformedVector = Elite::FVector3(tempNormal.x, tempNormal.y, tempNormal.z);
	Normalize(transformedVector);
	return transformedVector;
}

float Elite::Renderer::InsideOutsideTest(const Elite::FPoint2& pixelPoint, const Elite::FVector2& firstVector, const Elite::FVector2& secondVector)const
{
	const Elite::FVector2 edge = secondVector - firstVector;
	const Elite::FVector2 pointToSide = Elite::FVector2(pixelPoint) - firstVector;
	//
	const float weight = Elite::Cross(pointToSide, edge);
	return weight;
}

void Elite::Renderer::VertexTransformationFunction(Elite::FMatrix4& worldViewProjectionMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FPoint3& cameraPos)
{
	for (size_t i = 0; i < m_Vertices.size(); i++)
	{
		//normal and tangent to world
		m_Vertices[i].Normal = TransformToWorldSpaceAndNormalize(m_OriginalVertices[i].Normal, worldMatrix);
		m_Vertices[i].Tangent = TransformToWorldSpaceAndNormalize(m_OriginalVertices[i].Tangent, worldMatrix);
		//WorldViewProjectionMatrix pos
		m_Vertices[i].Position = worldViewProjectionMatrix * m_OriginalVertices[i].Position;
		//viewDirection
		m_Vertices[i].ViewDirection = Elite::FVector3(worldMatrix * Elite::FVector4(m_OriginalVertices[i].Position)) - Elite::FVector3(cameraPos);
		Normalize(m_Vertices[i].ViewDirection);
		//perspective divide
		m_Vertices[i].Position.x /= m_Vertices[i].Position.w;
		m_Vertices[i].Position.y /= m_Vertices[i].Position.w;
		m_Vertices[i].Position.z /= m_Vertices[i].Position.w;
		//ndcToRaster
		if (m_Vertices[i].Position.x <= 1 && m_Vertices[i].Position.x >= -1 && m_Vertices[i].Position.y <= 1 && m_Vertices[i].Position.y >= -1 && m_Vertices[i].Position.z <= 1 && m_Vertices[i].Position.z >= 0)
		{
			Elite::FVector2 ndcToRaster;
			ndcToRaster.x = (m_Vertices[i].Position.x + 1) / 2 * m_Width;
			ndcToRaster.y = (1 - m_Vertices[i].Position.y) / 2 * m_Height;
			m_Vertices[i].Position = Elite::FPoint4(ndcToRaster.x, ndcToRaster.y, m_Vertices[i].Position.z, m_Vertices[i].Position.w);
			m_IsVertexInScreen[i] = true;
		}
		else
		{
			m_IsVertexInScreen[i] = false;
		}
	}
}

Elite::RGBColor Elite::Renderer::PixelShading(const Vertex& outputV) const
{
	//variables
	RGBColor finalColor = { 0.f, 0.f, 0.f };
	const Elite::FVector3 lightDirection = { 0.577f, -0.577f, -0.577f };
	const Elite::RGBColor lightColor = Elite::RGBColor(0.1f, 0.1f, 0.1f);
	const float lightIntensity = 7.0f;

	//Calculate local axis for normal
	const Elite::FVector3 binormal = Cross(outputV.Tangent, outputV.Normal);
	const Elite::FMatrix3 tangentSpaceAxis = Elite::FMatrix3{ outputV.Tangent , binormal ,outputV.Normal };

	//normal map----------------------------------------------------
	const Elite::RGBColor tempNormalSample = m_pNormalMap->Sample(outputV.TexCoord);
	const Elite::FVector3 sampledValue{ tempNormalSample.r * 2.f - 1.f,tempNormalSample.g * 2.f - 1.f,tempNormalSample.b * 2.f - 1.f };
	Elite::FVector3 newNormal = tangentSpaceAxis * sampledValue;

	//diffuse color map----------------------------------------------
	const Elite::RGBColor diffuseMapSample = m_pUvMap->Sample(outputV.TexCoord);
	float observedArea = (Dot(-newNormal, lightDirection));
	observedArea = std::max(0.f, observedArea);
	observedArea /= float(M_PI);
	observedArea *= lightIntensity;
	//
	Normalize(newNormal);
	//phong----------------------------------------------------------
	const Elite::RGBColor phongSpecularColor = GetPhongColor(outputV, newNormal, lightDirection);

	//final color calc
	finalColor = lightColor + (diffuseMapSample * observedArea) + phongSpecularColor;
	finalColor.MaxToOne();
	return finalColor;
}

Elite::RGBColor Elite::Renderer::GetPhongColor(const Vertex& outputV, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection) const
{
	//sampling from the maps---------------------------------------------
	const Elite::RGBColor glossMapSample = m_pGlossinessMap->Sample(outputV.TexCoord);
	const Elite::RGBColor specularMapSample = m_pSpecularMap->Sample(outputV.TexCoord);
	//
	const float shininess = { 25.0f };
	const Elite::FVector3 reflect{ Reflect(lightDirection, newNormal) };
	float specularIntensity = Dot(-outputV.ViewDirection, reflect);
	specularIntensity = Clamp(specularIntensity, 0.f, 1.f);
	const Elite::RGBColor phongSpecularColor = specularMapSample * std::pow(specularIntensity, glossMapSample.r * shininess);
	//
	return phongSpecularColor;
}