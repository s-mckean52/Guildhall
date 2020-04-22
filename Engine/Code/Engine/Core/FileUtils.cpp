#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( std::string const& filepath, size_t* out_size )
{
	FILE* fp = nullptr;
	fopen_s( &fp, filepath.c_str(), "r" );
	if( fp == nullptr )
	{
		return nullptr;
	}

	fseek( fp, 0, SEEK_END );
	long file_size = ftell( fp ) + 1;

	unsigned char* buffer = new unsigned char[file_size];
	if (buffer != nullptr)
	{
		fseek(fp, 0, SEEK_SET);
		size_t bytes_read = fread(buffer, 1, file_size, fp);
		buffer[bytes_read] = NULL;

		if (out_size != nullptr)
		{
			*out_size = (size_t)bytes_read;
		}
	}

	fclose(fp);
	return buffer;
}


char const* FileReadToString( std::string const& filepath )
{
	FILE* fp = nullptr;
	fopen_s(&fp, filepath.c_str(), "r");
	if (fp == nullptr)
	{
		return nullptr;
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp) + 1;

	unsigned char* buffer = new unsigned char[file_size];
	if( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		size_t bytes_read = fread( buffer, 1, file_size, fp );
		buffer[bytes_read] = NULL;
	}

	fclose(fp);
	return (char const*)buffer;
}


//---------------------------------------------------------------------------------------------------------
bool IsObjFile( std::string const& filepath )
{
	std::string filetype = filepath.substr( filepath.length() - 4 );
	if( strcmp( filetype.c_str(), ".obj" ) == 0 )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool ReadAndParseObjFile( std::string const& filepath )
{
	GUARANTEE_OR_DIE( IsObjFile( filepath ), "Tried to parse something other than a .obj file" );

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;

	std::string fileAsString = FileReadToString( filepath );

	//Read File line by line
	Strings fileLines = SplitStringOnDelimiter( fileAsString, '\n' );
	for( unsigned int lineIndex = 0; lineIndex < fileLines.size(); ++lineIndex )
	{
		std::string currentLine = fileLines[ lineIndex ];
		char const identifier = currentLine[0];
	}

	return true;
}
