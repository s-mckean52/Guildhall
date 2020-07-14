#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

struct RaycastResult
{
public:
	RaycastResult() = default;
	~RaycastResult() {};

	RaycastResult( 	Vec2 const& setStartPosition,
					Vec2 const& setForwardNormal,
					float		setMaxDistance,
					Vec2		setImpactPosition,
					bool		setDidImpact		= false,
					float		setImpactDistance	= 0.f, 
					Vec2		setImpactNormal		= Vec2::ZERO,
					Entity*		setImpactEntity		= nullptr );


public:
	Vec2	startPosition;
	Vec2	forwardNormal;
	Vec2	impactPosition;
	Vec2	impactNormal	= Vec2::ZERO;
	float	maxDistance		= 0.f;
	float	impactDistance	= 0.f;
	bool	didImpact		= false;
	Entity* impactEntity	= nullptr;
};