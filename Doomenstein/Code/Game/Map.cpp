#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, World* theWorld )
{
	m_game = theGame;
	m_world = theWorld;

	std::vector<Vertex_PCUTBN> litCubeVerticies;
	std::vector<uint> litCubeIndicies;
	m_mapMesh = new GPUMesh( g_theRenderer );
}


//---------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_mapMesh;
	m_mapMesh = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Map::Update()
{
	CreateMapVerts();
}


//---------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	g_theRenderer->BindTextureByPath( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	g_theRenderer->BindShader( (Shader*)nullptr );

	g_theRenderer->DrawMesh( m_mapMesh );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileXYCoordsForTileIndex( int tileIndex ) const
{
	int yPos = tileIndex / m_dimensions.x;
	int xPos = tileIndex - ( yPos * m_dimensions.x );
	return IntVec2( xPos, yPos );
}


//---------------------------------------------------------------------------------------------------------
Vec3 Map::GetTilePositionForTileIndex( int tileIndex ) const
{
	int yPos = tileIndex / m_dimensions.x;
	int xPos = tileIndex - ( yPos * m_dimensions.x );
	return Vec3( static_cast<float>( xPos ), static_cast<float>( yPos ), 0.f );
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateMapVerts()
{
	size_t numTiles = m_dimensions.x * m_dimensions.y;
	size_t numFaces = numTiles * 4;
	size_t numVerticies = numFaces * 6;
	m_mapVerts.clear();
	m_mapVerts.reserve( numVerticies );


	for( uint tileIndex = 0; tileIndex < numTiles; ++tileIndex )
	{
		AppendVertsForTile( tileIndex );
 	}

	m_mapMesh->UpdateVerticies( static_cast<uint>( m_mapVerts.size() ), &m_mapVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
void Map::AppendVertsForTile( int tileIndex )
{
	if( IsTileSolid( nullptr ) )
	{
		AppendVertsForSolidTile( tileIndex );
	}
	else if( !IsTileSolid( nullptr ) )
	{
		AppendVertsForOpenTile( tileIndex );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::AppendVertsForOpenTile( int tileIndex )
{
	Rgba8 color = Rgba8::WHITE;

	Vec3 minTilePosition =	GetTilePositionForTileIndex( tileIndex );
	Vec3 frontBottomLeft =	minTilePosition;
	Vec3 frontBottomRight = frontBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 frontTopLeft =		frontBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 frontTopRight =	frontBottomLeft + Vec3( 1.f, 0.f, 1.f );

	//looking from front Back
	Vec3 backBottomLeft =	minTilePosition + Vec3::UNIT_POSITIVE_Y;
	Vec3 backBottomRight =	backBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 backTopLeft =		backBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 backTopRight =		backBottomLeft + Vec3( 1.f, 0.f, 1.f );

	Vec3 frontTangent = ( frontBottomRight - frontBottomLeft ).GetNormalize();
	Vec3 frontBitangent = ( frontTopLeft - frontBottomLeft ).GetNormalize();
	Vec3 frontNormal = CrossProduct3D( frontTangent, frontBitangent );

	//Top																									 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 1.f ) ) );
	
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 1.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight, 		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 1.f ) ) ); 																											  

	//Bottom
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 1.f ) ) );

	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 1.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 1.f ) ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::AppendVertsForSolidTile( int tileIndex )
{
	Rgba8 color = Rgba8::WHITE;

	Vec3 minTilePosition =	GetTilePositionForTileIndex( tileIndex );
	Vec3 frontBottomLeft =	minTilePosition;
	Vec3 frontBottomRight = frontBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 frontTopLeft =		frontBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 frontTopRight =	frontBottomLeft + Vec3( 1.f, 0.f, 1.f );

	//looking from front Back
	Vec3 backBottomLeft =	minTilePosition + Vec3::UNIT_POSITIVE_Y;
	Vec3 backBottomRight =	backBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 backTopLeft =		backBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 backTopRight =		backBottomLeft + Vec3( 1.f, 0.f, 1.f );

	Vec3 frontTangent = ( frontBottomRight - frontBottomLeft ).GetNormalize();
	Vec3 frontBitangent = ( frontTopLeft - frontBottomLeft ).GetNormalize();
	Vec3 frontNormal = CrossProduct3D( frontTangent, frontBitangent );

	//Front
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 1.f ) ) );

	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 1.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 1.f ) ) );

	//Right
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 1.f ) ) );																											 		    

	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 1.f ) ) );																											 		    
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 1.f ) ) );

	//Back																									 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 1.f ) ) );

	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 1.f ) ) );																											 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 1.f ) ) );

	//Left																									 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 1.f ) ) );

	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 0.f ) ) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 1.f ) ) );																									 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 1.f ) ) );
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid( Tile* tile ) const
{
	UNUSED( tile );
	return false;
}
