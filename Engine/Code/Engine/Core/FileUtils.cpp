#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <vector>
#include <io.h>


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
Strings GetFileNamesInFolder( std::string const& folderpath, const char* filePattern )
{
	Strings fileNamesInFolder;

#ifdef _WIN32
	std::string fileNamePattern = filePattern ? filePattern : "*";
	std::string filePath = folderpath + "/" + fileNamePattern;
	_finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( filePath.c_str(), &fileInfo );
	while( searchHandle != -1 )
	{
		fileNamesInFolder.push_back( fileInfo.name );
		int errorCode = _findnext( searchHandle, &fileInfo );
		if( errorCode != 0 )
			break;
	}
#else
	ERROR_AND_DIE( Stringf( "Not yet implemented for platform!" ) );
#endif

	return fileNamesInFolder;
}


//---------------------------------------------------------------------------------------------------------
std::string GetFileNameWithoutExtension( std::string const& filepath )
{
	size_t nameStartPos = filepath.find_last_of( '/' ) + 1;
	size_t extensionStartPos = filepath.find( '.', nameStartPos );
	size_t fileNameLength = extensionStartPos - nameStartPos;
	std::string filename = filepath.substr( nameStartPos, fileNameLength );
	return filename;
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
	last_indicies_used_t lastIndiciesUsed;
	std::vector<Vertex_PCUTBN> objFaceVerts;
	for( ;; )
	{
		std::string newFaceVert = FindNextWord( string, stringStartIndex );
		if( newFaceVert == "" )
		{
			break;
		}

		objFaceVerts.push_back( CreateObjVertFromString( positions, normals, uvs, newFaceVert, lastIndiciesUsed ) );
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
Vertex_PCUTBN CreateObjVertFromString( std::vector<Vec3> const& positions, std::vector<Vec3> const& normals, std::vector<Vec2> const& uvs, std::string const& faceVertAsString, last_indicies_used_t& lastIndiciesData )
{
	// vertex/texture/normal -> 1/1/1

	Strings faceIndiciesAsText = SplitStringOnDelimiter( faceVertAsString, '/' );

	int positionIndex;
	int uvIndex;
	int normalIndex;
	Vertex_PCUTBN newVertex;

	if( faceIndiciesAsText[0] != "" ) {
		positionIndex = atoi( faceIndiciesAsText[0].c_str() );
		if( positionIndex < 0 )
		{
			positionIndex = static_cast<int>( positions.size() ) + positionIndex;
		}
		else
		{
			positionIndex -= 1;
		}

		lastIndiciesData.lastVertexIndexUsed = positionIndex;
		newVertex.m_position = positions[ positionIndex ];
	} else {
		ERROR_AND_DIE( "No position given for face vertex" );
	}

	if( faceIndiciesAsText[1] != "" ) {
		uvIndex = atoi( faceIndiciesAsText[1].c_str() );
		if( uvIndex < 0 )
		{
			uvIndex = static_cast<int>( uvs.size() ) + uvIndex;
		}
		else
		{
			uvIndex -= 1;
		}
		newVertex.m_uvTexCoords = uvs[ uvIndex ];
		lastIndiciesData.lastUVIndexUsed = uvIndex;
	} else {
		if( lastIndiciesData.lastUVIndexUsed != -1 )
			newVertex.m_uvTexCoords = uvs[ lastIndiciesData.lastUVIndexUsed ];
	}

	if( faceIndiciesAsText.size() >= 3 && faceIndiciesAsText[2] != "" ) {
		normalIndex = atoi( faceIndiciesAsText[2].c_str() );
		if( normalIndex < 0 )
		{
			normalIndex = static_cast<int>( normals.size() ) + normalIndex;
		}
		else
		{
			normalIndex -= 1;
		}
		newVertex.m_normal = normals[ normalIndex ];
		lastIndiciesData.lastNormalIndexUsed = normalIndex;
	} else {
		if( lastIndiciesData.lastNormalIndexUsed != -1 )
			newVertex.m_normal = normals[ lastIndiciesData.lastNormalIndexUsed ];
	}

	return newVertex;
}


//---------------------------------------------------------------------------------------------------------
bool ReadAndParseObjFile( std::string const& filepath, std::vector<Vertex_PCUTBN>& verticies, std::vector<uint>* vertOffsets )
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
		//else if( identifier = 'o' )
		else if( identifier == "g" )
		{
			if( vertOffsets != nullptr )
			{
				uint groupOffset = static_cast<uint>( verticies.size() );
				vertOffsets->push_back( groupOffset );
			}
		}
	}

	return true;
}
