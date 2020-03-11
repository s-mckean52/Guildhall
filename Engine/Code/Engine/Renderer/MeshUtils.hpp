#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>

struct	AABB2;
struct	Vec2;
struct	Rgba8;
struct	OBB2;
class	Polygon2D;
class	GPUMesh;

typedef Vec3( *graph_cb )( float, float );

void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, const Rgba8& color, const Vec2& uvAtMins = Vec2(), const Vec2& uvAtMaxes = Vec2() );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& box, const Rgba8& tint, const Vec2& uvAtMins = Vec2(), const Vec2& uvAtMaxes = Vec2() );
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float radius, const Rgba8& color );
void AppendVertsForLineBetweenPoints( std::vector<Vertex_PCU>& lineVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness );
void AppendVertsForCircle( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color, float thickness );
void AppendVertsForCircleAtPoint( std::vector<Vertex_PCU>& circleVerts, const Vec2& position, float radius, const Rgba8 color, float thickness );
void AppendVertsForFilledCircle( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color );
void AppendVertsForAABB2OutlineAtPoint( std::vector<Vertex_PCU>& vertextArray, const AABB2& box, const Rgba8& color, float thickness );
void AppendVertsForPolygon2DOutline( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color, float thickness );
void AppendVertsForPolygon2DFilled( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color );


//---------------------------------------------------------------------------------------------------------
void AddVerticiesAndIndiciesForCubeMesh( GPUMesh* cubeMesh, float sideLength );
void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& center, float radius, unsigned int horizontalCuts, unsigned int verticalCuts, Rgba8 const& color );
void AddPlaneToIndexedVertexArray(	std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices,
									Vec3 const& origin, Rgba8 const& color,
									Vec3 const& right, float xMin, float xMax,
									Vec3 const& up, float yMin, float yMax,
									unsigned int xSteps = 1, unsigned int ySteps = 0 );

void AddSurfaceToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices,
									 Vec3 const& origin, Rgba8 const& color,
									 float xMin, float xMax, unsigned int xSteps,
									 float yMin, float yMax, unsigned int ySteps,
									 graph_cb graphFunction );