#pragma once
#include "Engine/Math/Vec2.hpp"

class Camera
{
private: 
	Vec2 m_bottomLeft;
	Vec2 m_topRight;

public:
	~Camera() {};
	Camera() {};

	Vec2 GetCameraDimensions() const;
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void Translate2D( const Vec2& translation2D );
};