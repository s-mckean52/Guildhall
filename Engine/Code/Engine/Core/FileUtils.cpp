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


//---------------------------------------------------------------------------------------------------------
char const* FileReadToString( std::string const& filepath )
{
	size_t fileSize = 0;
	return (char const*)FileReadToNewBuffer( filepath, &fileSize );
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
bool AppendVec3( std::vector<Vec3>& vectorOfVec3, std::string const& string, unsigned int& stringStartIndex )
{
	std::string xValueAsString = FindNextWord( string, stringStartIndex );
	std::string yValueAsString = FindNextWord( string, stringStartIndex );
	std::string zValueAsString = FindNextWord( string, stringStartIndex );

	Vec3 vector;
	vector.x = static_cast<float>( atof( xValueAsString.c_str() ) );
	vector.y = static_cast<float>( atof( yValueAsString.c_str() ) );
	vector.z = static_cast<float>( atof( zValueAsString.c_str() ) );
	vectorOfVec3.push_back( vector );
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool AppendVec2( std::vector<Vec2>& vectorOfVec2, std::string const& string, unsigned int& stringStartIndex )
{
	std::string xValueAsString = FindNextWord( string, stringStartIndex );
	std::string yValueAsString = FindNextWord( string, stringStartIndex );

	Vec2 vector;
	vector.x = static_cast<float>( atof( xValueAsString.c_str() ) );
	vector.y = static_cast<float>( atof( yValueAsString.c_str() ) );
	vectorOfVec2.push_back( vector );
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool AppendFace( std::vector<Vertex_PCUTBN>& verticies, std::vector<Vec3> const& positions, std::vector<Vec3> const& normals, std::vector<Vec2> const& uvs, std::string const& string, unsigned int& stringStartIndex )
{
	std::vector<Vertex_PCUTBN> objFaceVerts;
	for( ;; )
	{
		std::string newFaceVert = FindNextWord( string, stringStartIndex );
		if( newFaceVert == "" )
		{
			break;
		}

		objFaceVerts.push_back( CreateObjVertFromString( positions, normals, uvs, newFaceVert ) );
	}
	GUARANTEE_OR_DIE( objFaceVerts.size() >= 3 && objFaceVerts.size() <= 4, "Loaded too many verts to a face" );

	if( objFaceVerts.size() == 4 )
	{
		objFaceVerts.insert( objFaceVerts.begin() + 3, objFaceVerts[0] );
		objFaceVerts.insert( objFaceVerts.begin() + 4, objFaceVerts[2] );
	}

	verticies.insert( verticies.end(), objFaceVerts.begin(), objFaceVerts.end() );

	return true;
}


//---------------------------------------------------------------------------------------------------------
Vertex_PCUTBN CreateObjVertFromString( std::vector<Vec3> const& positions, std::vector<Vec3> const& normals, std::vector<Vec2> const& uvs, std::string const& faceVertAsString )
{
	// vertex/texture/normal -> 1/1/1
	static int	lastVertexIndexUsed	= -1;
	static int	lastNormalIndexUsed	= -1;
	static int	lastUVIndexUsed		= -1;

	Strings faceIndiciesAsText = SplitStringOnDelimiter( faceVertAsString, '/' );

	int positionIndex;
	int uvIndex;
	int normalIndex;
	Vertex_PCUTBN newVertex;

	if( faceIndiciesAsText[0] != "" ) {
		positionIndex = atoi( faceIndiciesAsText[0].c_str() );
		if( positionIndex < 0 )
		{
			positionIndex = positions.size() + positionIndex;
		}
		else
		{
			positionIndex -= 1;
		}

		lastVertexIndexUsed = positionIndex;
		newVertex.m_position = positions[ positionIndex ];
	} else {
		ERROR_AND_DIE( "No position given for face vertex" );
	}

	if( faceIndiciesAsText[1] != "" ) {
		uvIndex = atoi( faceIndiciesAsText[1].c_str() );
		if( uvIndex < 0 )
		{
			uvIndex = uvs.size() + uvIndex;
		}
		else
		{
			uvIndex -= 1;
		}
		newVertex.m_uvTexCoords = uvs[ uvIndex ];
		lastUVIndexUsed = uvIndex;
	} else {
		if( lastUVIndexUsed != -1 )
			newVertex.m_uvTexCoords = uvs[ lastUVIndexUsed ];
	}

	if( faceIndiciesAsText[2] != "" ) {
		normalIndex = atoi( faceIndiciesAsText[2].c_str() );
		if( normalIndex < 0 )
		{
			normalIndex = normals.size() + normalIndex;
		}
		else
		{
			normalIndex -= 1;
		}
		newVertex.m_normal = normals[ normalIndex ];
		lastNormalIndexUsed = normalIndex;
	} else {
		if( lastNormalIndexUsed != -1 )
			newVertex.m_normal = normals[ lastNormalIndexUsed ];
	}

	return newVertex;
}


//---------------------------------------------------------------------------------------------------------
bool ReadAndParseObjFile( std::string const& filepath, std::vector<Vertex_PCUTBN>& verticies )
{
	GUARANTEE_OR_DIE( IsObjFile( filepath ), "Tried to parse something other than a .obj file" );

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;

	char const* fileAsString = FileReadToString( filepath );
	Strings fileLines = SplitStringOnDelimiter( fileAsString, '\n' );

	//Read File line by line
	for( unsigned int lineIndex = 0; lineIndex < fileLines.size(); ++lineIndex )
	{
		unsigned int stringIndex = 0;
		std::string currentLine = fileLines[ lineIndex ];
		std::string identifier = FindNextWord( currentLine, stringIndex );
		
		if( identifier == "v" )
		{
			AppendVec3( positions, currentLine, stringIndex );
		}
		else if( identifier == "vn" )
		{
			AppendVec3( normals, currentLine, stringIndex );
		}
		else if( identifier == "vt" )
		{
			AppendVec2( uvs, currentLine, stringIndex );
		}
		else if( identifier == "f" )
		{
			AppendFace( verticies, positions, normals, uvs, currentLine, stringIndex );
		}
	}

	return true;
}
