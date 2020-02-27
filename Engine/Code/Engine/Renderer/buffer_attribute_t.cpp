#include "Engine/Renderer/buffer_attribute_t.hpp"


//---------------------------------------------------------------------------------------------------------
buffer_attribute_t::buffer_attribute_t()
{
	name = "";
	type = BUFFER_FORMAT_VEC3;
	offset = 0;
}


//---------------------------------------------------------------------------------------------------------
buffer_attribute_t::buffer_attribute_t( char const* n, BufferFormatType t, unsigned int o )
{
	name = n;
	type = t;
	offset = o;
}
