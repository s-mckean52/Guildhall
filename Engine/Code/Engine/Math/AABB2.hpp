#pragma once
#include "Engine/Math/Vec2.hpp"

struct AABB2
{
public:
	~AABB2()	{};
	AABB2()		{};

	AABB2( const Vec2& minPosition, const Vec2& maxPosition );
	AABB2( float minX, float minY, float maxX, float maxY );

	const Vec2	GetCenter() const;
	const Vec2	GetDimensions() const;
	const Vec2	GetNearestPoint( const Vec2& pointToCheck ) const;
	const Vec2	GetPointAtUV( const Vec2& uvs ) const;
	const Vec2	GetUVForPoint( const Vec2& position ) const ;
	AABB2		GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const;
	AABB2		GetBoxAtLeft( float fractionOfWidth, float additionalWidth = 0.f ) const;
	AABB2		GetBoxAtRight( float fractionOfWidth, float additionalWidth = 0.f ) const;
	AABB2		GetBoxAtBottom( float fractionOfWidth, float additionalWidth = 0.f ) const;
	AABB2		GetBoxAtTop( float fractionOfWidth, float additionalWidth = 0.f ) const;
	float		GetOuterRadius() const;
	float		GetInnerRadius() const;
	void		GetCornerPositions( Vec2* out_fourPoints ) const;

	void		SetCenter( const Vec2& newCenterPoint );
	void		SetDimensions( const Vec2& newDimensions );

	void		Translate( const Vec2& displacementVector );
	void		StretchToIncludePoint( const Vec2& pointToInclude );

	bool		IsPointInside( const Vec2& pointToCheck ) const;

	bool		SetFromText( const char* asText );

public:
	Vec2 mins	= Vec2( 0.f, 0.f );
	Vec2 maxes	= Vec2( 1.f, 1.f );
};
