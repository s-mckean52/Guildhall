#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Actor.hpp"
#include "Game/TileMetaData.hpp"
#include "Game/MapGenStep.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( std::string mapDefinitionName )
{
	m_mapDefinition	= MapDefinition::s_mapDefinitions[ mapDefinitionName ];
	m_name			= m_mapDefinition->GetName();
	m_dimensions	= m_mapDefinition->GetDimensions();

	CreateTiles();
	SpawnActors();
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	m_player->Update( deltaSeconds );
	HandleCollisions();
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleCollisions()
{
	int entityCurrentTileIndex = GetEntityCurrentTile( m_player );

	HandleTileCollision( m_player, entityCurrentTileIndex - m_dimensions.x ); //south
	HandleTileCollision( m_player, entityCurrentTileIndex + 1 ); //east
	HandleTileCollision( m_player, entityCurrentTileIndex + m_dimensions.x ); //north
	HandleTileCollision( m_player, entityCurrentTileIndex - 1 ); //west
						 
	HandleTileCollision( m_player, entityCurrentTileIndex + 1 - m_dimensions.x ); //south-east
	HandleTileCollision( m_player, entityCurrentTileIndex + 1 + m_dimensions.x ); //north-east
	HandleTileCollision( m_player, entityCurrentTileIndex - 1 + m_dimensions.x ); //north-west
	HandleTileCollision( m_player, entityCurrentTileIndex - 1 - m_dimensions.x ); //south-west
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleTileCollision( Entity* entity, int collidedTileIndex )
{
	Tile collidedTile = m_tiles[ collidedTileIndex ];
	TileDefinition* collidedTileDefinition = collidedTile.GetTileDefinition();
	if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && !EntityCanMoveOnTile( entity, collidedTileDefinition ) )
	{
		Vec2 entityPosition = entity->GetPosition();
		float entityRadius = entity->GetPhysicsRadius();
		AABB2 tileBounds = collidedTile.GetWorldBounds();

		PushDiscOutOfAABB2( entityPosition, entityRadius, tileBounds );
		entity->SetPosition( entityPosition );
	}
}


//---------------------------------------------------------------------------------------------------------
int Map::GetEntityCurrentTile( Entity* entity )
{
	return GetTileIndexForWorldPos( entity->GetPosition() );
}


//---------------------------------------------------------------------------------------------------------
bool Map::EntityCanMoveOnTile( Entity* entity, TileDefinition* tileDef )
{
	if( entity->CanFly() && tileDef->DoesAllowFlying() )
		return true;
	else if( entity->CanSwim() && tileDef->DoesAllowSwimming() )
		return true;
	else if( entity->CanWalk() && tileDef->DoesAllowWalking() )
		return true;

	return false;
}


//---------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	RenderTiles();
	m_player->Render();
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateTiles()
{
	TileDefinition* fillTileDef = m_mapDefinition->GetDefaultTileDef();
	TileDefinition* edgeTileDef = m_mapDefinition->GetBorderTileDef();

	// Set all Tiles to default
	m_tiles.clear();
	for( int tileIndex = 0; tileIndex < m_dimensions.x * m_dimensions.y; ++tileIndex )
	{
		int tilePositionY = tileIndex / m_dimensions.x;
		int tilePositionX = tileIndex % m_dimensions.x;

		Tile newTile = Tile( fillTileDef, IntVec2( tilePositionX, tilePositionY ) );
		m_tiles.push_back( newTile );
		m_tileMetaData.push_back( TileMetaData() );
	}

	//---------------------------------------------------------------------------------------------------------
	//   Create Border of boundryType Top and Bottom
	for( int mapXIndex = 0; mapXIndex < m_dimensions.x; ++mapXIndex )
	{
		int bottomTileAtXIndex	= GetTileIndexForTileCoords( IntVec2( mapXIndex, 0 ) );
		int topTileAtXIndex		= GetTileIndexForTileCoords( IntVec2( mapXIndex, m_dimensions.y - 1 ) );

		m_tiles[ bottomTileAtXIndex ].SetTileDefinition( edgeTileDef );
		m_tiles[ topTileAtXIndex ].SetTileDefinition( edgeTileDef );
	}

	//---------------------------------------------------------------------------------------------------------
	//   Create Border of boundryType Left and Right
	for( int mapYIndex = 0; mapYIndex < m_dimensions.y; ++mapYIndex )
	{
		int leftTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( 0, mapYIndex ) );
		int rightTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( m_dimensions.x - 1, mapYIndex ) );

		m_tiles[ leftTileAtYIndex ].SetTileDefinition( edgeTileDef );
		m_tiles[ rightTileAtYIndex ].SetTileDefinition( edgeTileDef );
	}

	//---------------------------------------------------------------------------------------------------------
	// Scatter Edge Tiles Randomly
// 	for( int tileIndex = 0; tileIndex < m_dimensions.x * m_dimensions.y; ++tileIndex )
// 	{
// 		if( g_RNG->RollRandomFloatZeroToOneInclusive() < .15f )
// 		{
// 			m_tiles[ tileIndex ].SetTileDefinition( edgeTileDef );
// 		}
// 	}

	//Run Map Gen Steps
	for( int mapGenStepIndex = 0; mapGenStepIndex < m_mapDefinition->m_mapGenSteps.size(); ++mapGenStepIndex )
	{
		m_mapDefinition->m_mapGenSteps[ mapGenStepIndex ]->RunStep( *this );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
	std::vector<Vertex_PCU> tileVerts;
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		m_tiles[ tileIndex ].AppendVertsForRender( tileVerts );
	}
	g_theRenderer->BindTexture( &g_tileSpriteSheet->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( tileVerts );
}


//---------------------------------------------------------------------------------------------------------
MapDefinition* Map::GetMapDefinition() const
{
	return m_mapDefinition;
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex ) const
{
	return m_tiles[ tileIndex ].GetCoords();
}


//---------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( const IntVec2& tileCoords ) const
{
	return( tileCoords.x + ( m_mapDefinition->GetDimensions().x * tileCoords.y ) );
}


//---------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForWorldPos( Vec2 position ) const
{
	IntVec2 tileCoords = IntVec2( static_cast<int>( position.x ), static_cast<int>( position.y ) );
	return GetTileIndexForTileCoords( tileCoords );
}


//---------------------------------------------------------------------------------------------------------
Tile& Map::GetTileAtIndex( int tileIndex )
{
	return m_tiles[ tileIndex ];
}


//---------------------------------------------------------------------------------------------------------
TileMetaData& Map::GetTileMetaDataAtIndex( int tileIndex )
{
	return m_tileMetaData[ tileIndex ];
}


//---------------------------------------------------------------------------------------------------------
Vec2 Map::GetPlayerPosition()
{
	return m_player->GetPosition();
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnActors()
{
	m_player = new Actor( nullptr, this, Vec2( 2.f, 2.f ), "Player" );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::MoveInRandomCardinalDirection( const IntVec2& tileCoords )
{
	IntVec2 currentTileCoords;
	do
	{
		currentTileCoords = tileCoords;
		CardinalDirection moveDirection = static_cast<CardinalDirection>( g_RNG->RollRandomIntInRange( 0, NUM_CARDINAL_DIRECTIONS - 1 ) );

		switch( moveDirection )
		{
		case NORTH:
			currentTileCoords.y += 1;
			break;
		case EAST:
			currentTileCoords.x += 1;
			break;
		case SOUTH:
			currentTileCoords.y -= 1;
			break;
		case WEST:
			currentTileCoords.x -= 1;
			break;
		default:
			break;
		}
	} while( !IsTileCoordWithinMapBounds( currentTileCoords ) );

	return currentTileCoords;
}


//---------------------------------------------------------------------------------------------------------
void Map::ChangeTilesBasedOnMetaData()
{
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		TileDefinition* tileDefinitionToChangeTo = m_tileMetaData[ tileIndex ].m_tileTypeToChangeTo;
		if( m_tileMetaData[tileIndex].m_tileTypeToChangeTo != nullptr )
		{
			m_tiles[ tileIndex ].SetTileDefinition( tileDefinitionToChangeTo );
			m_tileMetaData[ tileIndex ].m_tileTypeToChangeTo = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsTileCoordWithinMapBounds( const IntVec2& tileCoords )
{
	IntVec2 mapDimensions = m_mapDefinition->GetDimensions();
	bool isWithinBounds = true;
	if( tileCoords.x < 1 || tileCoords.x > mapDimensions.x - 2 )
	{
		isWithinBounds = false;
	}
	else if( tileCoords.y < 1 || tileCoords.y > mapDimensions.y - 2 )
	{
		isWithinBounds = false;
	}
	return isWithinBounds;
}