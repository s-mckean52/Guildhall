#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> lineVerts;
	
	AppendVertsForLineBetweenPoints( lineVerts, startPosition, endPosition, color, thickness );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( lineVerts );
}


//---------------------------------------------------------------------------------------------------------
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> circleVerts;

	AppendVertsForCircleAtPoint( circleVerts, position, radius, color, thickness );

	//TransformVertexArray( circleVerts, 1.f, 0.f, position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( circleVerts );
}


//---------------------------------------------------------------------------------------------------------
void DrawAABB2AtPoint( const Vec2& position, const AABB2& box, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> aabb2Verts;
	
	AppendVertsForAABB2OutlineAtPoint( aabb2Verts, box, color, thickness );

	TransformVertexArray( aabb2Verts, 1.f, 0.f, position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( aabb2Verts );
}