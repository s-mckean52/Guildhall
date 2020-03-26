#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Polygon2D.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"


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
void AppendVertsForQuad3D( std::vector<Vertex_PCU>& vertexArray, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Rgba8& color, const Vec2& uvAtMins, const Vec2& uvAtMaxes )
{
	Vec2 uvAtBottomRight( uvAtMaxes.x, uvAtMins.y );
	Vec2 uvAtTopLeft( uvAtMins.x, uvAtMaxes.y );

	vertexArray.push_back( Vertex_PCU( p0, color, uvAtMins));
	vertexArray.push_back( Vertex_PCU( p1, color, uvAtBottomRight));
	vertexArray.push_back( Vertex_PCU( p2, color, uvAtMaxes));
						   
	vertexArray.push_back( Vertex_PCU( p0, color, uvAtMins));
	vertexArray.push_back( Vertex_PCU( p2, color, uvAtMaxes));
	vertexArray.push_back( Vertex_PCU( p3, color, uvAtTopLeft));
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForAABB3D(std::vector<Vertex_PCU>& vertexArray, const AABB3& box, const Rgba8& color, const Vec2& uvAtMins, const Vec2& uvAtMaxes )
{
	UNUSED( uvAtMins );
	UNUSED( uvAtMaxes );

	Vec3 frontBottomLeft = box.mins;
	Vec3 frontBottomRight = Vec3( box.maxes.x, box.mins.y, box.mins.z );
	Vec3 frontTopLeft = Vec3( box.mins.x, box.maxes.y, box.mins.z );
	Vec3 frontTopRight = Vec3( box.maxes.x, box.maxes.y, box.mins.z );

	//looking At Back
	Vec3 backBottomLeft = Vec3( box.maxes.x, box.mins.y, box.maxes.z );
	Vec3 backBottomRight = Vec3( box.mins.x, box.mins.y, box.maxes.z );
	Vec3 backTopLeft = Vec3( box.maxes.x, box.maxes.y, box.maxes.z );
	Vec3 backTopRight = Vec3( box.mins.x, box.maxes.y, box.maxes.z );


	//frontFace
	vertexArray.push_back( Vertex_PCU( frontBottomLeft,		color ) );
	vertexArray.push_back( Vertex_PCU( frontBottomRight,	color ) );
	vertexArray.push_back( Vertex_PCU( frontTopRight,		color ) );
						   			   				
	vertexArray.push_back( Vertex_PCU( frontBottomLeft,		color ) );
	vertexArray.push_back( Vertex_PCU( frontTopRight,		color ) );
	vertexArray.push_back( Vertex_PCU( frontTopLeft,		color ) );

	//Right Face
	vertexArray.push_back( Vertex_PCU( frontBottomRight,	color ) );
	vertexArray.push_back( Vertex_PCU( backBottomLeft,		color ) );
	vertexArray.push_back( Vertex_PCU( backTopLeft,			color ) );
						   			   				
	vertexArray.push_back( Vertex_PCU( frontBottomRight,	color ) );
	vertexArray.push_back( Vertex_PCU( backTopLeft,			color ) );
	vertexArray.push_back( Vertex_PCU( frontTopRight,		color ) );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForLineBetweenPoints( std::vector<Vertex_PCU>& lineVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness )
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
void AppendVertsForArrowBetweenPoints( std::vector<Vertex_PCU>& arrowVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness )
{
	Vec2 displacement = endPosition - startPosition;
	float halfThickness = thickness * 0.5f;
	float lineSegmentThickness = halfThickness * 0.5f;

	Vec2 forwardVector = displacement.GetNormalized();
	Vec2 leftVector = forwardVector.GetRotated90Degrees();

	Vec2 lineSegmentEndPosition = endPosition - ( forwardVector * thickness );

	//Add lineSegment
	Vec2 lineSegmentStartLeft = startPosition + ( ( -forwardVector + leftVector ) * lineSegmentThickness );
	Vec2 lineSegmentStartRight = startPosition + ( ( -forwardVector - leftVector ) * lineSegmentThickness );
	Vec2 lineSegmentEndLeft = lineSegmentEndPosition + ( leftVector * lineSegmentThickness );
	Vec2 lineSegmentEndRight = lineSegmentEndPosition + ( -leftVector * lineSegmentThickness );

	//Add Triangle
	Vec2 triangleLeft = lineSegmentEndPosition + ( leftVector * halfThickness );
	Vec2 triangleRight = lineSegmentEndPosition + ( -leftVector * halfThickness );
	Vec2 triangleTip = lineSegmentEndPosition + ( forwardVector * thickness );


	arrowVerts.push_back( Vertex_PCU( lineSegmentStartLeft, color ) );
	arrowVerts.push_back( Vertex_PCU( lineSegmentStartRight, color ) );
	arrowVerts.push_back( Vertex_PCU( lineSegmentEndRight, color ) );
						 			 
	arrowVerts.push_back( Vertex_PCU( lineSegmentStartLeft, color ) );
	arrowVerts.push_back( Vertex_PCU( lineSegmentEndRight, color ) );
	arrowVerts.push_back( Vertex_PCU( lineSegmentEndLeft, color ) );

	arrowVerts.push_back( Vertex_PCU( triangleLeft, color ) );
	arrowVerts.push_back( Vertex_PCU( triangleRight, color ) );
	arrowVerts.push_back( Vertex_PCU( triangleTip, color ) );
}


//---------------------------------------------------------------------------------------------------------
void AppendVertsForCircle(std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color, float thickness)
{
	AppendVertsForCircleAtPoint( circleVerts, Vec2::ZERO, radius, color, thickness );
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
void AppendVertsForCircleAtPoint( std::vector<Vertex_PCU>& circleVerts, const Vec2& position, float radius, const Rgba8 color, float thickness )
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

		circleVerts.push_back( Vertex_PCU( currentInnerVertex + position, color ) );
		circleVerts.push_back( Vertex_PCU( currentOuterVertex + position, color ) );
		circleVerts.push_back( Vertex_PCU( nextInnerVertex + position, color ) );

		circleVerts.push_back( Vertex_PCU( currentOuterVertex + position, color ) );
		circleVerts.push_back( Vertex_PCU( nextOuterVertex + position, color ) );
		circleVerts.push_back( Vertex_PCU( nextInnerVertex + position, color ) );

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

	AABB3 cubeMeshAsAABB3;
	cubeMeshAsAABB3.mins = Vec3( -halfSideLength, -halfSideLength, halfSideLength );
	cubeMeshAsAABB3.maxes = Vec3( halfSideLength, halfSideLength, -halfSideLength );

	std::vector<Vertex_PCU> cubeMeshVerts;
	std::vector<unsigned int> cubeMeshIndicies;

	AddBoxToIndexedVertexArray( cubeMeshVerts, cubeMeshIndicies, cubeMeshAsAABB3, Rgba8::WHITE );

	cubeMesh->UpdateVerticies( 24, &cubeMeshVerts[0] );
	cubeMesh->UpdateIndicies( 36, &cubeMeshIndicies[0] );
}


//---------------------------------------------------------------------------------------------------------
void AddBoxToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& box, Rgba8 const& color )
{
	Vec3 frontBottomLeft = box.mins;
	Vec3 frontBottomRight = Vec3( box.maxes.x, box.mins.y, box.mins.z );
	Vec3 frontTopLeft = Vec3( box.mins.x, box.maxes.y, box.mins.z );
	Vec3 frontTopRight = Vec3( box.maxes.x, box.maxes.y, box.mins.z );

	//looking At Back
	Vec3 backBottomLeft = Vec3( box.maxes.x, box.mins.y, box.maxes.z );
	Vec3 backBottomRight = Vec3( box.mins.x, box.mins.y, box.maxes.z );
	Vec3 backTopLeft = Vec3( box.maxes.x, box.maxes.y, box.maxes.z );
	Vec3 backTopRight = Vec3( box.mins.x, box.maxes.y, box.maxes.z );

	std::vector<Vertex_PCU> boxVerticies = {
		//Front
		Vertex_PCU( frontBottomLeft,	color,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( frontBottomRight,	color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( frontTopLeft,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( frontTopRight,		color,	Vec2( 1.f, 1.f ) ),

		//Right
		Vertex_PCU( frontBottomRight,	color,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( backBottomLeft,		color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( frontTopRight,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( backTopLeft,		color,	Vec2( 1.f, 1.f ) ),

		//Back
		Vertex_PCU( backBottomLeft,		color,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( backBottomRight,	color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( backTopLeft,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( backTopRight,		color,	Vec2( 1.f, 1.f ) ),

		//Left
		Vertex_PCU( backBottomRight,	color,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU( frontBottomLeft,	color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( backTopRight,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( frontTopLeft,		color,	Vec2( 1.f, 1.f ) ),

		//Top
		Vertex_PCU( frontTopLeft,		color,	Vec2( 0.f, 0.f ) ),
		Vertex_PCU(	frontTopRight,		color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( backTopRight,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU(	backTopLeft, 		color,	Vec2( 1.f, 1.f ) ),

		//Bottom
		Vertex_PCU( backBottomRight,	color,	Vec2( 1.f, 1.f ) ),
		Vertex_PCU( backBottomLeft,		color,	Vec2( 0.f, 1.f ) ),
		Vertex_PCU( frontBottomLeft,	color,	Vec2( 1.f, 0.f ) ),
		Vertex_PCU( frontBottomRight,	color,	Vec2( 0.f, 0.f ) ),
	};

	std::vector<unsigned int> boxIndicies = {
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

	unsigned int indexOffset = static_cast<unsigned int>( verts.size() );
	for( int vertexIndex = 0; vertexIndex < boxVerticies.size(); ++vertexIndex )
	{
		verts.push_back( boxVerticies[ vertexIndex ] );
	}

	for( int indexIndex = 0; indexIndex < boxIndicies.size(); ++indexIndex )
	{
		indices.push_back( indexOffset + boxIndicies[ indexIndex ] );
	}
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
			vertPosition.z -= cosPsi * sinTheta * radius;

			float u = RangeMapFloat( 0.f, 360.f, 0.f, 1.f, currentThetaDegrees );
			float v = RangeMapFloat( -90.f, 90.f, 0.f, 1.f, currentPsiDegrees );
			Vec2 vertUV = Vec2( u, v );

			verts.push_back( Vertex_PCU( vertPosition, color, vertUV ) );
			currentThetaDegrees += deltaThetaDegrees;
		}

		currentPsiDegrees += deltaPsiDegrees;
	}

	unsigned int indexOffset = static_cast<unsigned int>( indices.size() );
	for( unsigned int verticalIndex = 0; verticalIndex < horizontalCuts; ++verticalIndex )
	{
		for( unsigned int horizontalIndex = 0; horizontalIndex < verticalCuts; ++horizontalIndex )
		{
			unsigned int currentVertIndex = horizontalIndex + ( ( verticalCuts + 1 ) * verticalIndex );

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + verticalCuts + 1;
			unsigned int topRight = currentVertIndex + verticalCuts + 2;

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + bottomRight );
			indices.push_back( indexOffset + topRight );

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + topRight );
			indices.push_back( indexOffset + topLeft );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void AddPlaneToIndexedVertexArray(	std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices,
									Vec3 const& origin, Rgba8 const& color,
									Vec3 const& right, float xMin, float xMax,
									Vec3 const& up, float yMin, float yMax,
									unsigned int xSteps, unsigned int ySteps )
{
	if( ySteps == 0 )
	{
		ySteps = xSteps;
	}

	float xRange = xMax - xMin;
	float yRange = yMax - yMin;

	float xStepAmount = xRange / xSteps;
	float yStepAmount = yRange / ySteps;

	Vec3 xStepDisplacement = right * xStepAmount;
	Vec3 yStepDisplacement = up * yStepAmount;

	float currentHeight = yMin;
	for( unsigned int yStep = 0; yStep < ySteps + 1; ++yStep )
	{
		float currentWidth = xMin;
		for( unsigned int xStep = 0; xStep < xSteps + 1; ++xStep )
		{
			Vec3 currentPosition = origin;
			currentPosition += currentWidth * xStepDisplacement;
			currentPosition += currentHeight * yStepDisplacement;

			float u = RangeMapFloat( xMin, xMax, 0.f, 1.f, currentWidth );
			float v = RangeMapFloat( yMin, yMax, 0.f, 1.f, currentHeight );
			Vec2 uv = Vec2( u, v  );

			verts.push_back( Vertex_PCU( currentPosition, color, uv ) );
			currentWidth += xStepAmount;
		}
		currentHeight += yStepAmount;
	}

	unsigned int indexOffset = static_cast<unsigned int>( indices.size() );
	for( unsigned int yIndex = 0; yIndex < ySteps; ++yIndex )
	{
		for( unsigned int xIndex = 0; xIndex < xSteps; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( ( xSteps + 1 ) * yIndex);

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + xSteps + 1;
			unsigned int topRight = currentVertIndex + xSteps + 2;

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + bottomRight );
			indices.push_back( indexOffset + topRight );

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + topRight );
			indices.push_back( indexOffset + topLeft );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void AddSurfaceToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& origin, Rgba8 const& color, float xMin, float xMax, unsigned int xSteps, float yMin, float yMax, unsigned int ySteps, graph_cb graphFunction )
{
	float xRange = xMax - xMin;
	float yRange = yMax - yMin;

	float xStepAmount = xRange / xSteps;
	float yStepAmount = yRange / ySteps;


	float currentY = yMin;
	for( unsigned int yStep = 0; yStep < ySteps + 1; ++yStep )
	{
		float currentX = xMin;
		for( unsigned int xStep = 0; xStep < xSteps + 1; ++xStep )
		{
			Vec3 currentPosition = origin;
			currentPosition.x += currentX;
			currentPosition.y += currentY;

			float u = RangeMapFloat( xMin, xMax, 0.f, 1.f, currentX );
			float v = RangeMapFloat( yMin, yMax, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v );

			currentPosition += graphFunction( u, v );

			verts.push_back( Vertex_PCU( currentPosition, color, uv ) );
			currentX += xStepAmount;
		}
		currentY += yStepAmount;
	}

	unsigned int indexOffset = static_cast<unsigned int>( indices.size() );
	for( unsigned int yIndex = 0; yIndex < ySteps; ++yIndex )
	{
		for( unsigned int xIndex = 0; xIndex < xSteps; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( (xSteps + 1 ) * yIndex );

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + xSteps + 1;
			unsigned int topRight = currentVertIndex + xSteps + 2;

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + bottomRight );
			indices.push_back( indexOffset + topRight );

			indices.push_back( indexOffset + bottomLeft );
			indices.push_back( indexOffset + topRight );
			indices.push_back( indexOffset + topLeft );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void AddCylinderToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indicies, Vec3 const& startPosition, float startRadius, Vec3 const& endPosition, float endRadius, Rgba8 const& color, unsigned int cuts )
{
	unsigned int indexOffset = static_cast<unsigned int>( verts.size() );
	Mat44 endFaceOrientaiton = Mat44::LookAt( startPosition, endPosition );

	float deltaThetaDegrees = 360.f / cuts;
	float currentThetaDegrees = 0.f;

	for( unsigned int step = 0; step < cuts + 1; ++step )
	{
		Vec3 lookAtIBasis = endFaceOrientaiton.GetIBasis3D();
		Vec3 lookAtJBasis = endFaceOrientaiton.GetJBasis3D();

		float cosTheta = CosDegrees( currentThetaDegrees );
		float sinTheta = SinDegrees( currentThetaDegrees );

		Vec3 startFacePosition = startPosition + ( ( lookAtIBasis * cosTheta ) + ( lookAtJBasis * sinTheta ) ) * startRadius;
		Vec3 endFacePosition = endPosition + ( ( lookAtIBasis * cosTheta ) + ( lookAtJBasis * sinTheta ) ) * endRadius;

		float u = RangeMapFloat( 0.f, 360.f, 0.f, 1.f, currentThetaDegrees );
		Vec2 startVertUV = Vec2( u, 0.f );
		Vec2 endVertUV = Vec2( u, 1.f );

		verts.push_back( Vertex_PCU( startFacePosition, color, startVertUV ) );
		verts.push_back( Vertex_PCU( endFacePosition, color, endVertUV ) );
		currentThetaDegrees += deltaThetaDegrees;
	}

	//Add Indicies for start face
	unsigned int numVertsAdded = cuts * 2;
	for( unsigned int startFaceIndex = 2; startFaceIndex < numVertsAdded - 2; startFaceIndex += 2 )
	{
		indicies.push_back( indexOffset + numVertsAdded );
		indicies.push_back( indexOffset + numVertsAdded - startFaceIndex );
		indicies.push_back( indexOffset + numVertsAdded - ( startFaceIndex + 2 ) );
	}

	//Add Indicies for end face
	for( unsigned int endFaceIndex = 3; endFaceIndex < numVertsAdded - 1; endFaceIndex += 2 )
	{
		indicies.push_back( indexOffset + 1 );
		indicies.push_back( indexOffset + endFaceIndex );
		indicies.push_back( indexOffset + endFaceIndex + 2 );
	}

	for( unsigned int vertIndex = 0; vertIndex < numVertsAdded - 1; vertIndex += 2 )
	{
		unsigned int bottomLeft = vertIndex;
		unsigned int bottomRight = vertIndex + 2;
		unsigned int topLeft = vertIndex + 1;
		unsigned int topRight = vertIndex + 3;

		indicies.push_back( indexOffset + bottomLeft );
		indicies.push_back( indexOffset + bottomRight );
		indicies.push_back( indexOffset + topRight );
			 			    
		indicies.push_back( indexOffset + bottomLeft );
		indicies.push_back( indexOffset + topRight );
		indicies.push_back( indexOffset + topLeft );
	}
}


//---------------------------------------------------------------------------------------------------------
void AddConeToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indicies, Vec3 const& startPosition, float startRadius, Vec3 const& endPosition, Rgba8 const& color, unsigned int cuts )
{
	AddCylinderToIndexedVertexArray( verts, indicies, startPosition, startRadius, endPosition, 0.f, color, cuts );
}
