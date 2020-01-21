#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerEntity.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, IntVec2 mapDimensions )
	: m_theGame( theGame )
{
	m_safeZoneSize = IntVec2( 5, 5 );
	m_mapDimensions = mapDimensions;
	m_tileSize = Vec2( TILE_SIZE, TILE_SIZE );

	CreateTiles();

	m_players.push_back( new PlayerEntity( m_theGame, Vec2( 2.f, 2.f ), 0 ) );
	m_entities.push_back( m_players[ 0 ] );
	//m_players.push_back( new PlayerEntity( m_theGame, Vec2( m_mapDimensions.x - 2.f, 2.f ), 1 ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateTiles()
{

	//Create All Grass
	for( int tileIndex = 0; tileIndex < m_mapDimensions.x * m_mapDimensions.y; ++tileIndex )
	{
		int tilePositionY = tileIndex / m_mapDimensions.x;
		int tilePositionX = tileIndex % m_mapDimensions.x;

		m_tiles.push_back( Tile( IntVec2( tilePositionX, tilePositionY ), TILE_TYPE_GRASS ) );
	}

	//Create Border of Stone Top and Bottom
	for( int mapXIndex = 0; mapXIndex < m_mapDimensions.x; ++mapXIndex )
	{
		int bottomTileAtXIndex	= GetTileIndexForTileCoords( IntVec2( mapXIndex, 0 ) );
		int topTileAtXIndex		= GetTileIndexForTileCoords( IntVec2( mapXIndex, m_mapDimensions.y - 1 ) );

		m_tiles[ bottomTileAtXIndex ].m_tileType = TILE_TYPE_STONE;
		m_tiles[ topTileAtXIndex ].m_tileType = TILE_TYPE_STONE;
	}

	//Create Border of Stone Left and Right
	for( int mapYIndex = 0; mapYIndex < m_mapDimensions.y; ++mapYIndex )
	{
		int leftTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( 0, mapYIndex ) );
		int rightTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( m_mapDimensions.x - 1, mapYIndex ) );

		m_tiles[ leftTileAtYIndex ].m_tileType = TILE_TYPE_STONE;
		m_tiles[ rightTileAtYIndex ].m_tileType = TILE_TYPE_STONE;
	}

	//Randomly Insert Stone Squares
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		if( g_RNG->RollRandomFloatZeroToOneInclusive() <= .35f )
		{
			m_tiles[ tileIndex ].m_tileType = TILE_TYPE_STONE;
		}
	}

	//Create Safe Zone
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		IntVec2 tileCoords = GetTileCoordsForTileIndex( tileIndex );

		//Bottom Left
		if( ( tileCoords.x >= 1 && tileCoords.x < m_safeZoneSize.x + 1 ) && ( tileCoords.y >= 1 && tileCoords.y < m_safeZoneSize.y + 1 ) )
		{
			m_tiles[ tileIndex ].m_tileType = TILE_TYPE_GRASS;
		}
		//Top Right
		else if( ( tileCoords.x < m_mapDimensions.x - 1 && tileCoords.x > m_mapDimensions.x - m_safeZoneSize.x - 1 ) && ( tileCoords.y < m_mapDimensions.y - 1 && tileCoords.y > m_mapDimensions.y - m_safeZoneSize.y - 1 ) )
		{
			m_tiles[ tileIndex ].m_tileType = TILE_TYPE_GRASS;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Render()
{
	m_mapTilesVerts.clear();
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		DrawTile( m_mapTilesVerts, m_tiles[ tileIndex ] );
	}
	g_theRenderer->DrawVertexArray( m_mapTilesVerts );

	for( int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		m_entities[ entityIndex ]->Render();
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( const IntVec2& tileCoords ) const
{
	return( tileCoords.x + ( m_mapDimensions.x * tileCoords.y ) );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex ) const
{
	return m_tiles[ tileIndex ].m_tileCoords;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Map::GetWorldPosForTileCoords( const IntVec2& tileCoords ) const
{
	return Vec2( static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y) );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForWorldPos( const Vec2& worldPos ) const
{
	return IntVec2( static_cast<int>( worldPos.x ), static_cast<int>( worldPos.y ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::DrawTile( std::vector<Vertex_PCU>& tileVerts, const Tile& tileToDraw )
{
	//tileToDraw.Render();
	g_theRenderer->BindTexture( nullptr );
	AppendVertsForAABB2D( tileVerts, tileToDraw.GetBounds(), GetColorForTileType( tileToDraw ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{

	for( int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		m_players[ playerIndex ]->Update( deltaSeconds );
	}

	HandleCollisions();

	DeleteGarbageEntities();
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleCollisions()
{
	if( g_isNoClip ) return;

	for( int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		HandlePlayerCollisions( m_players[playerIndex] );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandlePlayerCollisions( PlayerEntity* player )
{
	int playerCurrentTileIndex = GetEntityCurrentTile( player );
	HandleTileCollision( player, playerCurrentTileIndex + 1 ); //east
	HandleTileCollision( player, playerCurrentTileIndex - 1 ); //west
	HandleTileCollision( player, playerCurrentTileIndex - m_mapDimensions.x ); //south
	HandleTileCollision( player, playerCurrentTileIndex + m_mapDimensions.x ); //north
	HandleTileCollision( player, playerCurrentTileIndex + 1 - m_mapDimensions.x ); //south-east
	HandleTileCollision( player, playerCurrentTileIndex + 1 + m_mapDimensions.x ); //north-east
	HandleTileCollision( player, playerCurrentTileIndex - 1 - m_mapDimensions.x ); //south-west
	HandleTileCollision( player, playerCurrentTileIndex - 1 + m_mapDimensions.x ); //north-west
}


//---------------------------------------------------------------------------------------------------------
bool Map::DoEntitiesOverlap( Entity* A, Entity* B )
{
	if( !A->IsDead() && !B->IsDead() )
	{
		return DoDiscsOverlap( A->GetPosition(), A->GetPhysicsRadius(), B->GetPosition(), B->GetPhysicsRadius() );
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
void Map::DrawLineBetweenEntities( Entity* A, Entity* B ) const
{
	DrawLineBetweenPoints( A->GetPosition(), B->GetPosition(), Rgba8( 50, 50, 50, 255 ), 0.15f );
}


//---------------------------------------------------------------------------------------------------------
void Map::DeleteGarbageEntities()
{
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleTileCollision( Entity* entity, int collidedTileIndex )
{
	Tile collidedTile = m_tiles[ collidedTileIndex ];
	if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && IsTileTypeSolid( collidedTile ) )
	{
		Vec2 entityPosition = entity->GetPosition();
		float entityRadius = entity->GetPhysicsRadius();
		AABB2 tileBounds = collidedTile.GetBounds();

		if( DoDiscAndAABB2Overlap( tileBounds, entityPosition, entityRadius ) )
		{
			Vec2 nearestPointOnTile = GetNearestPointOnAABB2D( entityPosition, tileBounds );
			Vec2 displacement = entityPosition - nearestPointOnTile;
			Vec2 entityCorrection = displacement.GetNormalized() * ( entityRadius - displacement.GetLength() );
			entity->SetPosition( entityPosition + entityCorrection );
		}
	}
	else if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && collidedTile.m_tileType == TILE_TYPE_GOAL )
	{
		entity->Die();
	}
}


//---------------------------------------------------------------------------------------------------------
PlayerEntity* Map::GetPlayer( int playerIndex ) const
{
	return m_players[ playerIndex ];
}


//---------------------------------------------------------------------------------------------------------
int Map::GetEntityCurrentTile( Entity* entity)
{
	IntVec2 entityTileCoords = GetTileCoordsForWorldPos( entity->GetPosition() );
	return GetTileIndexForTileCoords( entityTileCoords );
}