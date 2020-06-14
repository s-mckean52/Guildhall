#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <vector>

struct Vec2;
struct Vec3;
struct Vertex_PCUTBN;


struct last_indicies_used_t
{
	int lastVertexIndexUsed = -1;
	int lastNormalIndexUsed = -1;
	int lastUVIndexUsed		= -1;
};


//---------------------------------------------------------------------------------------------------------
//Generic File Methods
void*		FileReadToNewBuffer( std::string const& filepath, size_t* out_size );
char const*	FileReadToString( std::string const& filepath );
Strings		GetFileNamesInFolder( std::string const& folderpath, const char* filePattern );
std::string	GetFileNameWithoutExtension( std::string const& filepath );

//---------------------------------------------------------------------------------------------------------
//.obj File Methods
bool IsObjFile( std::string const& filepath );
bool AppendVec3( std::vector<Vec3>& vectorOfVec3, std::string const& string, unsigned int& stringStartIndex );
bool AppendVec2( std::vector<Vec2>& vectorOfVec2, std::string const& string, unsigned int& stringStartIndex );
bool AppendFace( std::vector<Vertex_PCUTBN>& verticies, std::vector<Vec3> const& positions, std::vector<Vec3> const& normals, std::vector<Vec2> const& uvs, std::string const& string, unsigned int& stringStartIndex );
bool CreateVertexPCUTBN( Vec3 const& position, Vec3 const& normal, Vec2 const& uv );
bool ReadAndParseObjFile( std::string const& filepath, std::vector<Vertex_PCUTBN>& verticies );
Vertex_PCUTBN CreateObjVertFromString( std::vector<Vec3> const& positions, std::vector<Vec3> const& normals, std::vector<Vec2> const& uvs, std::string const& faceVertAsString, last_indicies_used_t& lastIndiciesData );