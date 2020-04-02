#pragma once
#include "Engine/Math/Vec2.hpp"


struct Plane2D
{
public:
	Vec2 normal;
	float distance = 0.f;

public:
	Plane2D( Vec2 const& toNormal, float toDistance );
	Plane2D( Vec2 const& toNormal, Vec2 const& pointOnPlane );
	~Plane2D() = default;

	Vec2	GetOrigin();
	float	GetPointsDistanceFromPlane( Vec2 const& refPoint );
	bool	IsPointInFrontOfPlane( Vec2 const& refPoint );
};