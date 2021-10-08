#pragma once
#include "EMath.h"
#include "ERenderer.h"

class Camera final
{
public:
	Camera(const Elite::FVector3& position, Elite::Renderer::RendererApplication rendererApp);

	//Functions
	Elite::FMatrix4 GetLookAtMatrix();
	Elite::FPoint3 GetPosition() const;
	void ForwardTranslation(float distance);
	void RightTranslation(float distance);
	void UpTranslation(float distance);
	void Pitch(float angle);
	void Yaw(float angle);
	Elite::FMatrix4 GetProjectionMatrix(const uint32_t width, const uint32_t height) const;
	Elite::FMatrix4 GetViewMatrix();
	void SetRendererAppAndInvertAxis(Elite::Renderer::RendererApplication rendererApp);
private:
	//Datamembers
	Elite::FVector3  m_Forward{ 0.f, 0.f, 1.f }; //z axis
	Elite::FVector3		  m_Up{ 0.f, 1.f, 0.f }; //y axis
	Elite::FVector3    m_Right{ 1.f, 0.f, 0.f }; //x axis
	const Elite::FVector3 m_WorldUp{ 0.f,1.f,0.f };
	Elite::FPoint3 m_Position;
	const float m_Angle;
	const float m_Fov;
	const float m_NearPlane;
	const float m_FarPlane;
	//rendererApp
	Elite::Renderer::RendererApplication m_RendererApp;
	bool m_HasSwitchedRendererApp = false;
};
