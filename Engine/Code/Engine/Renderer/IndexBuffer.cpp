#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer( RenderContext* context, RenderMemoryHint memHint )
	: RenderBuffer( context, INDEX_BUFFER_BIT, memHint )
{
}


//---------------------------------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{
}


//---------------------------------------------------------------------------------------------------------
void IndexBuffer::Update( unsigned int indexCount, unsigned int const* indicies )
{
	size_t byteSize = indexCount * sizeof( unsigned int );
	size_t elementSize = sizeof( unsigned int );
	RenderBuffer::Update( indicies, byteSize, elementSize );
}


//---------------------------------------------------------------------------------------------------------
void IndexBuffer::Update( std::vector<unsigned int> const& indicies )
{
	Update( static_cast<unsigned int>( indicies.size() ), &indicies[ 0 ] );
}
