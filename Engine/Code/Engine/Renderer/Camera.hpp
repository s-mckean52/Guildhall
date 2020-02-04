#pragma once
#include "Engine/Math/Vec2.hpp"

struct Vec3;

class Camera
{
public:
	~Camera() {};
	Camera() {};

	void	SetOutputSize( Vec2 size );
	void	SetPosition( Vec3 position );
	void	SetProjectionOrthographic( float height, float nearZ = -1.f, float farZ = 1.f );
	void	SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );

	Vec2	GetCameraDimensions() const;
	Vec2	GetOrthoBottomLeft() const;
	Vec2	GetOrthoTopRight() const;
	float	GetAspectRatio() const;

	void	Translate2D( const Vec2& translation2D );
	
	Vec2	ClientToWorldPosition( Vec2 clientPosition );


private: 
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
	Vec2 m_outputSize;
	Vec2 m_position;
};