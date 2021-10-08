#include "pch.h"
#include "Camera.h"
#include "EMath.h"

Camera::Camera(const Elite::FVector3& position, Elite::Renderer::RendererApplication rendererApp)
	:m_Position{ position }
	, m_Angle{ 45 }
	, m_Fov{ tanf(Elite::ToRadians(m_Angle / 2.f)) }
	, m_NearPlane{ 0.1f }
	, m_FarPlane{ 150.0f }
	, m_RendererApp{ rendererApp }
{
	//to put the correct values for when you start with DirectX
	if (rendererApp == Elite::Renderer::RendererApplication::directX)
	{
		//flips
		m_Position.z = -m_Position.z;
		m_Forward.x = -m_Forward.x;
		m_Forward.y = -m_Forward.y;
	}
}

Elite::FMatrix4 Camera::GetLookAtMatrix()
{
	Normalize(m_Forward);
	m_Right = Cross(m_WorldUp, m_Forward);
	Normalize(m_Right);
	m_Up = Cross(m_Forward, m_Right);
	Normalize(m_Up);

	const Elite::FMatrix4 onb = Elite::FMatrix4
	{
		Elite::FVector4(m_Right),
		Elite::FVector4(m_Up),
		Elite::FVector4(m_Forward),
		Elite::FVector4(Elite::FVector3(m_Position), 1.f)
	};

	return  onb;
}

void Camera::ForwardTranslation(float distance)
{
	m_Position += m_Forward * distance;
}

void Camera::RightTranslation(float distance)
{
	m_Position += m_Right * distance;
}

void Camera::UpTranslation(float distance)
{
	m_Position += m_Up * distance;
}

void Camera::Pitch(float angle)
{
	Elite::FMatrix3 xRot = MakeRotation(Elite::ToRadians(angle), m_Right);
	m_Forward = Inverse(Transpose(xRot)) * m_Forward;
}

void Camera::Yaw(float angle)
{
	Elite::FMatrix3 yRot = MakeRotation(Elite::ToRadians(angle), m_Up);
	m_Forward = Inverse(Transpose(yRot)) * m_Forward;
}

Elite::FMatrix4 Camera::GetProjectionMatrix(const uint32_t width, const uint32_t height) const
{
	float aspectRatio = width / height;

	if (m_RendererApp == Elite::Renderer::RendererApplication::directX)
	{
		Elite::FMatrix4 projectionMatrix
		{
			1 / (aspectRatio * m_Fov) ,0,0,0,
			0, 1 / m_Fov , 0,0,
			0,0,m_FarPlane / (m_FarPlane - m_NearPlane), -(m_FarPlane * m_NearPlane) / (m_FarPlane - m_NearPlane) ,
			0,0,1,0,
		};
		return projectionMatrix;
	}
	else
	{
		Elite::FMatrix4 projectionMatrix
		{
			1 / (aspectRatio * m_Fov) ,0,0,0,
			0, 1 / m_Fov , 0,0,
			0,0,m_FarPlane / (m_NearPlane - m_FarPlane), (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane) ,
			0,0,-1,0,
		};
		return projectionMatrix;
	}
}

Elite::FMatrix4 Camera::GetViewMatrix()
{
	return Elite::Inverse(GetLookAtMatrix());
}

Elite::FPoint3 Camera::GetPosition() const
{
	return m_Position;
}

void Camera::SetRendererAppAndInvertAxis(Elite::Renderer::RendererApplication rendererApp)
{
	m_RendererApp = rendererApp;
	//flips ,if rasterizer it will be positive, if directX it will be negative
	m_Position.z = -m_Position.z;
	//m_Position.x = -m_Position.x; //otherwise mirrored
	m_Forward.x = -m_Forward.x;
	m_Forward.y = -m_Forward.y;
	//rotate the camera to see the other rendererApp (directX or rasterizer)
}