#include "Engine/Math/MikkT.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "ThirdParty/mikkt/mikktspace.h"


//---------------------------------------------------------------------------------------------------------
STATIC int GetNumFaces( SMikkTSpaceContext const* context )
{
	std::vector<Vertex_PCUTBN>& verticies = *(std::vector<Vertex_PCUTBN>*)( context->m_pUserData );
	return static_cast<int>( verticies.size() ) / 3;
}


//---------------------------------------------------------------------------------------------------------
STATIC int GetNumberOfVerticiesForFace( SMikkTSpaceContext const* context, const int iFace )
{
	UNUSED( context );
	UNUSED( iFace );
	return 3;
}


//---------------------------------------------------------------------------------------------------------
STATIC void GetPositionForFaceVert( SMikkTSpaceContext const* context, float fvPosOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& verticies = *(std::vector<Vertex_PCUTBN>*)( context->m_pUserData );
	int vertIndex = ( iFace * 3 ) + iVert;

	Vec3 vertexPosition = verticies[ vertIndex ].m_position;

	fvPosOut[0] = vertexPosition.x;
	fvPosOut[1] = vertexPosition.y;
	fvPosOut[2] = vertexPosition.z;
}


//---------------------------------------------------------------------------------------------------------
STATIC void GetNormalForFaceVert( SMikkTSpaceContext const* context, float fvNormOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& verticies = *(std::vector<Vertex_PCUTBN>*)( context->m_pUserData );
	int vertIndex = ( iFace * 3 ) + iVert;

	Vec3 vertexNormal = verticies[vertIndex].m_normal;

	fvNormOut[0] = vertexNormal.x;
	fvNormOut[1] = vertexNormal.y;
	fvNormOut[2] = vertexNormal.z;
}


//---------------------------------------------------------------------------------------------------------
STATIC void GetUVForFaceVert( SMikkTSpaceContext const* context, float fvTexcOut[], const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& verticies = *(std::vector<Vertex_PCUTBN>*)( context->m_pUserData );
	int vertIndex = ( iFace * 3 ) + iVert;

	Vec2 vertexTexCoords = verticies[ vertIndex ].m_uvTexCoords;

	fvTexcOut[0] = vertexTexCoords.x;
	fvTexcOut[1] = vertexTexCoords.y;
}


//---------------------------------------------------------------------------------------------------------
STATIC void SetTangent( const SMikkTSpaceContext* context, const float fvTangent[], const float fSign, const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *(std::vector<Vertex_PCUTBN>*)( context->m_pUserData );
	int vertIndex = ( iFace * 3 ) + iVert;

	Vec3 normal = vertices[ vertIndex ].m_normal;
	Vec3 tangent = Vec3( fvTangent[0], fvTangent[1], fvTangent[2] );
	Vec3 bitangent = CrossProduct3D( normal, tangent ) * fSign;

	vertices[ vertIndex ].m_tangent = tangent.GetNormalize(); 
	vertices[ vertIndex ].m_bitangent = bitangent.GetNormalize(); 
}


//---------------------------------------------------------------------------------------------------------
void GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& verticies )
{
	SMikkTSpaceInterface interface;
	interface.m_getNumFaces = GetNumFaces;
	interface.m_getNumVerticesOfFace = GetNumberOfVerticiesForFace;

	interface.m_getPosition = GetPositionForFaceVert;
	interface.m_getNormal = GetNormalForFaceVert;
	interface.m_getTexCoord = GetUVForFaceVert;

	interface.m_setTSpaceBasic = SetTangent;
	interface.m_setTSpace = nullptr;

	SMikkTSpaceContext context;
	context.m_pInterface = &interface;
	context.m_pUserData = &verticies;

	genTangSpaceDefault( &context );
}
