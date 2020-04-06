#pragma once

class RenderContext;
struct buffer_attribute_t;
struct ID3D11Buffer;

#define BIT_FLAG( b )		( 1 << ( b ) )


enum RenderBufferUsageBit : unsigned int
{
	VERTEX_BUFFER_BIT	= BIT_FLAG( 0 ),
	INDEX_BUFFER_BIT	= BIT_FLAG( 1 ),
	UNIFORM_BUFFER_BIT	= BIT_FLAG( 2 ),
};
typedef unsigned int RenderBufferUsage;


enum RenderMemoryHint : unsigned int
{
	//MEMORY_HINT_STATIC
	MEMORY_HINT_GPU,
	MEMORY_HINT_DYNAMIC,
	MEMORY_HINT_STAGING,
};


//---------------------------------------------------------------------------------------------------------
// A GPU Allocator (like malloc on the GPU
class RenderBuffer
{
public:
	RenderBuffer( RenderContext* owner, RenderBufferUsage usage, RenderMemoryHint memHint );
	~RenderBuffer();

	bool Update( void const* data, size_t dataByteSize, size_t elementByteSize );

private:
	bool IsCompatible( size_t dataByteSize, size_t elementByteSize ) const;
	void CleanUp();
	bool Create( size_t dataByteSize, size_t elementByteSize );


public:
	RenderContext* m_owner = nullptr;
	ID3D11Buffer* m_handle = nullptr;

	RenderBufferUsage m_usage;
	RenderMemoryHint m_memHint;

	size_t m_bufferByteSize;
	size_t m_elementByteSize;
};


//---------------------------------------------------------------------------------------------------------
class VertexBuffer : public RenderBuffer
{
public:
	VertexBuffer( RenderContext* context, RenderMemoryHint memHint, buffer_attribute_t const* bufferAttribute );
	~VertexBuffer();

public:
	buffer_attribute_t const* m_boundBufferAttribute;
};