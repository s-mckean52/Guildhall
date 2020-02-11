#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/OBB2.hpp"


//---------------------------------------------------------------------------------------------------------
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxes )
{
	Vec2 bottomRight( box.maxes.x, box.mins.y );
	Vec2 topLeft( box.mins.x, box.maxes.y );

	Vec2 uvAtBottomRight( uvAtMaxes.x, uvAtMins.y );
	Vec2 uvAtTopLeft( uvAtMins.x, uvAtMaxes.y );


	vertexArray.push_back( Vertex_PCU( box.mins,	tint,	uvAtMins		) );
	vertexArray.push_back( Vertex_PCU( bottomRight, tint,	uvAtBottomRight	) );
	vertexArray.push_back( Vertex_PCU( box.maxes,	tint,	uvAtMaxes		) );

	vertexArray.push_back( Vertex_PCU( box.mins,	tint,	uvAtMins		) );
	vertexArray.push_back( Vertex_PCU( box.maxes,	tint,	uvAtMaxes		) );
	vertexArray.push_back( Vertex_PCU( topLeft,		tint,	uvAtTopLeft		) );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForLineBetweenPoints( std::vector<Vertex_PCU>& lineVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness )
{
	Vec2 displacement = endPosition - startPosition;
	float halfThickness = thickness * 0.5f;

	Vec2 forwardVector = halfThickness * displacement.GetNormalized();
	Vec2 leftVector = forwardVector.GetRotated90Degrees();

	Vec2 startLeft = startPosition - forwardVector + leftVector;
	Vec2 startRight = startPosition - forwardVector - leftVector;
	Vec2 endLeft = endPosition + forwardVector + leftVector;
	Vec2 endRight = endPosition + forwardVector - leftVector;

	lineVerts.push_back( Vertex_PCU( startRight, color ) );
	lineVerts.push_back( Vertex_PCU( endRight, color ) );
	lineVerts.push_back( Vertex_PCU( endLeft, color ) );

	lineVerts.push_back( Vertex_PCU( startRight, color ) );
	lineVerts.push_back( Vertex_PCU( endLeft, color ) );
	lineVerts.push_back( Vertex_PCU( startLeft, color ) );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& box, const Rgba8& tint, const Vec2& uvAtMins /*= Vec2()*/, const Vec2& uvAtMaxes /*= Vec2() */ )
{
	Vec2 obb2CornerPoints[ 4 ] = {};
	box.GetCornerPositions( obb2CornerPoints );

	Vec2 uvAtBottomRight( uvAtMaxes.x, uvAtMins.y );
	Vec2 uvAtTopLeft( uvAtMins.x, uvAtMaxes.y );

	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 0 ], tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 1 ], tint, uvAtBottomRight ) );
	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 2 ], tint, uvAtMaxes ) );

	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 0 ], tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 2 ], tint, uvAtMaxes ) );
	vertexArray.push_back( Vertex_PCU( obb2CornerPoints[ 3 ], tint, uvAtTopLeft ) );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float radius, const Rgba8& color )
{
	constexpr int capsuleEndSegments = 32;
	constexpr float angleToRotateDegrees = 360.f / static_cast<float>( capsuleEndSegments * 2 );

	Vec2 capsuleMidDisplacement = capsuleMidEnd - capsuleMidStart;
	Vec2 capsuleMidIBasisNormalized = capsuleMidDisplacement.GetNormalized();
	Vec2 capsuleMidJBasisNormalized = capsuleMidIBasisNormalized.GetRotated90Degrees();

	//Add verts for Half Circle at each end
	Vec2 halfCircleCenter = capsuleMidEnd;
	Vec2 currentRadialDisplacement( radius, 0.f );
	currentRadialDisplacement.SetAngleDegrees( capsuleMidJBasisNormalized.GetAngleDegrees() + 180.f );
	Vec2 nextRadialDisplacement = currentRadialDisplacement.GetRotatedDegrees( angleToRotateDegrees );

	for( int circlePoint = 0; circlePoint < capsuleEndSegments * 2; ++circlePoint )
	{
		vertexArray.push_back( Vertex_PCU( halfCircleCenter, color ) );
		vertexArray.push_back( Vertex_PCU( halfCircleCenter + currentRadialDisplacement, color ) );
		vertexArray.push_back( Vertex_PCU( halfCircleCenter + nextRadialDisplacement, color ) );

		currentRadialDisplacement = nextRadialDisplacement;
		nextRadialDisplacement = currentRadialDisplacement.GetRotatedDegrees( angleToRotateDegrees );

		if( circlePoint == capsuleEndSegments - 1 )
		{
			halfCircleCenter = capsuleMidStart;
		}
	}

	//Add verts for Center OBB2
	Vec2	obb2IBasis		= capsuleMidIBasisNormalized;
	float	obb2Height		= radius * 2.f;
	float	obb2Width		= capsuleMidDisplacement.GetLength();
	Vec2	obb2Center		= capsuleMidStart + ( capsuleMidDisplacement * 0.5f );
	
	OBB2 centerOBB2 = OBB2( obb2Center, Vec2( obb2Width, obb2Height ), obb2IBasis );

	AppendVertsForOBB2D( vertexArray, centerOBB2, color );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForCircleAtPoint( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color, float thickness )
{
	constexpr int circleSegments = 64;
	constexpr float angleToRotateDegrees = 360.f / static_cast<float>( circleSegments );
	float halfThickness = 0.5f * thickness;

	Vec2 currentRadialPoint( radius, 0.f );
	Vec2 nextRadialPoint = currentRadialPoint.GetRotatedDegrees( angleToRotateDegrees );

	Vec2 currentInnerVertex;
	Vec2 currentOuterVertex;
	Vec2 nextInnerVertex;
	Vec2 nextOuterVertex;

	for( int circleSegmentIndex = 0; circleSegmentIndex < circleSegments; ++circleSegmentIndex )
	{
		currentInnerVertex		= currentRadialPoint.GetNormalized() * (radius - halfThickness);
		currentOuterVertex		= currentRadialPoint.GetNormalized() * (radius + halfThickness);
		nextInnerVertex			= nextRadialPoint.GetNormalized() * (radius - halfThickness);
		nextOuterVertex			= nextRadialPoint.GetNormalized() * (radius + halfThickness);

		circleVerts.push_back( Vertex_PCU( currentInnerVertex, color ) );
		circleVerts.push_back( Vertex_PCU( currentOuterVertex, color ) );
		circleVerts.push_back( Vertex_PCU( nextInnerVertex, color ) );

		circleVerts.push_back( Vertex_PCU( currentOuterVertex, color ) );
		circleVerts.push_back( Vertex_PCU( nextOuterVertex, color ) );
		circleVerts.push_back( Vertex_PCU( nextInnerVertex, color ) );

		currentRadialPoint = nextRadialPoint;
		nextRadialPoint.RotateDegrees( angleToRotateDegrees );
	}
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForFilledCircle( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color )
{
	constexpr int circleSegments = 64;
	constexpr float angleToRotateDegrees = 360.f / static_cast<float>(circleSegments);

	Vec2 centerVertex = Vec2( 0.f, 0.f );
	Vec2 currentRadialPoint( radius, 0.f );
	Vec2 nextRadialPoint = currentRadialPoint.GetRotatedDegrees( angleToRotateDegrees );

	Vec2 currentVertex;
	Vec2 nextVertex;

	for( int circleSegmentIndex = 0; circleSegmentIndex < circleSegments; ++circleSegmentIndex )
	{
		currentVertex		= currentRadialPoint.GetNormalized() * radius;
		nextVertex			= nextRadialPoint.GetNormalized() * radius;

		circleVerts.push_back( Vertex_PCU( centerVertex, color ) );
		circleVerts.push_back( Vertex_PCU( currentVertex, color ) );
		circleVerts.push_back( Vertex_PCU( nextVertex, color ) );

		currentRadialPoint = nextRadialPoint;
		nextRadialPoint.RotateDegrees( angleToRotateDegrees );
	}
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForAABB2OutlineAtPoint( std::vector<Vertex_PCU>& vertextArray, const AABB2& box, const Rgba8& color, float thickness )
{
	Vec2 topLeftPoint		= Vec2( box.mins.x, box.maxes.y );
	Vec2 bottomRightPoint	= Vec2( box.maxes.x, box.mins.y );

	AppendVertsForLineBetweenPoints( vertextArray, box.mins, bottomRightPoint, color, thickness );
	AppendVertsForLineBetweenPoints( vertextArray, bottomRightPoint, box.maxes, color, thickness );
	AppendVertsForLineBetweenPoints( vertextArray, box.maxes, topLeftPoint, color, thickness );
	AppendVertsForLineBetweenPoints( vertextArray, topLeftPoint, box.mins, color, thickness );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForPolygon2DOutline( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2> polygonVerts, const Rgba8& color, float thickness )
{
	Vec2 startVert;
	Vec2 endVert;
	for( int polygonVertIndex = 0; polygonVertIndex < polygonVerts.size(); ++polygonVertIndex )
	{
		startVert = polygonVerts[ polygonVertIndex ];
		if( polygonVertIndex != polygonVerts.size() - 1 )
		{
			endVert = polygonVerts[ static_cast<size_t>( polygonVertIndex ) + 1 ];
		}
		else
		{
			endVert = polygonVerts[ 0 ];
		}

		AppendVertsForLineBetweenPoints( vertexArray, startVert, endVert, color, thickness );
	}
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForPolygon2DFilled( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2> polygonVerts, const Rgba8& color )
{
	Vec2 startVert = polygonVerts[ 0 ];
	Vec2 secondTriVert = polygonVerts[ 1 ];
	Vec2 thirdTriVert = polygonVerts[ 2 ];

	for( int polygonVertIndex = 1; polygonVertIndex < polygonVerts.size() - 1; ++polygonVertIndex )
	{
		vertexArray.push_back( Vertex_PCU( startVert, color ) );
		vertexArray.push_back( Vertex_PCU( secondTriVert, color ) );
		vertexArray.push_back( Vertex_PCU( thirdTriVert, color ) );

		int nextVertIndex = ( polygonVertIndex + 2 ) % polygonVerts.size();
		secondTriVert = thirdTriVert;
		thirdTriVert = polygonVerts[ nextVertIndex ];
	}
}
