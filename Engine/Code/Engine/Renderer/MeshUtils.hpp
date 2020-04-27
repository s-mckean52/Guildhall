#pragma once
#include "Engine/Core/Vertex_Master.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>

struct	AABB2;
struct	AABB3;
struct	Vec2;
struct	Rgba8;
struct	OBB2;
class	Polygon2D;
class	GPUMesh;

typedef Vec3( *graph_cb )( float, float );

//---------------------------------------------------------------------------------------------------------
struct mesh_import_options_t
{
public:
	Mat44 transform = Mat44::IDENTITY;
	bool invertV = false;
	bool invertWindingOrder = false;
	bool generateNormals = false;
	bool generateTangents = false;

public:
	mesh_import_options_t() {};
};


//---------------------------------------------------------------------------------------------------------
void MeshLoadToVertexArray( std::vector<Vertex_PCUTBN>& vertices, mesh_import_options_t const& options = mesh_import_options_t() );

void MeshInvertV( std::vector<Vertex_PCUTBN>& vertices );
void MeshGenerateNormals( std::vector<Vertex_PCUTBN>& vertices );
void MeshInvertWindingOrder( std::vector<Vertex_PCUTBN>& vertices );
void MeshInvertIndexWindingOrder( std::vector<unsigned int>& indices );
void MeshGenerateTangents( std::vector<Vertex_PCUTBN>& vertices );


//---------------------------------------------------------------------------------------------------------
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, const Rgba8& color, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxes = Vec2::UNIT );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& box, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxes = Vec2::UNIT );
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float radius, const Rgba8& color );
void AppendVertsForLineBetweenPoints( std::vector<Vertex_PCU>& lineVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness );
void AppendVertsForArrowBetweenPoints( std::vector<Vertex_PCU>& arrowVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness );
void AppendVertsForCircle( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color, float thickness );
void AppendVertsForCircleAtPoint( std::vector<Vertex_PCU>& circleVerts, const Vec2& position, float radius, const Rgba8 color, float thickness );
void AppendVertsForFilledCircle( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color );
void AppendVertsForAABB2OutlineAtPoint( std::vector<Vertex_PCU>& vertextArray, const AABB2& box, const Rgba8& color, float thickness );
void AppendVertsForPolygon2DOutline( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color, float thickness );
void AppendVertsForPolygon2DFilled( std::vector<Vertex_PCU>& vertexArray, Polygon2D const& polygon, const Rgba8& color );


//---------------------------------------------------------------------------------------------------------
void AppendMasterVertsToPCUArray( std::vector<Vertex_Master>& masterVertArray, std::vector<Vertex_PCU>& pcuArray );
void AppendMasterVertsToPCUTBNArray( std::vector<Vertex_Master>& masterVertArray, std::vector<Vertex_PCUTBN>& pcutbnArray );

//---------------------------------------------------------------------------------------------------------
// Quad3D
void AppendVertsForQuad3D( std::vector<Vertex_Master>& vertexArray, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Rgba8& color, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxes = Vec2::UNIT );
void AppendVertsForQuad3D( std::vector<Vertex_PCU>& vertexArray, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Rgba8& color, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxes = Vec2::UNIT );
void AppendVertsForQuad3D( std::vector<Vertex_PCUTBN>& vertexArray, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Rgba8& color, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxes = Vec2::UNIT );

//---------------------------------------------------------------------------------------------------------
// Cube
void AddVerticiesAndIndiciesForCubeMesh( GPUMesh* cubeMesh, float sideLength );
void AddBoxToIndexedVertexArray( std::vector<Vertex_Master>& verts, std::vector<unsigned int>& indices, AABB3 const& box, Rgba8 const& color );
void AddBoxToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, AABB3 const& box, Rgba8 const& color );
void AddBoxToIndexedVertexArray( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, AABB3 const& box, Rgba8 const& color );

//---------------------------------------------------------------------------------------------------------
// Sphere
void AddUVSphereToIndexedVertexArray( std::vector<Vertex_Master>& verts, std::vector<unsigned int>& indices, Vec3 const& center, float radius, unsigned int horizontalCuts, unsigned int verticalCuts, Rgba8 const& color );
void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, Vec3 const& center, float radius, unsigned int horizontalCuts, unsigned int verticalCuts, Rgba8 const& color );
void AddUVSphereToIndexedVertexArray( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, Vec3 const& center, float radius, unsigned int horizontalCuts, unsigned int verticalCuts, Rgba8 const& color );

//---------------------------------------------------------------------------------------------------------
// Plane
void AddPlaneToIndexedVertexArray(	std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices,
									Vec3 const& origin, Rgba8 const& color,
									Vec3 const& right, float xMin, float xMax,
									Vec3 const& up, float yMin, float yMax,
									unsigned int xSteps = 1, unsigned int ySteps = 0 );

//---------------------------------------------------------------------------------------------------------
// Surface
void AddSurfaceToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices,
									 Vec3 const& origin, Rgba8 const& color,
									 float xMin, float xMax, unsigned int xSteps,
									 float yMin, float yMax, unsigned int ySteps,
									 graph_cb graphFunction );

//---------------------------------------------------------------------------------------------------------
// Cylinder
void AddCylinderToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indicies,
									  Vec3 const& startPosition, float startRadius,
									  Vec3 const& endPosition, float endRadius,
									  Rgba8 const& color, unsigned int cuts );

//---------------------------------------------------------------------------------------------------------
// Cone
void AddConeToIndexedVertexArray( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indicies,
								  Vec3 const& startPosition, float startRadius,
								  Vec3 const& endPosition,
								  Rgba8 const& color, unsigned int cuts );