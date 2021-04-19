#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"


//---------------------------------------------------------------------------------------------------------
Camera::~Camera()
{
	delete m_uniformBuffer;
	m_uniformBuffer = nullptr;
}


//---------------------------------------------------------------------------------------------------------
Camera::Camera( RenderContext* renderer )
{
	m_renderer = renderer;
	m_uniformBuffer = new RenderBuffer( renderer, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetPosition( const Vec3& position )
{
	m_transform.SetPosition( position );
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate( const Vec3& translation )
{
	m_transform.Translate( translation );
}


//---------------------------------------------------------------------------------------------------------
Rgba8 Camera::GetClearColor() const
{
	return m_clearColor;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetColorTargetSize( uint index ) const
{
	Texture* colorTarget = m_colorTargets[index];
	if( colorTarget != nullptr )
	{
		return colorTarget->GetSize();
	}
	else
	{
		return m_renderer->GetBackBuffer()->GetSize();
	}
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Camera::GetColorTargetTexelSize( uint index ) const
{
	Texture* colorTarget = m_colorTargets[index];
	if( colorTarget != nullptr )
	{
		return colorTarget->GetImageTexelSize();
	}
	else
	{
		return m_renderer->GetBackBuffer()->GetImageTexelSize();
	}
}


//---------------------------------------------------------------------------------------------------------
Vec2 Camera::GetCameraDimensions() const
{
	Vec3 cameraDimensions = GetOrthoTopRight() - GetOrthoBottomLeft();
	return Vec2( cameraDimensions.x, cameraDimensions.y );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight )
{
	float height = topRight.y - bottomLeft.y;

	SetProjectionOrthographic( height, 0.f, -1.f );
}


//---------------------------------------------------------------------------------------------------------
Vec3 Camera::GetOrthoBottomLeft() const
{
	Vec4 ndc( -1, -1, 0, 1 );
	return NDCToWorldCoords( ndc );
}


//---------------------------------------------------------------------------------------------------------
Vec3 Camera::GetOrthoTopRight() const
{
	Vec4 ndc( 1, 1, 0, 1 );
	return NDCToWorldCoords( ndc );
}


//---------------------------------------------------------------------------------------------------------
Vec3 Camera::GetPosition() const
{
	return m_transform.GetPosition();
}


//---------------------------------------------------------------------------------------------------------
float Camera::GetPitchDegrees() const
{
	return m_transform.GetRotationPitchYawRollDegrees().x;
}


//---------------------------------------------------------------------------------------------------------
float Camera::GetYawDegrees() const
{
	return m_transform.GetRotationPitchYawRollDegrees().y;
}


//---------------------------------------------------------------------------------------------------------
float Camera::GetRollDegrees() const
{
	return m_transform.GetRotationPitchYawRollDegrees().z;
}


//---------------------------------------------------------------------------------------------------------
Vec3 Camera::GetScale() const
{
	return m_transform.GetScale();
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
void Camera::UpdateViewMatrix()
{
	Mat44 cameraModel = m_transform.ToMatrix();
	MatrixInvertOrthoNormal( cameraModel );
	m_view = cameraModel;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetTransform( Transform transform )
{
	m_transform = transform;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetViewMatrix( Mat44 viewMatrix )
{
	m_view = viewMatrix;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetProjectionMatrix( Mat44 projectionMatrix )
{
	m_projection = projectionMatrix;
}


//---------------------------------------------------------------------------------------------------------
Vec3 Camera::NDCToWorldCoords( const Vec4& ndcCoords ) const
{
	Mat44 inverseProjectionMatrix = m_projection;
	MatrixInvert( inverseProjectionMatrix );
	Mat44 cameraToWorld = m_transform.ToMatrix();
	cameraToWorld.TransformBy( inverseProjectionMatrix );

	Vec4 world = cameraToWorld.TransformHomogeneousPoint3D( ndcCoords );
	world /= world.w;
	return Vec3( world.x, world.y, world.z );
}


//---------------------------------------------------------------------------------------------------------
bool Camera::ShouldClearColor() const
{
	return m_clearMode & CLEAR_COLOR_BIT;
}


//---------------------------------------------------------------------------------------------------------
bool Camera::ShouldClearDepth() const
{
	return m_clearMode & CLEAR_DEPTH_BIT;
}


//---------------------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget( uint index ) const
{
	return m_colorTargets[ index ];
}


//---------------------------------------------------------------------------------------------------------
RenderBuffer* Camera::GetUBO() const
{
	return m_uniformBuffer;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetClearMode( CameraClearFlags clearFlags, Rgba8 color, float depth, unsigned int stencil )
{
	m_clearMode		= clearFlags;
	m_clearColor	= color;
	m_clearDepth	= depth;
	m_clearStencil	= stencil;
}


//---------------------------------------------------------------------------------------------------------
float Camera::GetAspectRatio() const
{
	Vec2 outSize = GetColorTargetSize( 0 );
	return outSize.x / outSize.y;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetColorTarget( Texture* texture )
{
	SetColorTarget( 0, texture );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetColorTarget( uint index, Texture* texture )
{
	uint newColorTargetsSize = index + 1;
	if( newColorTargetsSize > m_colorTargets.size() )
	{
		m_colorTargets.resize( newColorTargetsSize );
	}
	m_colorTargets[ index ] = texture;
}


//---------------------------------------------------------------------------------------------------------
void Camera::Translate2D( const Vec2& translation2D )
{
	Translate( Vec3( translation2D, 0.f ) );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_depthStencilTarget = texture;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetYawDegrees( float yaw )
{
	m_transform.SetYawDegrees( yaw );
}

//---------------------------------------------------------------------------------------------------------
void Camera::SetPitchYawRollRotationDegrees( float pitch, float yaw, float roll )
{
	m_transform.SetRotationFromPitchYawRollDegrees( pitch, yaw, roll );
}


//---------------------------------------------------------------------------------------------------------
void Camera::AddPitchYawRollRotationDegrees( float pitch, float yaw, float roll )
{
	Vec3 pitchYawRoll = m_transform.GetRotationPitchYawRollDegrees();
	pitchYawRoll += Vec3( pitch, yaw, roll );
	SetPitchYawRollRotationDegrees( pitchYawRoll.x, pitchYawRoll.y , pitchYawRoll.z );
}

//---------------------------------------------------------------------------------------------------------
Vec3 Camera::ClientToWorldPosition( Vec2 const& clientPosition, float ndcZ ) const
{
	Vec3 ndc;

	ndc.x = RangeMapFloat( 0.f, 1.f, -1.f, 1.f, clientPosition.x );
	ndc.y = RangeMapFloat( 0.f, 1.f, -1.f, 1.f, clientPosition.y );
	ndc.z = ndcZ;
	
	Vec3 worldPos = NDCToWorldCoords( Vec4( ndc, 1.f ) );
	return worldPos;
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	float aspect = GetAspectRatio();
	float halfHeight = height * 0.5f;
	float halfWidth = halfHeight * aspect;

	Vec3 min = Vec3( -halfWidth, -halfHeight, nearZ );
	Vec3 max = Vec3( halfWidth, halfHeight, farZ );

	m_projection = Mat44::CreateOrthographicProjection( min, max );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetProjectionPerspective( float fieldOfViewDegrees, float nearZ, float farZ )
{
	m_projection = Mat44::CreatePerspectiveProjection( fieldOfViewDegrees, GetAspectRatio(), nearZ, farZ );
}


//---------------------------------------------------------------------------------------------------------
void Camera::UpdateCameraUBO()
{
	camera_data_t cameraData;
	cameraData.projection = m_projection;
	cameraData.model = m_transform.ToMatrix();
	cameraData.position = GetPosition();
	cameraData.view = m_view;

	Mat44 inverseProjection = m_projection;
	MatrixInvert( inverseProjection );
	cameraData.inverseProjection = inverseProjection;

	m_uniformBuffer->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

