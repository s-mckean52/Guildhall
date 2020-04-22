#pragma once
#include <string>
#include <vector>

struct Vec2;
struct Vec3;

//Generic File Methods
void* FileReadToNewBuffer( std::string const& filepath, size_t* out_size );
char const* FileReadToString( std::string const& filepath );

//.obj File Methods
bool IsObjFile( std::string const& filepath );
bool AppendPosition( std::vector<Vec3> const& positions );
bool AppendNormal( std::vector<Vec3> const& normals );
bool AppendUV( std::vector<Vec2> const& uvs );
bool ReadAndParseObjFile( std::string const& filepath );