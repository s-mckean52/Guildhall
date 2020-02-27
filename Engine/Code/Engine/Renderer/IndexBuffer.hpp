#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"
#include <vector>

class RenderContext;

class IndexBuffer : public RenderBuffer
{
public:
	IndexBuffer( RenderContext* context, RenderMemoryHint memHint );
	~IndexBuffer();

	void Update( unsigned int indexCount, unsigned int const* indicies );
	void Update( std::vector<unsigned int> const& indicies );
};