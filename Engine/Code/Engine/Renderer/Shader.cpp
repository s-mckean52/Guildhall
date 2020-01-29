#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include <d3dcompiler.h>


//---------------------------------------------------------------------------------------------------------
bool ShaderStage::Compile( RenderContext* ctx, std::string const& filename, void const* source, size_t const sourceByteLen, ShaderType stage )
{
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( std::string const& filename )
{
	size_t file_size = 0;
	void* src = FileReadToNewBuffer( filename, &file_size );
	if( src == nullptr )
	{
		return false;
	}

	delete[] src;

	return false;
}


//---------------------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( std::string const& filename, size_t *out_size )
{
	FILE* fp = nullptr;
	fopen_s( &fp, filename.c_str(), "r" );
	if( fp == nullptr )
	{
		return nullptr;
	}

	fseek( fp, 0, SEEK_END );
	long file_size = ftell( fp ) + 1;

	unsigned char* buffer = new unsigned char[ file_size ];
	if( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		size_t bytes_read = fread( buffer, 1, file_size, fp );
		buffer[ bytes_read ] = NULL;

		if( out_size != nullptr )
		{
			*out_size = (size_t)bytes_read;
		}
	}

	fclose( fp );
	return buffer;
}
