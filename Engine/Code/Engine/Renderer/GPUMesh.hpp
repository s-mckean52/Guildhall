#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/GPUSubMesh.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
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

	bool			IsValidSubMeshIndex( uint subMeshIndex ) const;
	uint			GetSubMeshCount() const;
	GPUSubMesh*		GetSubMesh( uint subMeshIndex ) const;

	VertexBuffer*	GetVertexBuffer( uint subMeshIndex = 0 ) const;
	IndexBuffer*	GetIndexBuffer( uint subMeshIndex = 0 ) const;
	int				GetIndexCount( uint subMeshIndex = 0 ) const;
	int				GetVertexCount( uint subMeshIndex = 0 ) const;

	GPUSubMesh* AddSubMesh( RenderContext* context );

	void UpdateVerticies( uint vertexCount, void const* vertexData, uint vertexStride, buffer_attribute_t const* layout, uint subMeshIndex = 0 );
	void UpdateIndicies( uint indexCount, uint const* indicies, uint subMeshIndex = 0 );


	//Helper Template
	template <typename VERTEX_TYPE>
	void UpdateVerticies( unsigned int vertexCount, VERTEX_TYPE const* verticies, uint subMeshIndex = 0 )
	{
		UpdateVerticies( vertexCount, verticies, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT, subMeshIndex );
	}

	template <typename VERTEX_TYPE>
	GPUMesh( RenderContext* context, std::vector<VERTEX_TYPE>& verts, std::vector<uint>& indicies )
	{
		GPUSubMesh* newSubMesh = AddSubMesh( context );
		newSubMesh->UpdateVerticies( static_cast<uint>( verts.size() ), &verts[0] );
		newSubMesh->UpdateIndicies( static_cast<uint>( indicies.size() ), &indicies[0] );
	}

	template <typename VERTEX_TYPE>
	GPUMesh( RenderContext* context, std::vector<VERTEX_TYPE>& verts, std::vector<uint> const& subMeshVertOffsets, uint numSubMeshes )
	{
		for( uint subMeshIndex = 0; subMeshIndex < numSubMeshes; ++subMeshIndex )
		{
			uint subMeshVertSize = 0;
			uint vertStartIndex = subMeshVertOffsets[subMeshIndex];
			if( subMeshIndex != numSubMeshes - 1 )
			{
				subMeshVertSize = subMeshVertOffsets[ subMeshIndex + 1 ] - vertStartIndex;
			}
			else
			{
				subMeshVertSize = static_cast<uint>( verts.size() ) - vertStartIndex;
			}


			GPUSubMesh* newSubMesh = AddSubMesh( context );
			newSubMesh->UpdateVerticies( subMeshVertSize, &verts[vertStartIndex] );
		}
	}

	template <typename VERTEX_TYPE>
	GPUMesh( RenderContext* context, std::vector<VERTEX_TYPE>& verts, std::vector<uint> const& subMeshVertOffsets, std::vector<uint>& indicies, std::vector<uint> const& subMeshIndexOffsets )
	{
		uint numSubMeshes = subMeshVertOffsets.size();
		if( numSubMeshes != subMeshIndexOffsets.size() )
		{
			g_theConsole->ErrorString( "Sub mesh vert and indices offset mismatch" );
			return;
		}
		
		for( uint subMeshIndex = 0; subMeshIndex < numSubMeshes; ++subMeshIndex )
		{
			uint subMeshVertSize = 0;
			uint subMeshIndexSize = 0;
			uint vertStartIndex = subMeshVertOffsets[subMeshIndex];
			uint indexStartIndex = subMeshIndexOffsets[subMeshIndex];
			if( subMeshIndex != numSubMeshes - 1 )
			{
				subMeshVertSize = subMeshVertOffsets[ subMeshIndex + 1 ] - vertStartIndex;
				subMeshIndexSize = subMeshIndexOffsets[ subMeshIndex + 1 ] - indexStartIndex;
			}
			else
			{
				subMeshVertSize = static_cast<uint>( verts.size() ) - vertStartIndex;
				subMeshIndexSize = static_cast<uint>( indicies.size() ) - indexStartIndex;
			}


			GPUSubMesh* newSubMesh = AddSubMesh( context );
			newSubMesh->UpdateVerticies( subMeshVertSize, &verts[vertStartIndex] );
			newSubMesh->UpdateIndicies( subMeshIndexSize, &indicies[indexStartIndex] );
		}
	}

public:
	std::vector<GPUSubMesh*> m_subMeshes;
};