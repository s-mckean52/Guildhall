#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, World* theWorld, char const* imageFilepath )
{
	m_theGame = theGame;
	m_theWorld = theWorld;

	CreateTilesFromImage( imageFilepath );
}


//---------------------------------------------------------------------------------------------------------
Map::~Map()
{
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntites( deltaSeconds );
	UpdateMapVerts( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntites( float deltaSeconds )
{
	for( uint entityIndex = 0; entityIndex < m_entites.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entites[ entityIndex ];
		if( currentEntity != nullptr )
		{
			currentEntity->Update( deltaSeconds );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateMapVerts( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	m_verts.clear();
	m_verts.reserve( m_dimensions.x * m_dimensions.y * 6 );

	for( int i = 0; i < m_tiles.size(); ++i )
	{
		m_tiles[i]->AppendVertsForRender( m_verts );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	SpriteSheet* spriteSheetToUse = TileDefinition::s_spriteSheets["RPGTerrain"];
	Texture const& textureToUse = spriteSheetToUse->GetTexture();
	g_theRenderer->BindTexture( &textureToUse );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( m_verts );

	for( uint entityIndex = 0; entityIndex < m_entites.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entites[ entityIndex ];
		if( currentEntity != nullptr )
		{
			currentEntity->Render();
		}
	}
	
	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateTilesFromImage( char const* filepath )
{
	Image imageToUse = Image( filepath );
	m_dimensions = imageToUse.GetDimensions();
	
	size_t mapSize = m_dimensions.x * m_dimensions.y;
	m_tiles.resize( mapSize );

	for( int i = 0; i < mapSize; ++i )
	{
		int yCoord = i / m_dimensions.x;
		int xCoord = i - ( yCoord * m_dimensions.x );

		Rgba8 texelColor = imageToUse.GetTexelColor( IntVec2( xCoord, m_dimensions.y - 1 - yCoord ) );

		TileDefinition* tileDef = TileDefinition::GetTileDefWithSetColor( texelColor );
		m_tiles[i] = new Tile( tileDef, IntVec2( xCoord, yCoord ) );
	}
}
