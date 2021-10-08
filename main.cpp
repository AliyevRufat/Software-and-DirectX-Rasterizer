#include "pch.h"
//#undef main

//External includes
#include "vld.h"
#include "SDL.h"
#include "SDL_surface.h"

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "Camera.h"
#include "Scenegraph.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 550;
	const uint32_t height = 550;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX&Rasterizer - Rufat Aliyev",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//RendererApplication
	Elite::Renderer::RendererApplication rendererApp = { Elite::Renderer::RendererApplication::directX };
	//Texture Filtering Technique
	Elite::Renderer::TextureFilteringTechnique textureFiltering = { Elite::Renderer::TextureFilteringTechnique::point };
	//cullMode
	Elite::Renderer::CullMode cullMode = { Elite::Renderer::CullMode::backFace };

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	//Init camera
	Camera camera = { Elite::FVector3{0.f,5,70.0f } , rendererApp };

	//vars
	int appSwitchmultiplier{};
	if (rendererApp == Elite::Renderer::RendererApplication::directX)
	{
		appSwitchmultiplier = -1;
	}
	else
	{
		appSwitchmultiplier = 1;
	}
	float deltaTime{ 0.f };
	int rotationSpeedUpDirectX = 0;
	const float movementSpeed = 30.0f;
	const float rotationSpeed = 20.0f;
	const float meshRotationSpeed = 70.0f;
	bool isLeftButtonPressed = false;
	bool isRightButtonPressed = false;
	bool hasSwitchedRendererApp = false;
	bool isTransparencyOn = true;
	//mouse vars
	Elite::FPoint2 prevMousePosition{ 0,0 };
	Elite::FPoint2 newMousePosition{ 0 ,0 };
	//World&ProjMatrix
	Elite::FVector3 worldPosition{ 0.f,0.f,0.0f };
	//
	Elite::FMatrix4 worldMatrixRasterizer{
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		Elite::FVector4{worldPosition,1 }
	};
	//
	Elite::FMatrix4 worldMatrixDirectX{
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		Elite::FVector4{worldPosition,1 }
	};
	//
	Elite::FMatrix4 projectionMatrix = camera.GetProjectionMatrix(width, height);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		deltaTime = pTimer->GetElapsed();

		//change appSwitchmultiplier sign (that is multiplied with the movement so that you move in the right axis)
		if (rendererApp == Elite::Renderer::RendererApplication::directX)
		{
			appSwitchmultiplier = -1;
			rotationSpeedUpDirectX = 20;
		}
		else
		{
			appSwitchmultiplier = 1;
			rotationSpeedUpDirectX = 1;
		}
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//change the using renderer app
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
				{
					if (rendererApp == Elite::Renderer::RendererApplication::directX)
					{
						rendererApp = Elite::Renderer::RendererApplication::rasterizer;
						hasSwitchedRendererApp = true;
					}
					else
					{
						rendererApp = Elite::Renderer::RendererApplication::directX;
						hasSwitchedRendererApp = true;
					}
				}
				//change the transparency
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
				{
					isTransparencyOn = !isTransparencyOn;
				}
				//change the culling mode
				if (e.key.keysym.scancode == SDL_SCANCODE_C)
				{
					if (cullMode == Elite::Renderer::CullMode::backFace)
					{
						cullMode = Elite::Renderer::CullMode::frontFace;
					}
					else if (cullMode == Elite::Renderer::CullMode::frontFace)
					{
						cullMode = Elite::Renderer::CullMode::none;
					}
					else
					{
						cullMode = Elite::Renderer::CullMode::backFace;
					}
				}
				//change the texture filtering
				if (rendererApp == Elite::Renderer::RendererApplication::directX && e.key.keysym.scancode == SDL_SCANCODE_F)
				{
					if (textureFiltering == Elite::Renderer::TextureFilteringTechnique::point)
					{
						textureFiltering = Elite::Renderer::TextureFilteringTechnique::linear;
					}
					else if (textureFiltering == Elite::Renderer::TextureFilteringTechnique::linear)
					{
						textureFiltering = Elite::Renderer::TextureFilteringTechnique::anisotropic;
					}
					else
					{
						textureFiltering = Elite::Renderer::TextureFilteringTechnique::point;
					}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					isLeftButtonPressed = true;
				}
				if (e.button.button == SDL_BUTTON_RIGHT)
				{
					isRightButtonPressed = true;
				}
				break;
			case SDL_MOUSEMOTION:
				newMousePosition.x = float(e.motion.x);
				newMousePosition.y = float(e.motion.y);
				//delta mouse pos
				Elite::FPoint2 deltaMousePos{ newMousePosition.x - prevMousePosition.x ,newMousePosition.y - prevMousePosition.y };

				if (isLeftButtonPressed && isRightButtonPressed)
				{
					// if LMB and RMB pressed at the same time and dragged on Y axis(translate y)
					camera.UpTranslation(movementSpeed * deltaTime * deltaMousePos.y * rotationSpeedUpDirectX);
				}
				else if (isLeftButtonPressed && !isRightButtonPressed)
				{
					// if LMB dragged on Y axis (translate z)
					camera.ForwardTranslation(movementSpeed * deltaTime * deltaMousePos.y * appSwitchmultiplier * rotationSpeedUpDirectX);

					// if LMB dragged on X axis (rotate y)
					camera.Yaw(rotationSpeed * deltaTime * deltaMousePos.x * appSwitchmultiplier * rotationSpeedUpDirectX);
				}
				else if (!isLeftButtonPressed && isRightButtonPressed)
				{
					// if RMB dragged on X axis (rotate y)
					camera.Yaw(rotationSpeed * deltaTime * deltaMousePos.x * appSwitchmultiplier * rotationSpeedUpDirectX);
					// if RMB dragged on Y axis (rotate x)
					camera.Pitch(rotationSpeed * deltaTime * deltaMousePos.y * appSwitchmultiplier * rotationSpeedUpDirectX);
				}

				prevMousePosition.x = float(e.motion.x);
				prevMousePosition.y = float(e.motion.y);
				break;
			case SDL_MOUSEBUTTONUP:
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					isLeftButtonPressed = false;
				}
				if (e.button.button == SDL_BUTTON_RIGHT)
				{
					isRightButtonPressed = false;
				}
				break;
			case SDL_KEYDOWN:

				break;
			}
		}
		//movement cam
		const Uint8* pStates = SDL_GetKeyboardState(nullptr);

		if (pStates[SDL_SCANCODE_W])
		{
			camera.ForwardTranslation(deltaTime * -movementSpeed * appSwitchmultiplier);
		}
		else if (pStates[SDL_SCANCODE_S])
		{
			camera.ForwardTranslation(deltaTime * movementSpeed * appSwitchmultiplier);
		}
		if (pStates[SDL_SCANCODE_A])
		{
			camera.RightTranslation(deltaTime * -movementSpeed);
		}
		else if (pStates[SDL_SCANCODE_D])
		{
			camera.RightTranslation(deltaTime * movementSpeed);
		}
		//--------- Render ---------
		if (hasSwitchedRendererApp)
		{
			//adjust pos.z and forward .x and .y if application switched
			camera.SetRendererAppAndInvertAxis(rendererApp);

			//update proj matrix
			projectionMatrix = camera.GetProjectionMatrix(width, height);

			//set boolean off for the next swuitch
			hasSwitchedRendererApp = false;
		}
		//rotation and world matrices of rasterizer and directX (update them both always to have the same rotation)
		Elite::FMatrix4 rotationMatrixRasterizer{ Elite::MakeRotationY(Elite::ToRadians(meshRotationSpeed * deltaTime)) };
		Elite::FMatrix4 rotationMatrixDirectX{ Elite::MakeRotationY(Elite::ToRadians(-meshRotationSpeed * deltaTime)) };

		worldMatrixRasterizer = worldMatrixRasterizer * rotationMatrixRasterizer;
		worldMatrixDirectX = worldMatrixDirectX * rotationMatrixDirectX;

		Elite::FMatrix4	wVPMatrix{};
		Elite::FMatrix4 worldMatrix{};

		//update the worldviewprojection matrix and the worldmatrix according to the using renderer application
		if (rendererApp == Elite::Renderer::RendererApplication::directX)
		{
			wVPMatrix = { projectionMatrix * Elite::Inverse(camera.GetLookAtMatrix()) * worldMatrixDirectX };
			worldMatrix = worldMatrixDirectX;
		}
		else
		{
			wVPMatrix = { projectionMatrix * Elite::Inverse(camera.GetLookAtMatrix()) * worldMatrixRasterizer };
			worldMatrix = worldMatrixRasterizer;
		}
		//render
		pRenderer->Render(wVPMatrix, worldMatrix, camera.GetLookAtMatrix(), camera.GetPosition(), textureFiltering, rendererApp, isTransparencyOn, cullMode);
		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//clean up scenegraph
	Scenegraph::GetInstance()->ResetInstance();
	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}