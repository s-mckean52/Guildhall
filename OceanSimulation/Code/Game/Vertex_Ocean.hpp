#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"

struct buffer_attribute_t;


//---------------------------------------------------------------------------------------------------------
struct Vertex_OCEAN
{
public:
	Vec3	m_position;
	Rgba8	m_color;
	Vec2	m_uvTexCoords;

	Vec3	m_tangent	= Vec3( 1.f, 0.f, 0.f );
	Vec3	m_bitangent = Vec3( 0.f, 1.f, 0.f );
	Vec3	m_normal	= Vec3( 0.f, 0.f, -1.f );

	Vec4	m_jacobian	= Vec4( 1.f, 0.f, 0.f, 0.f );

	static const buffer_attribute_t LAYOUT[];

public:
	~Vertex_OCEAN() {};
	Vertex_OCEAN() {};
	explicit Vertex_OCEAN( const Vec3& position, const Rgba8& color, const Vec3& tangent, const Vec3& bitangent, const Vec3& normal, const Vec2& uvTexCoords = Vec2( 0.f, 0.f ), float jacobian = 1.f );
};