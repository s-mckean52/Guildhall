#pragma once
#include <string>

enum BufferFormatType
{
	BUFFER_FORMAT_VEC2,
	BUFFER_FORMAT_VEC3,
	BUFFER_FORMAT_R8G8B8A8_UNORM,
};


//---------------------------------------------------------------------------------------------------------
struct buffer_attribute_t
{
public:
	std::string name = "";
	BufferFormatType type;
	unsigned int offset;

public:
	buffer_attribute_t();
	buffer_attribute_t( char const* n, BufferFormatType t, unsigned int o );

	bool IsDefault() const;
};