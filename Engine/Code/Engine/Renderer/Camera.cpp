#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"

//---------------------------------------------------------------------------------------------------------
Rgba8 Camera::GetClearColor() const
{
	return m_clearColor;
}


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
bool Camera::ShouldClearColor() const
{
	return m_clearMode & CLEAR_COLOR_BIT;
}


//---------------------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget() const
{
	return m_colorTarget;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetClearMode( unsigned int clearFlags, Rgba8 color, float depth, unsigned int stencil )
{
	m_clearMode		= clearFlags;
	m_clearColor	= color;

	UNUSED( depth );
	UNUSED( stencil );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetColorTarget( Texture* texture )
{
	m_colorTarget = texture;
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate2D( const Vec2& translation2D )
{
	m_bottomLeft += translation2D;
	m_topRight += translation2D;
}

