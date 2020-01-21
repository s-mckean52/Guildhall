#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

struct AABB2;
struct Vec2;
struct Rgba8;
struct OBB2;


void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, const Rgba8& color, const Vec2& uvAtMins = Vec2(), const Vec2& uvAtMaxes = Vec2() );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& box, const Rgba8& tint, const Vec2& uvAtMins = Vec2(), const Vec2& uvAtMaxes = Vec2() );
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float radius, const Rgba8& color );
void AppendVertsForLineBetweenPoints( std::vector<Vertex_PCU>& lineVerts, const Vec2& startPosition, const Vec2& endPosition, const Rgba8 color, float thickness );
void AppendVertsForCircleAtPoint( std::vector<Vertex_PCU>& circleVerts, float radius, const Rgba8 color, float thickness );
void AppendVertsForAABB2OutlineAtPoint( std::vector<Vertex_PCU>& vertextArray, const AABB2& box, const Rgba8& color, float thickness );