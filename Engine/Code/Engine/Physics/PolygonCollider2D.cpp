#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::SetMembers( Physics2D* physics, std::vector<Vec2> polygonVerts, Vec2 localPosition )
{
	m_physicsSystem = physics;
	m_polygonVerts = polygonVerts;
	m_localPosition = localPosition;
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::Destroy()
{
	m_physicsSystem->DestroyCollider2D( this );
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::UpdateWorldShape()
{
	if( m_rigidbody != nullptr )
	{
		Vec2 rigidbodyWorldPosition = m_rigidbody->m_worldPosition;
		m_worldPosition = rigidbodyWorldPosition + m_localPosition;
		for( int polygonIndex = 0; polygonIndex < m_polygonVerts.size(); ++polygonIndex )
		{
			m_polygonVerts[ polygonIndex ] -= rigidbodyWorldPosition;
		}
	}
	else
	{
		m_worldPosition = m_localPosition;
	}
}


//---------------------------------------------------------------------------------------------------------
Vec2 PolygonCollider2D::GetClosestPoint( Vec2 const& position ) const
{
	return GetNearestPointOnPolygon2D( position, m_polygonVerts );
}


//---------------------------------------------------------------------------------------------------------
bool PolygonCollider2D::Contains( Vec2 const& position ) const
{
	return IsPointInsidePolygon2D( position, m_polygonVerts );
}


//---------------------------------------------------------------------------------------------------------
bool PolygonCollider2D::Intersects( Collider2D const* collider ) const
{
	switch( collider->m_type )
	{
	case COLLIDER_TYPE_DISC2D:
	{
		DiscCollider2D* colliderAsDisc2D = (DiscCollider2D*)collider;
		return DoPolygonAndDiscOverlap( m_polygonVerts, colliderAsDisc2D->m_worldPosition, colliderAsDisc2D->m_radius );
	}
	case COLLIDER_TYPE_POLYGON2D:
	{
		return false;
		PolygonCollider2D* colliderAsPolygon = (PolygonCollider2D*)collider;
		//return DoPolygonsOverlap( m_polygonVerts, colliderAsPolygon->m_polygonVerts );
	}
	default:
	{
		return false;
	}
	}
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	std::vector< Vertex_PCU > debugVerts;
	AppendVertsForPolygon2DFilled( debugVerts, m_polygonVerts, fillColor );
	AppendVertsForPolygon2DOutline( debugVerts, m_polygonVerts, borderColor, 5.f );
	TransformVertexArray( debugVerts, 1.f, 0.f, m_worldPosition );

	context->BindTexture( nullptr );
	context->DrawVertexArray( debugVerts );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D::~PolygonCollider2D()
{
}
