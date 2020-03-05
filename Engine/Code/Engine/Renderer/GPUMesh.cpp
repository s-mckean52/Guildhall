#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"


//---------------------------------------------------------------------------------------------------------
GPUMesh::GPUMesh( RenderContext* context )
{
	m_vertexBuffer = new VertexBuffer( context, MEMORY_HINT_DYNAMIC );
	m_indexBuffer = new IndexBuffer( context, MEMORY_HINT_DYNAMIC );
}


//---------------------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void GPUMesh::UpdateVerticies( unsigned int vertexCount, void const* vertexData, unsigned int vertexStride, buffer_attribute_t const* layout )
{
	m_vertexCount = vertexCount;
	unsigned int byteSize = vertexCount * vertexStride;
	m_vertexBuffer->Update( vertexData, byteSize, vertexStride );
}


//---------------------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndicies( unsigned int indexCount, unsigned int const* indicies )
{
	m_indexCount = indexCount;
	m_indexBuffer->Update( indexCount, indicies );
}