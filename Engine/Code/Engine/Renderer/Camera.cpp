#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetCameraDimensions() const
{
	return m_topRight - m_bottomLeft;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetOutputSize( Vec2 size )
{
	m_outputSize = size;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetPosition( Vec3 position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	UNUSED( nearZ );
	UNUSED( farZ );

	float aspectRatio = GetAspectRatio();
	float halfHeight = height * 0.5f;
	float halfWidth = halfHeight * aspectRatio;
	Vec2 halfDimensions( halfWidth, halfHeight );

	m_bottomLeft = m_position - halfDimensions;
	m_topRight = m_position + halfDimensions;
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
float Camera::GetAspectRatio() const
{
	return ( m_outputSize.x / m_outputSize.y );
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate2D( const Vec2& translation2D )
{
	m_bottomLeft += translation2D;
	m_topRight += translation2D;
}

//---------------------------------------------------------------------------------------------------------
Vec2 Camera::ClientToWorldPosition( Vec2 clientPosition )
{
	Vec2 worldPos;
	Vec2 outputDimensions = GetCameraDimensions();

// 	float normalizedClientX = RangeMapFloat( 0.f, outputDimensions.x, 0.f, 1.f, clientPosition.x );
// 	float normalizedClientY = RangeMapFloat( 0.f, outputDimensions.y, 0.f, 1.f, clientPosition.y );

	worldPos.x = RangeMapFloat( 0.f, 1.f, m_bottomLeft.x, m_topRight.x, clientPosition.x );
	worldPos.y = RangeMapFloat( 0.f, 1.f, m_bottomLeft.y, m_topRight.y, clientPosition.y );

	// TODO - take into account render target
	//        clientPos being the pixel location on the texture

	// TODO - use projection matrix to compute this

	// TODO - Support ndc-depth paramater for 3D-coordinates, needed for ray casts.

	// TODO - take into account viewport

	return worldPos;
}

