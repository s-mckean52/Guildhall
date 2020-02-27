#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

class Polygon2D
{
public:
	bool IsValid() const;
	bool IsConvex() const;

	bool	IsPointInside( Vec2 const& point ) const;
	float	GetDistance( Vec2 const& point ) const;
	int		GetClosestEdgeIndex( Vec2 const& point ) const;
	Vec2	GetClosestPoint( Vec2 const& point ) const;
	Vec2	GetClosestPointOnEdge( Vec2 const& point ) const;
	Vec2	GetVertexAtIndex( int index ) const;

	int			GetVertexCount() const;
	int			GetEdgeCount() const;
	void		GetEdge( int edgeNumber, Vec2& out_start, Vec2& out_end ) const;
	Polygon2D	GetTranslated( const Vec2& translation ) const;


public:
	static Polygon2D MakeFromLineLoop( Vec2 const* points, unsigned int pointCount );
	static Polygon2D MakeFromPointCloud( Vec2 const* points, unsigned int pointCount );

private:
	std::vector<Vec2> m_points;
};