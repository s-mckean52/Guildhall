#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/RaycastResult.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerEntity.hpp"
#include "Game/Boulder.hpp"
#include "Game/Bullet.hpp"
#include "Game/NpcTank.hpp"
#include "Game/NpcTurret.hpp"
#include "Game/World.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/Explosion.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, World* theWorld, IntVec2 mapDimensions )
	: m_theGame( theGame )
	, m_theWorld( theWorld )
{
	m_safeZoneSize = IntVec2( 5, 5 );
	m_mapDimensions = mapDimensions;
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateDefaultTiles( TileType defaultType )
{
	m_tiles.clear();
	for( int tileIndex = 0; tileIndex < m_mapDimensions.x * m_mapDimensions.y; ++tileIndex )
	{
		int tilePositionY = tileIndex / m_mapDimensions.x;
		int tilePositionX = tileIndex % m_mapDimensions.x;

		m_tiles.push_back( Tile( IntVec2( tilePositionX, tilePositionY ), defaultType ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateBoundryTiles( TileType boundryType )
{
	//---------------------------------------------------------------------------------------------------------
	//   Create Border of boundryType Top and Bottom
	for( int mapXIndex = 0; mapXIndex < m_mapDimensions.x; ++mapXIndex )
	{
		int bottomTileAtXIndex	= GetTileIndexForTileCoords( IntVec2( mapXIndex, 0 ) );
		int topTileAtXIndex		= GetTileIndexForTileCoords( IntVec2( mapXIndex, m_mapDimensions.y - 1 ) );

		m_tiles[bottomTileAtXIndex].m_tileType = boundryType;
		m_tiles[topTileAtXIndex].m_tileType = boundryType;
	}

	//---------------------------------------------------------------------------------------------------------
	//   Create Border of boundryType Left and Right
	for( int mapYIndex = 0; mapYIndex < m_mapDimensions.y; ++mapYIndex )
	{
		int leftTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( 0, mapYIndex ) );
		int rightTileAtYIndex	= GetTileIndexForTileCoords( IntVec2( m_mapDimensions.x - 1, mapYIndex ) );

		m_tiles[leftTileAtYIndex].m_tileType = boundryType;
		m_tiles[rightTileAtYIndex].m_tileType = boundryType;
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnWormsOfTileType( int numWorms, TileType wormType, int wormLength )
{
	if( wormType == INVALID_TILE_TYPE ) return;

	for( int wormCount = 0; wormCount < numWorms; ++wormCount )
	{
		IntVec2 spawnTile;
		spawnTile.x = g_RNG->RollRandomIntInRange( 1, m_mapDimensions.x - 2 );
		spawnTile.y = g_RNG->RollRandomIntInRange( 1, m_mapDimensions.y - 2 );
		RunWorm( spawnTile, wormType, wormLength );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::RunWorm( const IntVec2& spawnTile, TileType wormTileType, int wormLength )
{
	IntVec2 currentTileCoords = spawnTile;
	for( int distanceTraveled = 0; distanceTraveled < wormLength; ++distanceTraveled )
	{
		int currentTileIndex = GetTileIndexForTileCoords( currentTileCoords );
		m_tiles[ currentTileIndex ].m_tileType = wormTileType;
		currentTileCoords = MoveInRandomCardinalDirection( currentTileCoords );
	}
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
	} 	while( !IsTileCoordWithinMapBounds( currentTileCoords ) );

	return currentTileCoords;
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsTileCoordWithinMapBounds( const IntVec2& tileCoords )
{
	bool isWithinBounds = true;
	if( tileCoords.x < 1 || tileCoords.x > m_mapDimensions.x - 2 )
	{
		isWithinBounds = false;
	}
	else if( tileCoords.y < 1 || tileCoords.y > m_mapDimensions.y - 2 )
	{
		isWithinBounds = false;
	}
	return isWithinBounds;
}


//---------------------------------------------------------------------------------------------------------
void Map::SetExitTile( TileType goalType )
{
	int exitTileIndex = (m_mapDimensions.x - 2) + (m_mapDimensions.x * (m_mapDimensions.y - 2));
	Tile& exitTile = m_tiles[ exitTileIndex ];
	exitTile.m_tileType = goalType;
	m_exitPosition = exitTile.GetBounds().GetCenter();
}


//---------------------------------------------------------------------------------------------------------
void Map::SetPlayerStartTile( TileType startType )
{
	m_startPosition = Vec2( 2.f, 2.f );
	int startTileIndex = GetTileIndexForWorldPos( m_startPosition );
	m_tiles[ startTileIndex ].m_tileType = startType;
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateSafeZones( TileType startZoneType, TileType exitZoneType )
{
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		IntVec2 tileCoords = GetTileCoordsForTileIndex( tileIndex );

		//Bottom Left
		if( (tileCoords.x >= 1 && tileCoords.x < m_safeZoneSize.x + 1) && (tileCoords.y >= 1 && tileCoords.y < m_safeZoneSize.y + 1) )
		{
			m_tiles[tileIndex].m_tileType = startZoneType;
		}

		//Top Right
		else if( (tileCoords.x < m_mapDimensions.x - 1 && tileCoords.x > m_mapDimensions.x - m_safeZoneSize.x - 1) &&
				 (tileCoords.y < m_mapDimensions.y - 1 && tileCoords.y > m_mapDimensions.y - m_safeZoneSize.y - 1) )
		{
			m_tiles[tileIndex].m_tileType = exitZoneType;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Map::HasPathToExit()
{
	m_tileData.clear();
	for( int tileIndex = 0; tileIndex < m_mapDimensions.x * m_mapDimensions.y; ++tileIndex )
	{
		m_tileData.push_back( MapTileData() );
	}

	int playerStartIndex = GetTileIndexForWorldPos( m_startPosition );
	int floodFillStartIndex = m_mapDimensions.x + 1;
	int floodFillStopIndex = (m_mapDimensions.x - 2) + ( m_mapDimensions.x * ( m_mapDimensions.y - 2 ) );
	int exitTileIndex = GetTileIndexForWorldPos( m_exitPosition );

	m_tileData[ playerStartIndex ].m_isAccessible = true;
	bool hasBeenUpdated = true;

	while( hasBeenUpdated )
	{
		hasBeenUpdated = false;
		for( int tileIndex = floodFillStartIndex; tileIndex < floodFillStopIndex + 1; ++tileIndex )
		{
			MapTileData& currentTileData = m_tileData[ tileIndex ];
			if( !currentTileData.m_hasBeenProcessed && currentTileData.m_isAccessible ) 
			{
				if( WasAdjacentTileSetAccessible( tileIndex ) )
				{
					hasBeenUpdated = true;
				}
				currentTileData.m_hasBeenProcessed = true;
			}
		}
	}
	if( m_tileData[ exitTileIndex ].m_isAccessible )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Map::WasAdjacentTileSetAccessible( int tileIndex )
{
	int northTileIndex = tileIndex + m_mapDimensions.x;
	int eastTileIndex = tileIndex + 1;
	int southTileIndex = tileIndex - m_mapDimensions.x;
	int westTileIndex = tileIndex - 1;

	bool northWasSet = WasTileSetAccessible( northTileIndex );
	bool eastWasSet = WasTileSetAccessible( eastTileIndex );
	bool southWasSet = WasTileSetAccessible( southTileIndex );
	bool westWasSet = WasTileSetAccessible( westTileIndex );

	if( northWasSet	||
		eastWasSet	||
		southWasSet	||
		westWasSet )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Map::WasTileSetAccessible( int tileIndex )
{
	if( !IsTileSolid( tileIndex ) )
	{
		m_tileData[ tileIndex ].m_isAccessible = true;
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
void Map::MakeInaccessibleTilesSolid( TileType tileType )
{
	for( int tileIndex = 0; tileIndex < m_tileData.size(); ++tileIndex )
	{
		if( !m_tileData[ tileIndex ].m_isAccessible )
		{
			m_tiles[ tileIndex ].m_tileType = tileType;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Render()
{
	DrawAllTiles();

	for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		RenderEntityType( static_cast<EntityType>( entityTypeIndex ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::RenderEntityType( EntityType type ) const
{
	if( type == ENTITY_TYPE_EXPLOSION )
	{
		g_theRenderer->SetBlendMode( BlendMode::ADDITIVE );
	}

	EntityList entityTypeList = m_entityLists[ type ];
	for( int entityIndex = 0; entityIndex < entityTypeList.size(); ++entityIndex )
	{
		Entity* currentEntity = entityTypeList[ entityIndex ];
		if( currentEntity )
		{
			currentEntity->Render();
		}
	}

	if( type == ENTITY_TYPE_EXPLOSION )
	{
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );

	CheckExitMap();
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
int Map::GetTileIndexForWorldPos( const Vec2& worldPos ) const
{
	return GetTileIndexForTileCoords( GetTileCoordsForWorldPos( worldPos ) );
}


//---------------------------------------------------------------------------------------------------------
void Map::DrawAllTiles()
{
	m_mapTilesVerts.clear();
	for( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		DrawTile( m_mapTilesVerts, m_tiles[tileIndex] );
	}
	g_theRenderer->BindTexture( TileDefinition::s_terrainTexture_8x8 );
	g_theRenderer->DrawVertexArray( m_mapTilesVerts );
}


//---------------------------------------------------------------------------------------------------------
void Map::DrawTile( std::vector<Vertex_PCU>& tileVerts, const Tile& tileToDraw )
{
	TileDefinition currentTileDef = TileDefinition::s_definitions[ tileToDraw.m_tileType ];
	AppendVertsForAABB2D( tileVerts, tileToDraw.GetBounds(), currentTileDef.GetTintColor(), currentTileDef.GetUVAtMins(), currentTileDef.GetUVAtMaxes() );
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{

	for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		UpdateEntityType( static_cast<EntityType>( entityTypeIndex ), deltaSeconds );
	}

	HandleCollisions();

	DeleteGarbageEntities();
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntityType( EntityType type, float deltaSeconds )
{
	EntityList entityTypeList = m_entityLists[ type ];
	for( int entityIndex = 0; entityIndex < entityTypeList.size(); ++entityIndex )
	{
		Entity* currentEntity = entityTypeList[entityIndex];
		if( currentEntity )
		{
			currentEntity->Update( deltaSeconds );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleCollisions()
{
	ResolveBulletCollisions( ENTITY_TYPE_GOOD_BULLET );
	ResolveBulletCollisions( ENTITY_TYPE_BAD_BULLET );

	for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		EntityList& entityTypeList = m_entityLists[ entityTypeIndex ];
		for( int entityIndex = 0; entityIndex < entityTypeList.size(); ++entityIndex )
		{
			Entity* currentEntity = entityTypeList[ entityIndex ];
			if( currentEntity != nullptr )
			{
				HandleEntityCollisions( currentEntity );
			}
		}
	}

}


//---------------------------------------------------------------------------------------------------------
void Map::HandleEntityCollisions( Entity* entity )
{
	int entityCurrentTileIndex = GetEntityCurrentTile( entity );

	if( entity->IsPushedByEntities() )//TODO: clean this up
	{
		for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
		{
			EntityList& entityTypeList = m_entityLists[ entityTypeIndex ];
			for( int entityIndex = 0; entityIndex < entityTypeList.size(); ++entityIndex )
			{
				Entity *collidedEntity = entityTypeList[entityIndex];
				if( collidedEntity && collidedEntity != entity && DoEntitiesOverlap( entity, collidedEntity ) )
				{
					float entityPhysicsRadius = entity->GetPhysicsRadius();
					Vec2 entityPosition = entity->GetPosition();
					float collidedEntityPhysicsRadius = collidedEntity->GetPhysicsRadius();
					Vec2 collidedEntityPosition = collidedEntity->GetPosition();

					if( collidedEntity->DoesPushEntities() && collidedEntity->IsPushedByEntities() )
					{
						PushDiscsOutOfEachOther2D( entityPosition, entityPhysicsRadius, collidedEntityPosition, collidedEntityPhysicsRadius );
						entity->SetPosition( entityPosition );
						collidedEntity->SetPosition( collidedEntityPosition );
					}

					else if( collidedEntity->DoesPushEntities() )
					{
						PushDiscOutOfDisc2D( entityPosition, entityPhysicsRadius, collidedEntityPosition, collidedEntityPhysicsRadius );
						entity->SetPosition( entityPosition );
					}
				}
			}
		}
	}

	if( entity->IsPushedByWalls() )
	{
		HandleTileCollision( entity, entityCurrentTileIndex - m_mapDimensions.x ); //south
		HandleTileCollision( entity, entityCurrentTileIndex + 1 ); //east
		HandleTileCollision( entity, entityCurrentTileIndex + m_mapDimensions.x ); //north
		HandleTileCollision( entity, entityCurrentTileIndex - 1 ); //west

		HandleTileCollision( entity, entityCurrentTileIndex + 1 - m_mapDimensions.x ); //south-east
		HandleTileCollision( entity, entityCurrentTileIndex + 1 + m_mapDimensions.x ); //north-east
		HandleTileCollision( entity, entityCurrentTileIndex - 1 + m_mapDimensions.x ); //north-west
		HandleTileCollision( entity, entityCurrentTileIndex - 1 - m_mapDimensions.x ); //south-west
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::ResolveBulletCollisions( EntityType type )
{
	EntityList bulletList = m_entityLists[ type ];

	for( int bulletIndex = 0; bulletIndex < bulletList.size(); ++bulletIndex )
	{
		Bullet* currentBullet = static_cast<Bullet*>( bulletList[ bulletIndex ] );

		if( currentBullet != nullptr && !DidBulletCollideWithSolidTile( currentBullet ) )
		{
			ResolveBulletOverlapsEntity( currentBullet );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Map::DidBulletCollideWithSolidTile( Bullet* bullet )
{
	Vec2 collisionPoint = bullet->GetPosition() + ( bullet->GetForwardVector() * bullet->GetPhysicsRadius() );
	int collisionPointTileIndex = GetTileIndexForWorldPos( collisionPoint );
	TileDefinition collisionPointTileDef = TileDefinition::s_definitions[ m_tiles[ collisionPointTileIndex ].m_tileType ];

	if( collisionPointTileDef.IsSolid() )
	{
		bullet->Die();
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
void Map::ResolveBulletOverlapsEntity( Bullet* bullet )
{
	for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		EntityList& entityTypeList = m_entityLists[ entityTypeIndex ];
		for( int entityIndex = 0; entityIndex < entityTypeList.size(); ++entityIndex )
		{
			Entity* entityToCheckCollision = entityTypeList[ entityIndex ];
			if( entityToCheckCollision && entityToCheckCollision->IsHitByBullet() && AreOpposingFactions( bullet, entityToCheckCollision ) )
			{
				if( DoEntitiesOverlap( bullet, entityToCheckCollision ) )
				{
					bullet->Die();
					entityToCheckCollision->TakeDamage( bullet->GetDamageToDeal() );
				}
			}
			else if( entityToCheckCollision && entityToCheckCollision->GetType() == ENTITY_TYPE_BOULDER )
			{
				if( DoEntitiesOverlap( bullet, entityToCheckCollision ) )
				{
					DeflectBulletOffEntity( bullet, entityToCheckCollision );
				}
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::DeflectBulletOffEntity( Bullet* bulletToDeflect, Entity* entityToDeflectOffOf )
{
	Vec2 bulletPosition = bulletToDeflect->GetPosition();
	Vec2 entityToDeflectOffPosition = entityToDeflectOffOf->GetPosition();
	float entityToDeflectOffPhysicsRadius = entityToDeflectOffOf->GetPhysicsRadius();

	PushDiscOutOfDisc2D( bulletPosition, bulletToDeflect->GetPhysicsRadius(), entityToDeflectOffPosition, entityToDeflectOffPhysicsRadius );
	bulletToDeflect->SetPosition( bulletPosition );

	Vec2 bulletVelocity = bulletToDeflect->GetForwardVector() * bulletToDeflect->GetSpeed();
	Vec2 contactPoint = GetNearestPointOnDisc2D( bulletPosition, entityToDeflectOffPosition, entityToDeflectOffPhysicsRadius );

	Vec2 entityBulletDispalcement = contactPoint - entityToDeflectOffPosition;

	bulletVelocity.Reflect( entityBulletDispalcement );
	bulletToDeflect->SetOrientationDegrees( bulletVelocity.GetAngleDegrees() );	
}


//---------------------------------------------------------------------------------------------------------
bool Map::AreOpposingFactions( Entity* A, Entity* B )
{
	if( A->GetFaction() == B->GetFaction() )
	{
		return false;
	}
	return true;
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
	for( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		DeleteGarbageEntitiesOfType( static_cast<EntityType>( entityTypeIndex ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::DeleteGarbageEntitiesOfType( EntityType type )
{
	EntityList& entityList = m_entityLists[ type ];
	for( int entityIndex = 0; entityIndex < entityList.size(); ++entityIndex )
	{
		Entity* currentEntity = entityList[ entityIndex ];
		if( currentEntity && currentEntity->IsGarbage() )
		{
			delete currentEntity;
			entityList[entityIndex] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleTileCollision( Entity* entity, int collidedTileIndex )
{
	Tile collidedTile = m_tiles[ collidedTileIndex ];
	TileDefinition collidedTileDefinition = TileDefinition::s_definitions[ collidedTile.m_tileType ];
	if( collidedTileIndex >= 0 && collidedTileIndex < m_tiles.size() && collidedTileDefinition.IsSolid() )
	{
		Vec2 entityPosition = entity->GetPosition();
		float entityRadius = entity->GetPhysicsRadius();
		AABB2 tileBounds = collidedTile.GetBounds();

		PushDiscOutOfAABB2( entityPosition, entityRadius, tileBounds );
 		entity->SetPosition( entityPosition );
	}
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsPointInSolid( Vec2 pointToCheck )
{
	int tileIndexForPoint = GetTileIndexForWorldPos( pointToCheck );
	
	return IsTileSolid( tileIndexForPoint );
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid( int tileIndex )
{
	TileDefinition tileDefinitionAtTile = TileDefinition::s_definitions[ m_tiles[ tileIndex ].m_tileType ];

	return tileDefinitionAtTile.IsSolid();
}


//---------------------------------------------------------------------------------------------------------
bool Map::HasLineOfSight( const Vec2& startPos, const Vec2& endPos )
{
	Vec2 displacement = endPos - startPos;

	RaycastResult raycastResult = Raycast( startPos, displacement.GetNormalized(), displacement.GetLength() );

	return !raycastResult.m_didImpact;
}


//---------------------------------------------------------------------------------------------------------
RaycastResult Map::Raycast( Vec2 startPoint, Vec2 fwdDir, float maxDistance )
{
	bool didImpact = false;
	TileType collidedTileType = INVALID_TILE_TYPE;
	Vec2 currentRaycastPoint = startPoint + ( fwdDir * RAYCAST_STEP_DISTANCE );
	Vec2 raycastCurrentDisplacement = currentRaycastPoint - startPoint;

	while( raycastCurrentDisplacement.GetLength() < maxDistance && !didImpact )
	{
		if( IsPointInSolid( currentRaycastPoint ) )
		{
			didImpact = true;
			collidedTileType = m_tiles[ GetTileIndexForWorldPos( currentRaycastPoint) ].m_tileType;
		}
		else
		{
			currentRaycastPoint += fwdDir * RAYCAST_STEP_DISTANCE;
			raycastCurrentDisplacement = currentRaycastPoint - startPoint;
		}
	}

	float fractionTraveledOfMaxDistance = raycastCurrentDisplacement.GetLength() / maxDistance;

	return RaycastResult( currentRaycastPoint, didImpact, raycastCurrentDisplacement.GetLength(), fractionTraveledOfMaxDistance, collidedTileType );
}


//---------------------------------------------------------------------------------------------------------
void Map::CheckExitMap()
{
	Entity* player = GetEntity( ENTITY_TYPE_PLAYER, 0 );
	if( DoDiscsOverlap( player->GetPosition(), player->GetPhysicsRadius(), m_exitPosition, 0.f ) )
	{
		m_theWorld->MoveToNextMap();
	}
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnEntity( EntityType entityType, const Vec2& spawnPosition )
{
	Entity* spawnedEntity = nullptr;
	switch( entityType )
	{
	case ENTITY_TYPE_PLAYER:
		spawnedEntity = new PlayerEntity( m_theGame, this, spawnPosition, 0 );
		break;
	case ENTITY_TYPE_GOOD_BULLET:
		spawnedEntity = new Bullet( this, spawnPosition, FACTION_GOOD );
		break;
	case ENTITY_TYPE_BAD_BULLET:
		spawnedEntity = new Bullet( this, spawnPosition, FACTION_EVIL );
		break;
	case ENTITY_TYPE_BOULDER:
		spawnedEntity = new Boulder( this, spawnPosition, FACTION_NEUTRAL );
		break;
	case ENTITY_TYPE_NPC_TURRET:
		spawnedEntity = new NpcTurret( this, spawnPosition, FACTION_EVIL );
		break;
	case ENTITY_TYPE_NPC_TANK:
		spawnedEntity = new NpcTank( this, spawnPosition, FACTION_EVIL );
		break;

	default:
		ERROR_AND_DIE( Stringf( "Unknown entity type #%i", entityType ) );
		break;
	}

	AddEntityToMap( spawnedEntity );
	return spawnedEntity;
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnExplosion( const Vec2& position, float radius, float duration )
{
	Entity* newExplosion = new Explosion( this, position, duration, radius );
	AddEntityToMap( newExplosion );
}


//---------------------------------------------------------------------------------------------------------
void Map::AddEntityToMap( Entity* entityToAdd )
{
	EntityList* entityTypeList = &m_entityLists[ entityToAdd->GetType() ];
	AddEntityToList( entityToAdd, entityTypeList );
}


//---------------------------------------------------------------------------------------------------------
void Map::AddEntityToList( Entity* entity, EntityList* entityList )
{
	entityList->push_back( entity );
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::GetEntity( EntityType type,  int index ) const
{
	return m_entityLists[ type ][ index ];
}


//---------------------------------------------------------------------------------------------------------
int Map::GetEntityCurrentTile( Entity* entity) const
{
	IntVec2 entityTileCoords = GetTileCoordsForWorldPos( entity->GetPosition() );
	return GetTileIndexForTileCoords( entityTileCoords );
}


//---------------------------------------------------------------------------------------------------------
TileType Map::GetEntityCurrentTileType( Entity* entity ) const
{
	int currentTileIndex = GetEntityCurrentTile( entity );
	return m_tiles[ currentTileIndex ].m_tileType;
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnEntityAtRandomPosition( const EntityType& entityType )
{
	IntVec2 randomSpawnTile;
	int randomSpawnTileIndex = 0;

	while( IsTileSolid( randomSpawnTileIndex ) )
	{
		randomSpawnTile.x = g_RNG->RollRandomIntInRange( 1, m_mapDimensions.x - 2 );
		randomSpawnTile.y = g_RNG->RollRandomIntInRange( 1, m_mapDimensions.y - 2 );
		randomSpawnTileIndex = GetTileIndexForTileCoords( randomSpawnTile );
	}

	Vec2 spawnPosition = GetWorldPosForTileCoords( randomSpawnTile );

	//Center Entity on Tile
	spawnPosition.x += 0.5f;
	spawnPosition.y += 0.5f;


	SpawnEntity( entityType, spawnPosition );
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnNumberOfEntites( int numEntitiesToSpawn )
{
	for( int newEntityNum = 0; newEntityNum < numEntitiesToSpawn; ++newEntityNum )
	{
		SpawnRandomEntity();
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnRandomEntity()
{
	bool entitySpawned = false;
	while( !entitySpawned )
	{
		EntityType randomEntityType = static_cast<EntityType>( g_RNG->RollRandomIntInRange( 0, NUM_ENTITY_TYPES ) );
		if( randomEntityType == ENTITY_TYPE_NPC_TANK || randomEntityType == ENTITY_TYPE_NPC_TURRET || randomEntityType == ENTITY_TYPE_BOULDER )
		{
			SpawnEntityAtRandomPosition( randomEntityType );
			entitySpawned = true;
		}
	}
}