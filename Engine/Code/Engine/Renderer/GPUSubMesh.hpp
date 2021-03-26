#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include <vector>

class VertexBuffer;
class IndexBuffer;
class RenderContext;
struct buffer_attribute_t;

class GPUSubMesh
{
public:
	GPUSubMesh( RenderContext* context );
	~GPUSubMesh();

	VertexBuffer*	GetVertexBuffer() const { return m_vertexBuffer; }
	IndexBuffer*	GetIndexBuffer() const	{ return m_indexBuffer; }
	int				GetIndexCount() const	{ return m_indexCount; }
	int				GetVertexCount() const	{ return m_vertexCount; }

	void UpdateVerticies( unsigned int vertexCount, void const* vertexData, unsigned int vertexStride, buffer_attribute_t const* layout );
	void UpdateIndicies( unsigned int indexCount, unsigned int const* indicies );


	//Helper Template
	template <typename VERTEX_TYPE>
	void UpdateVerticies( unsigned int vertexCount, VERTEX_TYPE const* verticies )
	{
		UpdateVerticies( vertexCount, verticies, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
	}

	template <typename VERTEX_TYPE>
	GPUSubMesh( RenderContext* context, std::vector<VERTEX_TYPE>& verts, std::vector<unsigned int>& indicies )
	{
		m_vertexBuffer = new VertexBuffer( context, MEMORY_HINT_DYNAMIC, VERTEX_TYPE::LAYOUT );
		m_indexBuffer = new IndexBuffer( context, MEMORY_HINT_DYNAMIC );
		UpdateVerticies( static_cast<unsigned int>( verts.size() ), &verts[0] );
		UpdateIndicies( static_cast<unsigned int>( indicies.size() ), &indicies[0] );
	}

public:
	int m_indexCount = 0;
	int m_vertexCount = 0;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
};