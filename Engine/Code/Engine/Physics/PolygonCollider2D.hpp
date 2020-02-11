#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

class PolygonCollider2D : public Collider2D
{
public:
	void SetMembers( Physics2D* physics, std::vector<Vec2> polygonVerts, Vec2 localPosition );

	void Destroy()											override;
	void UpdateWorldShape()									override;

	Vec2 GetClosestPoint( Vec2 const& position ) const		override;
	bool Contains( Vec2 const& position ) const				override;
	bool Intersects( Collider2D const* collider ) const		override;

	void DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor ) override;


private:
	~PolygonCollider2D() override;

public:
	Vec2				m_localPosition;
	Vec2				m_worldPosition;
	std::vector<Vec2>	m_polygonVerts;
};