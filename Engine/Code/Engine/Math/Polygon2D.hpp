#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

struct Polygon2D
{
	Vec2				center;
	std::vector<Vec2>	localVerts;

	// Construction / destructor
	Polygon2D() = default;
	~Polygon2D() = default;
	Polygon2D( const Polygon2D& copyFrom );
	explicit Polygon2D( const std::vector<Vec2> verts, const Vec2& position );

	// Accessors (const methods)
	bool				IsValid() const;
	bool				IsPointInside( const Vec2& point ) const;
	const Vec2			GetCenter() const;
	const Vec2			GetNearestPoint( const Vec2& referentPos ) const;
	float				GetOuterRadius() const;
	float				GetInnerRadius() const;
	std::vector<Vec2>	GetLocalVerts() const;

	// Mutators
	void	AddLocalVert( const Vec2& worldPosition );
	void	SetLocalVerts( std::vector<Vec2> worldVerts );
	void	Translate( const Vec2& translation );
	void	SetCenter( const Vec2& newCenter );
	void	SetOrientationDegrees( float newOrientaiton );
	void	RotateByDegrees( float relativeRotationDegrees );
};