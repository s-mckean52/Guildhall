#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Polygon2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

class Polygon2D;

class PolygonCollider2D : public Collider2D
{
public:
	void SetMembers( Physics2D* physics, Vec2 const* localPolygonVerts, unsigned int numVerts, Vec2 localPosition );

	void Destroy()											override;
	void UpdateWorldShape()									override;
	void Move( Vec2 const& movement )						override;

	Vec2	GetClosestPoint( Vec2 const& position ) const		override;
	bool	Contains( Vec2 const& position ) const				override;
	void	SetWorldBounds()									override;

	void DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor ) override;

private:
	~PolygonCollider2D() override;

public:
	Vec2				m_localPosition;
	Vec2				m_worldPosition;
	Polygon2D			m_localPolygon;
	Polygon2D			m_worldPolygon;
};