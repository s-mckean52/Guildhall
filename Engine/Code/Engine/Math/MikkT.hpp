#pragma once
#include <vector>

struct SMikkTSpaceContext;
struct Vertex_PCUTBN;

//---------------------------------------------------------------------------------------------------------
static int GetNumFaces( SMikkTSpaceContext const* context );
static int GetNumberOfVerticiesForFace( SMikkTSpaceContext const* context, const int iFace );
static void GetPositionForFaceVert( SMikkTSpaceContext const* context, float fvPosOut[], const int iFace, const int iVert );
static void GetNormalForFaceVert( SMikkTSpaceContext const* context, float fvNormOut[], const int iFace, const int iVert );
static void GetUVForFaceVert( SMikkTSpaceContext const* context, float fvTexcOut[], const int iFace, const int iVert );


//---------------------------------------------------------------------------------------------------------
static void SetTangent( const SMikkTSpaceContext* context, const float fvTangent[], const float fSign, const int iFace, const int iVert );
void GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& verticies );