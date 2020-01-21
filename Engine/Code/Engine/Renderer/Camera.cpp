#include "Engine/Renderer/Camera.hpp"


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetCameraDimensions() const
{
	return m_topRight - m_bottomLeft;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight )
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_bottomLeft;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoTopRight() const
{
	return m_topRight;
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate2D( const Vec2& translation2D )
{
	m_bottomLeft += translation2D;
	m_topRight += translation2D;
}

