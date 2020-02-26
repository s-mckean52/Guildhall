#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Camera::~Camera()
{
	delete m_uniformBuffer;
	m_uniformBuffer = nullptr;

	delete m_colorTarget;
	m_colorTarget = nullptr;

	delete m_depthStencilTarget;
	m_depthStencilTarget = nullptr;
}


//---------------------------------------------------------------------------------------------------------
Camera::Camera( RenderContext* renderer )
{
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
Vec2 Camera::GetCameraDimensions() const
{
	return Vec2( 16.0f, 9.0f );
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
Vec3 Camera::NDCToWorldCoords( Vec4 ndcCoords ) const
{
	Mat44 viewProjection = m_projection.GetTransformMatrixBy( m_view );
	Mat44 clipToWorld = viewProjection;
	MatrixInvert( clipToWorld );

	Vec4 world = clipToWorld.TransformHomogeneousPoint3D( ndcCoords );
	world /= world.w;
	return Vec3( world.x, world.y, world.z );
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
void Camera::SetClearMode( CameraClearFlags clearFlags, Rgba8 color, float depth, unsigned int stencil )
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
	Translate( Vec3( translation2D, 0.f ) );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_depthStencilTarget = texture;
}

//---------------------------------------------------------------------------------------------------------
void Camera::SetPitchYawRollRotationDegrees( float pitch, float yaw, float roll )
{
	Clamp( pitch, -95.f, 85.f );

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
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	float aspect = 16.f / 9.f;
	float halfHeight = height * 0.5f;
	float halfWidth = halfHeight * aspect;

	Vec3 min = Vec3( -halfWidth, -halfHeight, nearZ );
	Vec3 max = Vec3( halfWidth, halfHeight, farZ );

	m_projection = Mat44::CreateOrthographicProjection( min, max );
}


//---------------------------------------------------------------------------------------------------------
void Camera::SetProjectionPerspective( float fieldOfViewDegrees, float nearZ, float farZ )
{
	m_projection = Mat44::CreatePerspectiveProjection( fieldOfViewDegrees, 16.0f / 9.0f, nearZ, farZ );
}


//---------------------------------------------------------------------------------------------------------
void Camera::UpdateCameraUBO()
{
	camera_data_t cameraData;
	cameraData.projection = m_projection;
	
	UpdateViewMatrix();
	cameraData.view = m_view;

	m_uniformBuffer->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

