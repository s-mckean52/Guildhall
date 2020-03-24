#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Transform.hpp"

struct Vec3;

enum CameraClearBitFlag : unsigned int
{
	CLEAR_NONE			= 0,
	CLEAR_COLOR_BIT		= ( 1 << 0 ),
	CLEAR_DEPTH_BIT		= ( 1 << 1 ),
	CLEAR_STENCIL_BIT	= ( 1 << 2 ),
};
typedef unsigned int CameraClearFlags;

class	RenderBuffer;
class	RenderContext;
struct	Texture;
struct	Vec4;

class Camera
{
public:
	Camera( RenderContext* renderer );
	~Camera();

	Rgba8			GetClearColor() const;
	Vec2			GetColorTargetSize() const;
	Vec2			GetCameraDimensions() const;
	Vec3			GetOrthoBottomLeft() const;
	Vec3			GetOrthoTopRight() const;
	Vec3			GetPosition() const;
	Mat44			GetProjectionMatrix() const;
	Texture*		GetColorTarget() const;
	RenderBuffer*	GetUBO() const;
	RenderContext*	GetRenderContext() const		{ return m_renderer; }
	Transform		GetTransform() const			{ return m_transform; }
	Texture*		GetDepthStencilTarget() const	{ return m_depthStencilTarget; }
	float			GetClearDepth() const			{ return m_clearDepth; }

	void	SetPosition( const Vec3& position );
	void	Translate( const Vec3& translation );
	void	SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );

	float	GetAspectRatio() const;

	Vec3	ClientToWorldPosition( Vec2 const& clientPosition, float ndcZ = 0.f ) const;

	bool	ShouldClearColor() const;
	bool	ShouldClearDepth() const;
	Mat44	GetViewMatrix() const;
	Vec3	NDCToWorldCoords( const Vec4& ndcCoords ) const;

	void SetClearMode( CameraClearFlags clearFlags, Rgba8 color, float depth = 1.0f, unsigned int stencil = 0 );
	void SetColorTarget( Texture* texture );
	void Translate2D( const Vec2& translation2D );

	void SetDepthStencilTarget( Texture* texture );
	void SetPitchYawRollRotationDegrees( float pitch, float yaw, float roll );
	void AddPitchYawRollRotationDegrees( float pitch, float yaw, float roll );

	void SetProjectionOrthographic( float height, float nearZ = -1.f, float farZ = 1.f );
	void SetProjectionPerspective( float fov, float nearZ = 0.f, float farZ = 1.f );

	void UpdateCameraUBO();
	void UpdateViewMatrix();

	void SetTransform( Transform transform );
	void SetViewMatrix( Mat44 viewMatrix );
	void SetProjectionMatrix( Mat44 projectionMatrix );

private:
	Mat44 m_projection;
	Mat44 m_view;

	Transform m_transform;

	RenderContext* m_renderer = nullptr;
	Texture* m_colorTarget = nullptr;
	Texture* m_depthStencilTarget = nullptr;

	RenderBuffer* m_uniformBuffer = nullptr;

	CameraClearFlags	m_clearMode		= 0;
	Rgba8				m_clearColor	= Rgba8::WHITE;
	float				m_clearDepth	= 1.0f;
	unsigned int		m_clearStencil	= 0;
};