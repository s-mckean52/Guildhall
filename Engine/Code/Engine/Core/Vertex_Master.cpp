#include "Engine/Core/Vertex_Master.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/buffer_attribute_t.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC buffer_attribute_t const Vertex_Master::LAYOUT[] = {
	buffer_attribute_t( "POSITION",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_Master, m_position ) ),
	buffer_attribute_t( "COLOR",		BUFFER_FORMAT_R8G8B8A8_UNORM,	offsetof( Vertex_Master, m_color ) ),
	buffer_attribute_t( "TEXCOORD",		BUFFER_FORMAT_VEC2,				offsetof( Vertex_Master, m_uvTexCoords ) ),
	buffer_attribute_t( "NORMAL",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_Master, m_normal ) ),
	buffer_attribute_t( "TANGENT",		BUFFER_FORMAT_VEC3,				offsetof( Vertex_Master, m_tangent ) ),
	buffer_attribute_t( "BITANGENT",	BUFFER_FORMAT_VEC3,				offsetof( Vertex_Master, m_bitangent ) ),
	buffer_attribute_t(),
};


//---------------------------------------------------------------------------------------------------------
Vertex_Master::Vertex_Master( const Vec3& position, const Rgba8& color, const Vec3& tangent, const Vec3& bitangent, const Vec3& normal, const Vec2& uvTexCoords )
{
	m_position = position;
	m_color = color;
	m_uvTexCoords = uvTexCoords;

	m_tangent = tangent;
	m_bitangent = bitangent;
	m_normal = normal;
}


//---------------------------------------------------------------------------------------------------------
STATIC Vertex_PCU Vertex_Master::ToVertexPCU( const Vertex_Master& vertexToConvert )
{
	Vec3	position = vertexToConvert.m_position;
	Rgba8	color = vertexToConvert.m_color;
	Vec2	uvTexCoords = vertexToConvert.m_uvTexCoords;

	return Vertex_PCU( position, color, uvTexCoords );
}


//---------------------------------------------------------------------------------------------------------
STATIC Vertex_PCUTBN Vertex_Master::ToVertexPCUTBN( const Vertex_Master& vertexToConvert )
{
	Vec3	position = vertexToConvert.m_position;
	Rgba8	color = vertexToConvert.m_color;
	Vec2	uvTexCoords = vertexToConvert.m_uvTexCoords;

	Vec3	normal = vertexToConvert.m_normal;
	Vec3	tangent = vertexToConvert.m_tangent;
	Vec3	bitangent = vertexToConvert.m_bitangent;

	return Vertex_PCUTBN( position, color, tangent, bitangent, normal, uvTexCoords );
}