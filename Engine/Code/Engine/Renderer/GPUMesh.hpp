#pragma once
#include <vector>

class VertexBuffer;
class IndexBuffer;
class RenderContext;
struct buffer_attribute_t;

class GPUMesh
{
public:
	GPUMesh( RenderContext* context );
	~GPUMesh();

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

public:
	int m_indexCount = 0;
	int m_vertexCount = 0;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
};