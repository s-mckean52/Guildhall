#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Polygon2D.hpp"
#include "Engine/Renderer/GPUMesh.hpp"


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
void AppendVertsForPolygon2DOutline( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color, float thickness )
{
	Vec2 startVert;
	Vec2 endVert;

	for( int polygonEdgeIndex = 0; polygonEdgeIndex < polygon.GetEdgeCount(); ++polygonEdgeIndex )
	{
		polygon.GetEdge( polygonEdgeIndex, startVert, endVert );
		AppendVertsForLineBetweenPoints( vertexArray, startVert, endVert, color, thickness );
	}
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForPolygon2DFilled( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color )
{
	Vec2 startVert;
	Vec2 secondTriVert;
	Vec2 thirdTriVert;

	int polygonEdgeCount = polygon.GetVertexCount();
	polygon.GetEdge( 0, startVert, secondTriVert );

	for( int polygonEdgeIndex = 1; polygonEdgeIndex < polygonEdgeCount - 1; ++polygonEdgeIndex )
	{
		polygon.GetEdge( polygonEdgeIndex, secondTriVert, thirdTriVert );

		vertexArray.push_back( Vertex_PCU( startVert, color ) );
		vertexArray.push_back( Vertex_PCU( secondTriVert, color ) );
		vertexArray.push_back( Vertex_PCU( thirdTriVert, color ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void AddVerticiesAndIndiciesForCubeMesh( GPUMesh* cubeMesh, float sideLength )
{
	float halfSideLength = sideLength * 0.5f;

	Vertex_PCU cubeVerticies[] ={
		//Front
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, halfSideLength ),		Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),

		//Right
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, halfSideLength ),		Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),

		//Back
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),

		//Left
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),

		//Top
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, halfSideLength ),		Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( Vec3( halfSideLength, halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),

		//Bottom
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 1.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, -halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( Vec3( halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( Vec3( -halfSideLength, -halfSideLength, halfSideLength ),	Rgba8::WHITE,	Vec2( 0.f, 0.f ) ),
	};

	unsigned int cubeIndicies[] ={
		//front
		0, 1, 3,
		0, 3, 2,

		//Right
		4, 5, 7,
		4, 7, 6,

		//Back
		8, 9, 11,
		8, 11, 10,

		//Right
		12, 13, 15,
		12, 15, 14,

		//Top
		16, 17, 19,
		16, 19, 18,

		//Bottom
		20, 21, 23,
		20, 23, 22,
	};

	cubeMesh->UpdateVerticies( 24, cubeVerticies );
	cubeMesh->UpdateIndicies( 36, cubeIndicies );
}


//---------------------------------------------------------------------------------------------------------
void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& center, float radius, unsigned int horizontalCuts, unsigned int verticalCuts, Rgba8 const& color )
{
	float deltaThetaDegrees = 360.f / verticalCuts;
	float deltaPsiDegrees = 180.f / horizontalCuts;

	float currentThetaDegrees = 0.f;
	float currentPsiDegrees = -90.f;

	for( unsigned int verticalStep = 0; verticalStep < horizontalCuts + 1; ++verticalStep )
	{
		currentThetaDegrees = 0.f;
		float cosPsi = CosDegrees( currentPsiDegrees );
		float sinPsi = SinDegrees( currentPsiDegrees );

		for( unsigned int horizontalStep = 0; horizontalStep < verticalCuts + 1; ++horizontalStep )
		{
			float cosTheta = CosDegrees( currentThetaDegrees );
			float sinTheta = SinDegrees( currentThetaDegrees );

			Vec3 vertPosition = center;
			vertPosition.x += cosPsi * cosTheta * radius;
			vertPosition.y += sinPsi * radius;
			vertPosition.z += cosPsi * sinTheta * radius;

			float u = RangeMapFloat( 360.f, 0.f, 0.f, 1.f, currentThetaDegrees );
			float v = RangeMapFloat( -90.f, 90.f, 0.f, 1.f, currentPsiDegrees );
			Vec2 vertUV = Vec2( u, v );

			verts.push_back( Vertex_PCU( vertPosition, color, vertUV ) );
			currentThetaDegrees += deltaThetaDegrees;
		}

		currentPsiDegrees += deltaPsiDegrees;
	}


	for( unsigned int verticalIndex = 0; verticalIndex < horizontalCuts; ++verticalIndex )
	{
		for( unsigned int horizontalIndex = 0; horizontalIndex < verticalCuts; ++horizontalIndex )
		{
			unsigned int currentVertIndex = horizontalIndex + ( ( verticalCuts + 1 ) * verticalIndex );

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + verticalCuts + 1;
			unsigned int topRight = currentVertIndex + verticalCuts + 2;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void AddPlaneToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& startPos, unsigned int horizintalCuts, unsigned int verticalCuts, Rgba8 const& color )
{

	for( unsigned int verticalIndex = 0; verticalIndex < verticalCuts; ++verticalIndex )
	{
		for( unsigned int horizontalIndex = 0; horizontalIndex < horizintalCuts; ++horizontalIndex )
		{
			unsigned int bottomLeft = horizontalIndex;
			unsigned int bottomRight = horizontalIndex + 1;
			unsigned int topLeft = horizontalIndex + ( horizintalCuts * verticalIndex ) + 1;
			unsigned int topRight = horizontalIndex + horizintalCuts + 2;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}
}
