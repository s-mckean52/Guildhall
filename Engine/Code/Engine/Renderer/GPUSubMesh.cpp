#include "Engine/Renderer/GPUSubMesh.hpp"
#include "Engine/Core/Vertex_Master.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
GPUSubMesh::GPUSubMesh( RenderContext* context )
{
	m_vertexBuffer = new VertexBuffer( context, MEMORY_HINT_DYNAMIC, Vertex_Master::LAYOUT );
	m_indexBuffer = new IndexBuffer( context, MEMORY_HINT_DYNAMIC );
}

//---------------------------------------------------------------------------------------------------------
GPUSubMesh::~GPUSubMesh()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void GPUSubMesh::UpdateVerticies( unsigned int vertexCount, void const* vertexData, unsigned int vertexStride, buffer_attribute_t const* layout )
{
	m_vertexCount = vertexCount;
	unsigned int byteSize = vertexCount * vertexStride;
	m_vertexBuffer->Update( vertexData, byteSize, vertexStride );
	m_vertexBuffer->m_boundBufferAttribute = layout;
}


//---------------------------------------------------------------------------------------------------------
void GPUSubMesh::UpdateIndicies( unsigned int indexCount, unsigned int const* indicies )
{
	m_indexCount = indexCount;
	m_indexBuffer->Update( indexCount, indicies );
}