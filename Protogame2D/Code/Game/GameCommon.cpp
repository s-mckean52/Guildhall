#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//---------------------------------------------------------------------------------------------------------
void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness )
{
	Vec2 displacement = startPosition - endPosition;
	float halfThickness = thickness * 0.5f;

	Vec2 forwardVector = halfThickness * displacement.GetNormalized();
	Vec2 leftVector = forwardVector.GetRotated90Degrees();

	Vec2 startLeft = startPosition - forwardVector + leftVector;
	Vec2 startRight = startPosition - forwardVector - leftVector;
	Vec2 endLeft = endPosition + forwardVector + leftVector;
	Vec2 endRight = endPosition + forwardVector - leftVector;

	Vertex_PCU lineVerticies[] = 
	{
		Vertex_PCU( startRight, color ),
		Vertex_PCU( endRight, color ),
		Vertex_PCU( endLeft, color ),

		Vertex_PCU( startRight, color ),
		Vertex_PCU( endLeft, color ),
		Vertex_PCU( startLeft, color )
	};

	int lineVerticiesSize = sizeof( lineVerticies ) / sizeof( lineVerticies[0] );

	g_theRenderer->DrawVertexArray( lineVerticiesSize, lineVerticies );
}


//---------------------------------------------------------------------------------------------------------
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8 color, float thickness )
{
	constexpr float angleToRotateDegrees = 360.f / 64.f;
	constexpr int circleVerticiesSize = 6 * 64;
	float halfThickness = 0.5f * thickness;

	Vertex_PCU circleVerticies[ circleVerticiesSize ];

	Vec2 currentRadialPoint( radius, 0.f );
	Vec2 nextRadialPoint = currentRadialPoint.GetRotatedDegrees( angleToRotateDegrees );

	Vec2 currentInnerVertex;
	Vec2 currentOuterVertex;
	Vec2 nextInnerVertex;
	Vec2 nextOuterVertex;

	for( int circleVertexIndex = 0; circleVertexIndex < circleVerticiesSize; circleVertexIndex += 6 )
	{
		currentInnerVertex		= currentRadialPoint.GetNormalized() * (radius - halfThickness);
		currentOuterVertex		= currentRadialPoint.GetNormalized() * (radius + halfThickness);
		nextInnerVertex			= nextRadialPoint.GetNormalized() * (radius - halfThickness);
		nextOuterVertex			= nextRadialPoint.GetNormalized() * (radius + halfThickness);

		circleVerticies[ circleVertexIndex ]		= Vertex_PCU( currentInnerVertex, color );
		circleVerticies[ circleVertexIndex + 1 ]	= Vertex_PCU( currentOuterVertex, color ); 
		circleVerticies[ circleVertexIndex + 2 ]	= Vertex_PCU( nextInnerVertex, color );

		circleVerticies[ circleVertexIndex + 3 ]	= Vertex_PCU( currentOuterVertex, color );
		circleVerticies[ circleVertexIndex + 4 ]	= Vertex_PCU( nextOuterVertex, color );
		circleVerticies[ circleVertexIndex + 5 ]	= Vertex_PCU( nextInnerVertex, color );

		currentRadialPoint = nextRadialPoint;
		nextRadialPoint.RotateDegrees( angleToRotateDegrees );
	}

	TransformVertexArray( circleVerticiesSize, circleVerticies, 1.f, 0.f, position );

	g_theRenderer->DrawVertexArray( circleVerticiesSize, circleVerticies );
}