#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec3.hpp"

struct RaycastResult
{
public:
	RaycastResult() = default;
	~RaycastResult() {};

	RaycastResult( 	Vec3 const& setStartPosition,
					Vec3 const& setForwardNormal,
					float		setMaxDistance,
					Vec3		setImpactPosition,
					bool		setDidImpact		= false,
					float		setImpactDistance	= 0.f, 
					Vec3		setImpactNormal		= Vec3::ZERO,
					Entity*		setImpactEntity		= nullptr );


public:
	Vec3	startPosition;
	Vec3	forwardNormal;
	Vec3	impactPosition;
	Vec3	impactNormal	= Vec3::ZERO;
	float	maxDistance		= 0.f;
	float	impactDistance	= 0.f;
	bool	didImpact		= false;
	Entity* impactEntity	= nullptr;
};