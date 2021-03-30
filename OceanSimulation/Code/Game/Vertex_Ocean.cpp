#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/buffer_attribute_t.hpp"
#include "Game/Vertex_OCEAN.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC buffer_attribute_t const Vertex_OCEAN::LAYOUT[] = {
	buffer_attribute_t( "POSITION",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_OCEAN, m_position ) ),
	buffer_attribute_t( "COLOR",		BUFFER_FORMAT_R8G8B8A8_UNORM,	offsetof( Vertex_OCEAN, m_color ) ),
	buffer_attribute_t( "TEXCOORD",		BUFFER_FORMAT_VEC2,				offsetof( Vertex_OCEAN, m_uvTexCoords ) ),
	buffer_attribute_t( "NORMAL",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_OCEAN, m_normal ) ),
	buffer_attribute_t( "TANGENT",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_OCEAN, m_tangent ) ),
	buffer_attribute_t( "BITANGENT",	BUFFER_FORMAT_VEC3,				offsetof( Vertex_OCEAN, m_bitangent ) ),
	buffer_attribute_t( "JACOBIAN",		BUFFER_FORMAT_VEC4,			offsetof( Vertex_OCEAN, m_jacobian ) ),
	buffer_attribute_t(),
};


//---------------------------------------------------------------------------------------------------------
Vertex_OCEAN::Vertex_OCEAN( const Vec3& position, const Rgba8& color, const Vec3& tangent, const Vec3& bitangent, const Vec3& normal, const Vec2& uvTexCoords, float jacobian )
{
	m_position = position;
	m_color = color;
	m_uvTexCoords = uvTexCoords;

	m_tangent = tangent;
	m_bitangent = bitangent;
	m_normal = normal;

	m_jacobian.x = jacobian;
}
