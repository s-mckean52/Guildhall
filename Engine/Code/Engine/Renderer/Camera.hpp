#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

enum CameraClearBitFlag : unsigned int
{
	CLEAR_COLOR_BIT		= ( 1 << 0 ),
	CLEAR_DEPTH_BIT		= ( 1 << 1 ),
	CLEAR_STENCIL_BIT	= ( 1 << 2 ),
};

class Camera
{
private: 
	Vec2 m_bottomLeft;
	Vec2 m_topRight;

	Rgba8			m_clearColor	= Rgba8::BLACK;
	unsigned int	m_clearMode		= 0;

public:
	~Camera() {};
	Camera() {};

	Rgba8	GetClearColor() const;
	Vec2	GetCameraDimensions() const;
	Vec2	GetOrthoBottomLeft() const;
	Vec2	GetOrthoTopRight() const;
	bool	ShouldClearColor() const;

	void SetClearMode( unsigned int clearFlags, Rgba8 color, float depth = 0.0f, unsigned int stencil = 0 );

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void Translate2D( const Vec2& translation2D );
};