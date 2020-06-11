#pragma once
#include "Engine/Math/Vec3.hpp"

struct AABB3
{
public:
	~AABB3()	{};
	AABB3()		{};

	AABB3( const Vec3& minPosition, const Vec3& maxPosition );
	AABB3( float minX, float minY, float minZ,
		   float maxX, float maxY, float maxZ );

	const Vec3	GetCenter() const;
	const Vec3	GetDimensions() const;
	void		SetCenter( const Vec3& newCenterPoint );
	void		SetDimensions( const Vec3& newDimensions );
	void		Translate( const Vec3& displacementVector );

public:
	Vec3 mins	= Vec3::ZERO;
	Vec3 maxes	= Vec3::ONES;
};
