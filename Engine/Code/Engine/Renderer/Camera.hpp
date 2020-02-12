#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"

enum CameraClearBitFlag : unsigned int
{
	CLEAR_COLOR_BIT		= ( 1 << 0 ),
	CLEAR_DEPTH_BIT		= ( 1 << 1 ),
	CLEAR_STENCIL_BIT	= ( 1 << 2 ),
};

struct Texture;

class Camera
{
private: 
	Vec2 m_bottomLeft;
	Vec2 m_topRight;

	Vec3 m_position;

	Mat44 m_projection;
	Mat44 m_view;

	Rgba8			m_clearColor	= Rgba8::BLACK;
	unsigned int	m_clearMode		= 0;

	Texture* m_colorTarget = nullptr;

public:
	~Camera() {};
	Camera() {};

	void SetPosition( const Vec3& position );
	void Translate( const Vec3& translation );

	Rgba8		GetClearColor() const;
	Vec2		GetCameraDimensions() const;
	Vec2		GetOrthoBottomLeft() const;
	Vec2		GetOrthoTopRight() const;
	Mat44		GetProjectionMatrix() const;
	Mat44		GetViewMatrix() const;
	Texture*	GetColorTarget() const;
	bool		ShouldClearColor() const;

	void SetClearMode( unsigned int clearFlags, Rgba8 color, float depth = 0.0f, unsigned int stencil = 0 );
	void SetColorTarget( Texture* texture );

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void Translate2D( const Vec2& translation2D );
};