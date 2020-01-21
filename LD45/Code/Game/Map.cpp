#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, IntVec2 mapDimensions )
	: m_theGame( theGame )
{
	m_safeZoneSize = IntVec2( 6, 6 );
	m_mapDimensions = mapDimensions;
	m_tileSize = Vec2( TILE_SIZE, TILE_SIZE );

	CreateTiles();

	m_players.push_back( new PlayerController( m_theGame, Vec2( 2.f, 2.f ), 0 ) );
	m_players.push_back( new PlayerController( m_theGame, Vec2( m_mapDimensions.x - 2.f, 2.f ), 1 ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateTiles()
{
	TileType currentTileType = INVALID_TILE_TYPE;
	bool isGoalSpawned = false;

	for( int indexY = 0; indexY < m_mapDimensions.y; ++indexY )
	{
		for( int indexX = 0; indexX < m_mapDimensions.x; ++indexX )
		{
			if( indexY == 0 || indexY == m_mapDimensions.y - 1 || indexX == 0 || indexX == m_mapDimensions.x - 1 )
			{
				currentTileType = TILE_TYPE_STONE;
			}
			else
			{
				if( g_RNG->GetRandomFloatZeroToOneInclusive() <= .35f )
				{
					currentTileType = TILE_TYPE_STONE;
				}
				else
				{
					currentTileType = TILE_TYPE_GRASS;
				}

				if( (indexX < m_safeZoneSize.x && indexY < m_safeZoneSize.y ) || ( indexX > m_mapDimensions.x - m_safeZoneSize.x - 1  && indexY < m_safeZoneSize.y ) )
				{
					currentTileType = TILE_TYPE_GRASS;
				}
				else if( indexY > m_mapDimensions.y - 7 && !isGoalSpawned )
				{
					if( g_RNG->GetRandomFloatZeroToOneInclusive() <= .1f)
					{
						currentTileType = TILE_TYPE_GOAL;
						isGoalSpawned = true;
					}
				}
			}

			IntVec2 newTileCoords( indexX, indexY );
			int tileIndex = ( m_mapDimensions.x * indexY ) + indexX; 
			Tile newTile( newTileCoords, tileIndex, currentTileType );

			m_tiles.push_back( newTile );
		}
	}
	if( !isGoalSpawned )
	{
		int topRowCenterTileIndex = m_mapDimensions.x * ( m_mapDimensions.y - 2 ) + ( m_mapDimensions.x / 2 );
		m_tiles[ topRowCenterTileIndex ].SetTileType( TILE_TYPE_GOAL );
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
	g_theRenderer->DrawVertextArray( m_mapTilesVerts );

	for( int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		m_players[ playerIndex ]->Render();
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
AABB2 Map::GetTileBounds( int tileIndex ) const
{
	AABB2 bounds;
	bounds.mins.x = static_cast<float>( m_tiles[ tileIndex ].GetTileCoords().x );
	bounds.mins.y = static_cast<float>( m_tiles[ tileIndex ].GetTileCoords().y );
	bounds.maxes = Vec2( bounds.mins.x + TILE_SIZE, bounds.mins.y + TILE_SIZE );
	return bounds;
}


//---------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( const IntVec2& tileCoords ) const
{
	return( tileCoords.x + (m_mapDimensions.x * tileCoords.y) );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex ) const
{
	return m_tiles[ tileIndex ].GetTileCoords();
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
	Rgba8 tileColor;
	if( tileToDraw.GetTileType() == TILE_TYPE_GRASS )
	{
		tileColor = RGBA8_DARK_GREEN;
	}
	else if( tileToDraw.GetTileType() == TILE_TYPE_STONE )
	{
		tileColor = RGBA8_DARK_GREY;
	}
	else if( tileToDraw.GetTileType() == TILE_TYPE_GOAL )
	{
		tileColor = RGBA8_MAGENTA;
	}

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->AppendVertsForAABB2D( tileVerts, GetTileBounds( tileToDraw.GetTileIndex() ), tileColor );
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
	for( int playerIndex = 0; playerIndex < m_players.size(); ++playerIndex )
	{
		HandlePlayerCollisions( m_players[playerIndex] );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandlePlayerCollisions( PlayerController* player )
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
	if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && m_tiles[ collidedTileIndex ].GetTileType() == TILE_TYPE_STONE )
	{
		Vec2 entityPosition = entity->GetPosition();
		float entityRadius = entity->GetPhysicsRadius();
		AABB2 tileBounds = GetTileBounds( collidedTileIndex );

		if( DoDiscAndAABB2Overlap( tileBounds, entityPosition, entityRadius ) )
		{
			Vec2 nearestPointOnTile = GetNearestPointOnAABB2D( entityPosition, tileBounds );
			Vec2 displacement = entityPosition - nearestPointOnTile;
			Vec2 entityCorrection = displacement.GetNormalized() * ( entityRadius - displacement.GetLength() );
			entity->SetPosition( entityPosition + entityCorrection );
		}
	}
	else if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && m_tiles[collidedTileIndex].GetTileType() == TILE_TYPE_GOAL )
	{
		entity->Die();
	}
}


//---------------------------------------------------------------------------------------------------------
PlayerController* Map::GetPlayer( int playerIndex ) const
{
	return m_players[ playerIndex ];
}


//---------------------------------------------------------------------------------------------------------
int Map::GetEntityCurrentTile( Entity* entity)
{
	IntVec2 entityTileCoords = GetTileCoordsForWorldPos( entity->GetPosition() );
	return GetTileIndexForTileCoords( entityTileCoords );
}