#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

struct buffer_attribute_t;


//---------------------------------------------------------------------------------------------------------
struct Vertex_PCU
{
public:
	Vec3	m_position;
	Rgba8	m_color;
	Vec2	m_uvTexCoords;

	static const buffer_attribute_t LAYOUT[];

public:
	~Vertex_PCU() {};
	Vertex_PCU() {};
	explicit Vertex_PCU( const Vec2& position, const Rgba8& color, const Vec2& uvTexCoords = Vec2( 0.f, 0.f ) );
	explicit Vertex_PCU( const Vec3& position, const Rgba8& color, const Vec2& uvTexCoords = Vec2( 0.f, 0.f ) );
};