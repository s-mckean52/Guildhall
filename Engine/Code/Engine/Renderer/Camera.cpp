#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
Camera::~Camera()
{
	delete m_uniformBuffer;
	m_uniformBuffer = nullptr;

	delete m_colorTarget;
	m_colorTarget = nullptr;
}


//---------------------------------------------------------------------------------------------------------
Camera::Camera( RenderContext* renderer )
{
	m_uniformBuffer = new RenderBuffer( renderer, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetPosition( const Vec3& position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate( const Vec3& translation )
{
	m_position += translation;
}


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

	m_projection = Mat44::CreateOrthographicProjection( Vec3( bottomLeft, 0.0f ), Vec3( topRight, 1.0f ) );
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
Mat44 Camera::GetProjectionMatrix() const
{
	return m_projection;
}


//---------------------------------------------------------------------------------------------------------
Mat44 Camera::GetViewMatrix() const
{
	return m_view;
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
RenderBuffer* Camera::GetUBO() const
{
	return m_uniformBuffer;
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

