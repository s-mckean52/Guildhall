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
#include "Engine/Physics/Collision2D.hpp"


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
	SetWorldBounds();
}


//---------------------------------------------------------------------------------------------------------
void DiscCollider2D::Move( Vec2 const& movement )
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
void DiscCollider2D::SetWorldBounds()
{
	Vec2 min;
	Vec2 max;

	min.x = m_worldPosition.x - m_radius;
	min.y = m_worldPosition.y - m_radius;
	
	max.x = m_worldPosition.x + m_radius;
	max.y = m_worldPosition.y + m_radius;
	
	m_worldBounds = AABB2( min, max );
// 	m_worldBoundsRadius = m_radius;
// 	m_worldBoundsCenter = m_worldPosition;
}


//---------------------------------------------------------------------------------------------------------
float DiscCollider2D::CalculateMoment( float mass )
{
	float moment = mass * 0.5f * m_radius * m_radius;
	return moment;
}


//---------------------------------------------------------------------------------------------------------
void DiscCollider2D::DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	float borderThickness = 0.05f;
	float roationDegrees = m_rigidbody->GetRotationDegrees();
	std::vector< Vertex_PCU > debugVerts;
	AppendVertsForFilledCircle( debugVerts, m_radius, fillColor );
	AppendVertsForLineBetweenPoints( debugVerts, Vec2::ZERO, Vec2::RIGHT * m_radius, Rgba8::BLUE, borderThickness * 0.5f );
	AppendVertsForCircle( debugVerts, m_radius, borderColor, borderThickness );
	TransformVertexArray( debugVerts, 1.f, roationDegrees, m_worldPosition );
	//AppendVertsForAABB2OutlineAtPoint( debugVerts, m_worldBounds, Rgba8::CYAN, 3.f );

	context->BindTexture( nullptr );
	context->BindShader( (Shader*)nullptr );
	context->DrawVertexArray( debugVerts );
}


//---------------------------------------------------------------------------------------------------------
DiscCollider2D::~DiscCollider2D()
{
}
