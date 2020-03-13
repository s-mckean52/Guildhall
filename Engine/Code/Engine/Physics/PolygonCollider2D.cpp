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
	float rotationRadians = 0.f;
	if( m_rigidbody != nullptr )
	{
		Vec2 rigidbodyWorldPosition = m_rigidbody->m_worldPosition;
		m_worldPosition = rigidbodyWorldPosition + m_localPosition;
		rotationRadians = m_rigidbody->GetRotationRadians();
	}
	else
	{
		m_worldPosition = m_localPosition;
	}
	m_worldPolygon = m_localPolygon;
	m_worldPolygon = m_worldPolygon.GetRotatedRadians( rotationRadians );
	m_worldPolygon = m_worldPolygon.GetTranslated( m_worldPosition );
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


	for( int pointIndex = 0; pointIndex < m_worldPolygon.GetVertexCount(); ++pointIndex )
	{
		Vec2 polygonVert = m_worldPolygon.GetVertexAtIndex( pointIndex );

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
	m_worldBounds = AABB2( aabbMin, aabbMax );

// 	float longestLength = 0.f;
// 	Vec2 lengthStartPoint;
// 	Vec2 lengthEndPoint;
// 	Vec2 longestLengthDisplacement;
// 
// 	for( int polygonPointIndex = 0; polygonPointIndex < m_worldPolygon.GetVertexCount(); ++polygonPointIndex )
// 	{
// 		Vec2 edgeStart = m_worldPolygon.GetVertexAtIndex( polygonPointIndex );
// 		for (int otherPolygonPoint = polygonPointIndex; otherPolygonPoint < m_worldPolygon.GetVertexCount(); ++otherPolygonPoint)
// 		{
// 			Vec2 edgeEnd = m_worldPolygon.GetVertexAtIndex( otherPolygonPoint );
// 
// 			Vec2 edge = edgeEnd - edgeStart;
// 			float edgeLength = edge.GetLength();
// 
// 			if( edgeLength > longestLength )
// 			{
// 				longestLength = edgeLength;
// 				lengthStartPoint = edgeStart;
// 				lengthEndPoint = edgeEnd;
// 				longestLengthDisplacement = edge;
// 			}
// 		}
// 	}
// 
// 	//Get Second Longest Length from longest start or end
// 	float secondLongestLength = 0.f;
// 	Vec2 thirdPoint;
// 	for( int polygonPointIndex = 0; polygonPointIndex < m_worldPolygon.GetVertexCount(); ++polygonPointIndex )
// 	{
// 		Vec2 edgeEnd = m_worldPolygon.GetVertexAtIndex( polygonPointIndex );
// 
// 		if( edgeEnd != lengthStartPoint && edgeEnd != lengthEndPoint )
// 		{
// 			Vec2 startToThirdEdge = edgeEnd - lengthStartPoint;
// 			Vec2 endToThirdEdge = edgeEnd - lengthEndPoint;
// 
// 			float startToThirdEdgeLength = startToThirdEdge.GetLength();
// 			float endToThirdEdgeLength = endToThirdEdge.GetLength();
// 			if( startToThirdEdgeLength > secondLongestLength )
// 			{
// 				secondLongestLength = startToThirdEdgeLength;
// 				thirdPoint = edgeEnd;
// 			}
// 
// 			if( endToThirdEdgeLength > secondLongestLength )
// 			{
// 				secondLongestLength = endToThirdEdgeLength;
// 				thirdPoint = edgeEnd;
// 			}
// 		}
// 	}
// 
// 	//Get Bounding circle
// 	float dotA = DotProduct2D( lengthStartPoint, lengthStartPoint );
// 	float dotB = DotProduct2D( lengthEndPoint, lengthEndPoint );
// 	float dotC = DotProduct2D( thirdPoint, thirdPoint );
// 
// 	float d = 2.f * ( ( lengthStartPoint.x * ( lengthEndPoint.y - thirdPoint.y ) ) +
// 					  ( lengthEndPoint.x * ( thirdPoint.y - lengthStartPoint.y ) ) +
// 					  ( thirdPoint.x * ( lengthStartPoint.y - lengthEndPoint.y ) ) );
// 
// 	m_worldBoundsCenter.x = ( ( dotA * ( lengthEndPoint.y - thirdPoint.y ) ) +
// 							  ( dotB * ( thirdPoint.y - lengthStartPoint.y ) ) +
// 							  ( dotC * ( lengthStartPoint.y - lengthEndPoint.y ) ) ) / d;
// 	m_worldBoundsCenter.y = ( ( dotA * ( thirdPoint.x - lengthEndPoint.x ) ) +
// 							  ( dotB * ( lengthStartPoint.x - thirdPoint.x ) ) +
// 							  ( dotC * ( lengthEndPoint.x - lengthStartPoint.x ) ) ) / d;
// 
// 	Vec2 uPrime = m_worldBoundsCenter - lengthStartPoint;
// 	float dotUPrime = DotProduct2D( uPrime, uPrime );
// 	m_worldBoundsRadius = sqrtf( dotUPrime );

// 	m_worldBoundsRadius = longestLength * 0.5f;
// 	m_worldBoundsCenter = lengthStartPoint + ( longestLengthDisplacement * 0.5f );

}


//---------------------------------------------------------------------------------------------------------
float PolygonCollider2D::CalculateMoment( float mass )
{
	float moment = 0.f;
	float polygonArea = 0.f;
	Vec2 startPoint = m_localPolygon.GetVertexAtIndex( 0 );
	for( int pointIndex = 1; pointIndex < m_localPolygon.GetVertexCount() - 1; pointIndex += 2 )
	{
		Vec2 secondPoint = m_localPolygon.GetVertexAtIndex( pointIndex );
		Vec2 thirdPoint = m_localPolygon.GetVertexAtIndex( pointIndex + 1 );

		Vec2 u = secondPoint - startPoint;
		Vec2 v = thirdPoint - startPoint;

		float uDotU = DotProduct2D( u, u );
		float vDotU = DotProduct2D( v, u );
		Vec2 h = v - ( ( vDotU / uDotU ) * u );
		float hLength = h.GetLength();
		float hDotH = DotProduct2D( h, h );

		Vec2 center = ( startPoint + secondPoint + thirdPoint ) / 3.f;
		float centerDotCenter = DotProduct2D( center, center );

		float triangleArea = u.GetLength() * hLength * 0.5f;

		float i = ( ( triangleArea / 18 ) * ( uDotU - vDotU  + ( ( vDotU * vDotU ) / uDotU ) + hDotH ) ) + ( triangleArea * centerDotCenter );

		polygonArea += triangleArea;
		moment += i;
	}

	moment *= mass;
	moment /= polygonArea;

	return moment;
}


//---------------------------------------------------------------------------------------------------------
void PolygonCollider2D::DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor )
{
	std::vector< Vertex_PCU > debugVerts;
	AppendVertsForPolygon2DFilled( debugVerts, m_worldPolygon, fillColor );
	AppendVertsForPolygon2DOutline( debugVerts, m_worldPolygon, borderColor, 0.05f );
	//AppendVertsForCircleAtPoint( debugVerts, m_worldBoundsCenter, m_worldBoundsRadius, Rgba8::CYAN, 3.f );
	//AppendVertsForAABB2OutlineAtPoint( debugVerts, m_worldBounds, Rgba8::CYAN, 3.f );

	context->BindTexture( nullptr );
	context->BindShader( (Shader*)nullptr );
	context->DrawVertexArray( debugVerts );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D::~PolygonCollider2D()
{
}
