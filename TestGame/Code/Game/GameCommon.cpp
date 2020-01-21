#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness )
{
	std::vector<Vertex_PCU> lineVerts;
	
	AppendVertsForLineBetweenPoints( lineVerts, startPosition, endPosition, color, thickness );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( lineVerts );
}


//---------------------------------------------------------------------------------------------------------
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8 color, float thickness )
{
	std::vector<Vertex_PCU> circleVerts;

	AppendVertsForCircleAtPoint( circleVerts, radius, color, thickness );

	TransformVertexArray( circleVerts, 1.f, 0.f, position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( circleVerts );
}