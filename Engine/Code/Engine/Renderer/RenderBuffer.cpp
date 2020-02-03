#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//TODO: Move to D3D11Common
D3D11_USAGE ToDXMemoryUsage( RenderMemoryHint hint );
UINT ToDXUsage( RenderBufferUsage usage );


//---------------------------------------------------------------------------------------------------------
RenderBuffer::RenderBuffer( RenderContext* owner, RenderBufferUsage usage, RenderMemoryHint memHint )
{
	m_owner = owner;
	m_usage = usage;
	m_memHint = memHint;

	m_handle = nullptr;
	m_bufferByteSize = 0U;
	m_elementByteSize = 0U;
}


//---------------------------------------------------------------------------------------------------------
RenderBuffer::~RenderBuffer()
{
	DX_SAFE_RELEASE( m_handle );
}


//---------------------------------------------------------------------------------------------------------
bool RenderBuffer::IsCompatible( size_t dataByteSize, size_t elementByteSize ) const
{
	if( m_handle == nullptr )
	{
		return false;
	}

	if( m_elementByteSize != elementByteSize )
	{
		return false;
	}

	if( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		return dataByteSize <= m_bufferByteSize;
	}
	else
	{
		return dataByteSize == m_bufferByteSize;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderBuffer::CleanUp()
{
	DX_SAFE_RELEASE( m_handle );
	m_bufferByteSize = 0;
	m_elementByteSize = 0;
}


//---------------------------------------------------------------------------------------------------------
bool RenderBuffer::Update( void const* data, size_t dataByteSize, size_t elementByteSize )
{
	if( !IsCompatible( dataByteSize, elementByteSize ) )
	{
		CleanUp();
	}

	Create( dataByteSize, elementByteSize );

	//Update the buffer
	ID3D11DeviceContext* context = m_owner->m_context;
	if( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		D3D11_MAPPED_SUBRESOURCE mapped;

		HRESULT result = context->Map( m_handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped );
		if( SUCCEEDED( result ) )
		{
			memcpy( mapped.pData, data, dataByteSize );
			context->Unmap( m_handle, 0 );
		}
	}
	else
	{
		context->UpdateSubresource( m_handle, 0, nullptr, data, 0, 0 );
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool RenderBuffer::Create( size_t dataByteSize, size_t elementByteSize )
{
	ID3D11Device* device = m_owner->m_device;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = dataByteSize;
	desc.Usage = ToDXMemoryUsage( m_memHint );
	desc.BindFlags = ToDXUsage( m_usage );
	
	desc.CPUAccessFlags = 0;
	if( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if( m_memHint == MEMORY_HINT_STAGING )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}

	desc.MiscFlags = 0; //Used in special cases
	desc.StructureByteStride = elementByteSize;

	device->CreateBuffer( &desc, nullptr, &m_handle );
	return m_handle != nullptr;
}


//---------------------------------------------------------------------------------------------------------
D3D11_USAGE ToDXMemoryUsage( RenderMemoryHint hint )
{
	switch( hint )
	{
	case MEMORY_HINT_GPU:
		return D3D11_USAGE_DEFAULT;
	case MEMORY_HINT_DYNAMIC:
		return D3D11_USAGE_DYNAMIC;
	case MEMORY_HINT_STAGING:
		return D3D11_USAGE_STAGING;
	default:
		ERROR_AND_DIE( "Unkown hint" );
		//return D3D11_USAGE_DEFAULT;
	}
}


//---------------------------------------------------------------------------------------------------------
UINT ToDXUsage( RenderBufferUsage usage )
{
	UINT ret = 0;

	if( usage & VERTEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_VERTEX_BUFFER;
	}

	if( usage & INDEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_INDEX_BUFFER;
	}

	if( usage & UNIFORM_BUFFER_BIT )
	{
		ret |= D3D11_BIND_CONSTANT_BUFFER;
	}

	return ret;
}


//---------------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( RenderContext* context, RenderMemoryHint memHint )
	: RenderBuffer( context, VERTEX_BUFFER_BIT, memHint )
{
}
