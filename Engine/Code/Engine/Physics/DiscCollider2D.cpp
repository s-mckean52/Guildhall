#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Math/Polygon2D.hpp"


//---------------------------------------------------------------------------------------------------------
void DiscCollider2D::Destroy()
{
	m_physicsSystem->DestroyCollider2D( this );
}


//---------------------------------------------------------------------------------------------------------
void DiscCollider2D::UpdateWorldShape()
{
	if( m_rigidbody != nullptr )
	{
		m_worldPosition = m_rigidbody->m_worldPosition + m_localPosition;
	}
	else
	{
		m_worldPosition = m_localPosition;
	}
}


//---------------------------------------------------------------------------------------------------------
Vec2 DiscCollider2D::GetClosestPoint( Vec2 const& position ) const
{
	return GetNearestPointOnDisc2D( position, m_worldPosition, m_radius );
}


//---------------------------------------------------------------------------------------------------------
bool DiscCollider2D::Contains( Vec2 const& position ) const
{
	return IsPointInsideDisk2D( position, m_worldPosition, m_radius );
}


//---------------------------------------------------------------------------------------------------------
bool DiscCollider2D::Intersects( Collider2D const* collider ) const
{
	switch( collider->GetType() )
	{
	case COLLIDER_TYPE_DISC2D:
	{
		DiscCollider2D* colliderAsDisc2D = (DiscCollider2D*)collider;
		return DoDiscsOverlap( colliderAsDisc2D->m_worldPosition, colliderAsDisc2D->m_radius, m_worldPosition, m_radius );
	}
	case COLLIDER_TYPE_POLYGON2D:
	{
		PolygonCollider2D* colliderAsPolygon2D = (PolygonCollider2D*)collider;
		return DoPolygonAndDiscOverlap( colliderAsPolygon2D->m_worldPolygon, m_worldPosition, m_radius );
	}
	default:
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
void DiscCollider2D::DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	std::vector< Vertex_PCU > debugVerts;
	AppendVertsForFilledCircle( debugVerts, m_radius, fillColor );
	AppendVertsForCircleAtPoint( debugVerts, m_radius, borderColor, 5.f );
	TransformVertexArray( debugVerts, 1.f, 0.f, m_worldPosition );

	context->BindTexture( nullptr );
	context->DrawVertexArray( debugVerts );
}


//---------------------------------------------------------------------------------------------------------
DiscCollider2D::~DiscCollider2D()
{
}
