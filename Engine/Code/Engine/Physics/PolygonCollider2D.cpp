#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::SetMembers( Physics2D* physics, Vec2 const* localPolygonVerts, unsigned int numVerts, Vec2 localPosition )
{
	m_type = COLLIDER_TYPE_POLYGON2D;
	m_physicsSystem = physics;
	m_localPosition = localPosition;
	m_localPolygon = Polygon2D::MakeFromLineLoop( localPolygonVerts, numVerts );
	m_worldPolygon = m_localPolygon;
	m_physicsMaterial = new PhysicsMaterial();
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
	}
	else
	{
		m_worldPosition = m_localPosition;
	}

	m_worldPolygon = m_localPolygon.GetTranslated( m_worldPosition );
	SetWorldBounds();
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::Move( Vec2 const& movement )
{
	if( m_rigidbody != nullptr )
	{
		m_rigidbody->m_worldPosition += movement;
	}
	else
	{
		m_localPosition += movement;
	}
	UpdateWorldShape();
}


//---------------------------------------------------------------------------------------------------------
Vec2 PolygonCollider2D::GetClosestPoint( Vec2 const& position ) const
{
	return GetNearestPointOnPolygon2D( position, m_worldPolygon );
}


//---------------------------------------------------------------------------------------------------------
bool PolygonCollider2D::Contains( Vec2 const& position ) const
{
	return IsPointInsidePolygon2D( position, m_worldPolygon );
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::SetWorldBounds()
{
	Vec2 aabbMin = Vec2( 1000000.f, 1000000.f );
	Vec2 aabbMax = Vec2( -1000000.f, -1000000.f );


	for( int pointIndex = 0; pointIndex < m_localPolygon.GetVertexCount(); ++pointIndex )
	{
		Vec2 polygonVert = m_localPolygon.GetVertexAtIndex( pointIndex );

		if( polygonVert.x > aabbMax.x )
		{
			aabbMax.x = polygonVert.x; 
		}
		if( polygonVert.x < aabbMin.x )
		{
			aabbMin.x = polygonVert.x;
		}

		if( polygonVert.y > aabbMax.y )
		{
			aabbMax.y = polygonVert.y;
		}
		if( polygonVert.y < aabbMin.y )
		{
			aabbMin.y = polygonVert.y;
		}
	}

	aabbMin += m_worldPosition;
	aabbMax += m_worldPosition;

	m_worldBounds = AABB2( aabbMin, aabbMax );
}


//---------------------------------------------------------------------------------------------------------
float PolygonCollider2D::CalculateMoment(float mass)
{
	return 0.f;
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	std::vector< Vertex_PCU > debugVerts;
	AppendVertsForPolygon2DFilled( debugVerts, m_localPolygon, fillColor );
	AppendVertsForPolygon2DOutline( debugVerts, m_localPolygon, borderColor, 5.f );
	TransformVertexArray( debugVerts, 1.f, 0.f, m_worldPosition );
	//AppendVertsForAABB2OutlineAtPoint( debugVerts, m_worldBounds, Rgba8::CYAN, 3.f );

	context->BindTexture( nullptr );
	context->BindShader( (Shader*)nullptr );
	context->DrawVertexArray( debugVerts );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D::~PolygonCollider2D()
{
}
