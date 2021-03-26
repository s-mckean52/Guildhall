#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/Vertex_Master.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
GPUMesh::GPUMesh( RenderContext* context )
{
	AddSubMesh( context );
}

//---------------------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	for( int subMeshIndex = 0; subMeshIndex < m_subMeshes.size(); ++subMeshIndex )
	{
		delete m_subMeshes[subMeshIndex];
		m_subMeshes[subMeshIndex] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
bool GPUMesh::IsValidSubMeshIndex( uint subMeshIndex ) const
{
	if( m_subMeshes.size() > 0 && subMeshIndex < m_subMeshes.size() )
		return true;

	return false;
}


//---------------------------------------------------------------------------------------------------------
VertexBuffer* GPUMesh::GetVertexBuffer( uint subMeshIndex ) const
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return nullptr;

	return m_subMeshes[subMeshIndex]->GetVertexBuffer();
}


//---------------------------------------------------------------------------------------------------------
IndexBuffer* GPUMesh::GetIndexBuffer( uint subMeshIndex ) const
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return nullptr;

	return m_subMeshes[subMeshIndex]->GetIndexBuffer();
}


//---------------------------------------------------------------------------------------------------------
int GPUMesh::GetIndexCount( uint subMeshIndex ) const
{
	if( IsValidSubMeshIndex( subMeshIndex ) )
		return 0;

	return m_subMeshes[subMeshIndex]->GetIndexCount();
}


//---------------------------------------------------------------------------------------------------------
int GPUMesh::GetVertexCount( uint subMeshIndex ) const
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return 0;

	return m_subMeshes[subMeshIndex]->GetVertexCount();
}


//---------------------------------------------------------------------------------------------------------
uint GPUMesh::GetSubMeshCount() const
{
	return static_cast<uint>( m_subMeshes.size() );
}


//---------------------------------------------------------------------------------------------------------
GPUSubMesh* GPUMesh::GetSubMesh( uint subMeshIndex ) const
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return nullptr;

	return m_subMeshes[subMeshIndex];
}


//---------------------------------------------------------------------------------------------------------
GPUSubMesh* GPUMesh::AddSubMesh( RenderContext* context )
{
	GPUSubMesh* newSubMesh = new GPUSubMesh( context );
	m_subMeshes.push_back( newSubMesh );
	return newSubMesh;
}

//---------------------------------------------------------------------------------------------------------
void GPUMesh::UpdateVerticies( uint vertexCount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout, uint subMeshIndex )
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return;

	m_subMeshes[subMeshIndex]->UpdateVerticies( vertexCount, vertexData, vertexStride, layout );
}


//---------------------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndicies( uint indexCount, uint const* indicies, uint subMeshIndex )
{
	if( !IsValidSubMeshIndex( subMeshIndex ) )
		return;

	m_subMeshes[subMeshIndex]->UpdateIndicies( indexCount, indicies );
}
