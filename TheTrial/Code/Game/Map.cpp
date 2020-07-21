#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Projectile.hpp"
#include "Game/Enemy.hpp"
#include "Game/Cursor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/RaycastResult.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
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
void Map::CleanUpEntities()
{
	for( int projectile = 0; projectile < m_projectiles.size(); ++projectile )
	{
		if( m_projectiles[projectile]->IsDead() )
		{
			m_projectiles.erase( m_projectiles.begin() + projectile );
		}
		else
		{
			static_cast<Projectile*>( m_projectiles[projectile] )->ValidateTarget();
		}
	}
	for( int i = 0; i < m_entities.size(); ++i )
	{
		if( m_entities[i]->IsDead() && m_entities[i] != m_player )
		{
			Enemy* enemyEntity = dynamic_cast<Enemy*>( m_entities[i] );
			if( enemyEntity != nullptr )
			{
				--m_enemyCount;
			}
			delete m_entities[i];
			m_entities.erase( m_entities.begin() + i );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::ClearEntities()
{
// 	for( int i = 0; i < m_entities.size(); ++i )
// 	{
// 		if( m_entities[i] != nullptr && m_entities[i] != m_player )
// 		{
// 			delete m_entities[i];
// 			m_entities[i] = nullptr;
// 		}
// 	}
	m_entities.clear();
	m_enemyCount = 0;
}


//---------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateMapVerts( deltaSeconds );
	UpdateEntites( deltaSeconds );
	ResolveEntityOverlaps();
	PushActorsOutOfWalls();
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntites( float deltaSeconds )
{
	//m_player->Update( deltaSeconds );
	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entities[ entityIndex ];
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
void Map::ResolveEntityOverlaps()
{
	Enemy* enemyOverlapingPlayer = GetDiscOverlapEnemy( m_player->GetCurrentPosition(), m_player->GetPhysicsRadius() );
	if( enemyOverlapingPlayer != nullptr )
	{
		Vec2 playerPosition = m_player->GetCurrentPosition();
		Vec2 enemyPosition = enemyOverlapingPlayer->GetCurrentPosition();
		PushDiscsOutOfEachOther2D( playerPosition, m_player->GetPhysicsRadius(), enemyPosition, enemyOverlapingPlayer->GetPhysicsRadius() );
		m_player->SetCurrentPosition( playerPosition );
		enemyOverlapingPlayer->SetCurrentPosition( enemyPosition );
		enemyOverlapingPlayer->AttackActor( m_player );
	}

	for( int i = 0; i < m_entities.size(); ++i )
	{
		Actor* currentActor = dynamic_cast<Actor*>( m_entities[i] );
		if( currentActor == nullptr || currentActor->IsDead() )
			continue;

		Vec2 currentActorPosition = currentActor->GetCurrentPosition();
		float currentActorRadius = currentActor->GetPhysicsRadius();
		for( int j = 0; j < m_entities.size(); ++j )
		{
			Actor* actorToCheck = dynamic_cast<Actor*>( m_entities[j] );
			if( actorToCheck == nullptr || currentActor->IsDead() )
				continue;

			Vec2 actorToCheckPosition = actorToCheck->GetCurrentPosition();
			float actorToCheckRadius = actorToCheck->GetPhysicsRadius();
			if( actorToCheck != currentActor && DoDiscsOverlap( currentActorPosition, currentActorRadius, actorToCheckPosition, actorToCheckRadius ) )
			{
				PushDiscsOutOfEachOther2D( currentActorPosition, currentActorRadius, actorToCheckPosition, actorToCheckRadius );
				currentActor->SetCurrentPosition( currentActorPosition );
				actorToCheck->SetCurrentPosition( actorToCheckPosition );
			}
		}
	}

	for( int projectileIndex = 0; projectileIndex < m_projectiles.size(); ++projectileIndex )
	{
		Projectile* projectile = static_cast<Projectile*>( m_projectiles[ projectileIndex ] );
		Enemy* hitEnemy = GetDiscOverlapEnemy( projectile->GetCurrentPosition(), projectile->GetPhysicsRadius() );
		if( hitEnemy != nullptr && !projectile->HasTarget() )
		{
			projectile->DealDamageToActor( hitEnemy );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::PushActorsOutOfWalls()
{
	for( int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Actor* actorToPush = dynamic_cast<Actor*>( m_entities[entityIndex] );
		if( actorToPush == nullptr )
			continue;

		IntVec2 actorCurrentTile;
		actorCurrentTile.x = RoundDownToInt( actorToPush->GetCurrentPosition().x );
		actorCurrentTile.y = RoundDownToInt( actorToPush->GetCurrentPosition().y );

		PushActorOutOfTile( actorToPush, actorCurrentTile, 0, 0 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, 1, 0 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, 1, 1 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, 0, 1 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, -1, 1 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, -1, 0 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, -1, -1 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, 0, -1 );
		PushActorOutOfTile( actorToPush, actorCurrentTile, 1, -1 );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::PushActorOutOfTile( Actor* actorToPush, IntVec2 currentTileCoord, int xDir, int yDir )
{
	IntVec2 tileCoordsToCheck = currentTileCoord + IntVec2( xDir, yDir );
	Tile* tile = GetTileByCoords( tileCoordsToCheck );

	if( IsTileSolid( tile ) )
	{
		Vec2 actorPosition = actorToPush->GetCurrentPosition();
		PushDiscOutOfAABB2( actorPosition, actorToPush->GetPhysicsRadius(), tile->GetWorldBounds() );
		actorToPush->SetCurrentPosition( actorPosition );
	}
}

//---------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	if( m_verts.size() == 0 )
		return;

	//Draw Map Tiles
	SpriteSheet* spriteSheetToUse = TileDefinition::s_spriteSheets["RPGTerrain"];
	Texture const& textureToUse = spriteSheetToUse->GetTexture();
	g_theRenderer->BindTexture( &textureToUse );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( m_verts );

	if( g_isDebugDraw )
	{
		DebugRender();
	}

	if( m_isExitEnabled )
	{
		std::vector<Vertex_PCU> verts;
		AppendVertsForFilledCircle( verts, 0.5f, Rgba8::BLUE );
		TransformVertexArray( verts, 1.f, 0.f, m_exitSpawnPosition );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( verts );
		//DrawCircleAtPoint( m_exitSpawnPosition, 0.5f, Rgba8::BLUE, DEBUG_THICKNESS );
	}

	//Draw Entities
	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entities[ entityIndex ];
		if( currentEntity != nullptr )
		{
			currentEntity->Render();
		}
	}
	//Draw Player
	//m_player->Render();
}


//---------------------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
}


//---------------------------------------------------------------------------------------------------------
void Map::SetExitIsEnabled( bool isEnabled )
{
	if( !m_isExitEnabled && isEnabled )
	{
		SoundID teleporterSound = g_theAudio->CreateOrGetSound( "Data/Audio/portalOpen.wav" );
		g_theAudio->PlaySound( teleporterSound, false, 0.75f * m_theGame->GetSFXVolume() );
	}
	m_isExitEnabled = isEnabled;
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsPlayerInExit()
{
	if( m_isExitEnabled && DoDiscsOverlap( m_player->GetCurrentPosition(), m_player->GetPhysicsRadius(), m_exitSpawnPosition, 0.5f ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
Tile* Map::GetTileByCoords( IntVec2 const& tileCoords )
{
	int tileIndex = ( tileCoords.y * m_dimensions.x ) + tileCoords.x;
	return m_tiles[ tileIndex ];
}


//---------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid( Tile* tile )
{
	return tile->GetTileDefinition()->IsSolid();
}

//---------------------------------------------------------------------------------------------------------
void Map::SpawnEnemy( int maxNumEnemies )
{
	if( m_enemyCount >= maxNumEnemies || m_isExitEnabled )
		return;

	int enemySpawnPositionIndex = g_RNG->RollRandomIntInRange( 0, static_cast<int>( m_enemySpawnPositions.size() ) - 1 );
	Vec2 spawnPosition = m_enemySpawnPositions[ enemySpawnPositionIndex ];

	if( GetDiscOverlapEnemy( spawnPosition, 0.5f ) == nullptr )
	{
		Enemy* newEnemy = new Enemy( m_theGame, spawnPosition );
		newEnemy->m_theWorld = m_theWorld;
		m_entities.push_back( newEnemy );
		++m_enemyCount;
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::CreateTilesFromImage( char const* filepath )
{
	const unsigned char enemySpawnAlpha = static_cast<unsigned char>( 255.f * 0.2f );
	const unsigned char playerSpawnAlpha = static_cast<unsigned char>( 255.f * 0.75f );
	const unsigned char exitSpawnAlpha = static_cast<unsigned char>( 255.f * 0.5f ) + 1;

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

		unsigned char texelAlpha = texelColor.a;
		if( texelAlpha == playerSpawnAlpha )
		{
			m_playerSpawnPosition = m_tiles[i]->GetCenterPosition();
		}
		else if( texelAlpha == exitSpawnAlpha )
		{
			m_exitSpawnPosition = m_tiles[i]->GetCenterPosition();
		}
		else if( texelAlpha == enemySpawnAlpha )
		{
			m_enemySpawnPositions.push_back( m_tiles[i]->GetCenterPosition() );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::AddPlayerToMap( Player* player )
{
	m_player = player;
	AddEntityToList( player );
	player->SetCurrentMap( this );
	player->SetCurrentPosition( m_playerSpawnPosition );
	player->SetIsMoving( false );
}


//---------------------------------------------------------------------------------------------------------
void Map::AddEntityToList( Entity* entity )
{
	m_entities.push_back( entity );
	if( dynamic_cast<Projectile*>( entity ) != nullptr )
	{
		m_projectiles.push_back( entity );
	}
}


//---------------------------------------------------------------------------------------------------------
Enemy* Map::GetDiscOverlapEnemy( Vec2 const& discCenterPosition, float discRadius )
{
	for( int i = 0; i < m_entities.size(); ++i )
	{
		Enemy* enemy = dynamic_cast<Enemy*>( m_entities[i] );
		if( enemy != nullptr && !enemy->IsDead() && DoDiscsOverlap( discCenterPosition, discRadius, enemy->GetCurrentPosition(), enemy->GetPhysicsRadius() ) )
		{
			return enemy;
		}
	}
	return nullptr;
}


//---------------------------------------------------------------------------------------------------------
RaycastResult Map::Raycast( Vec2 const& startPosition, Vec2 const& direction, float maxDistance, Actor* ignoredActor )
{
	std::vector<RaycastResult> results;
	results.push_back( RaycastAgainstTiles( startPosition, direction, maxDistance ) );
	results.push_back( RaycastAgainstActors( startPosition, direction, maxDistance, ignoredActor ) );
	return GetBestRaycast( results );
}


//---------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastAgainstTiles( Vec2 const& startPosition, Vec2 const& direction, float maxDistance )
{
	IntVec2 currentTileCoords;
	currentTileCoords.x = RoundDownToInt( startPosition.x );
	currentTileCoords.y = RoundDownToInt( startPosition.y );

	Tile* tile = GetTileByCoords( currentTileCoords );
	if( IsTileSolid( tile ) )
	{
		return RaycastResult( startPosition, direction, maxDistance, startPosition, true, 0.f, -direction );
	}

	Vec2 rayDisplacement = direction * maxDistance;
	int xStepSign = static_cast<int>( Signf( rayDisplacement.x ) );
	int yStepSign = static_cast<int>( Signf( rayDisplacement.y ) );

	float xStep = 1.f / abs( rayDisplacement.x );
	float yStep = 1.f / abs( rayDisplacement.y );
	
	int offsetToLeadingEdgeX = ( xStepSign + 1 ) / 2;
	int offsetToLeadingEdgeY = ( yStepSign + 1 ) / 2;
	
	float firstIntersectionX = static_cast<float>( currentTileCoords.x + offsetToLeadingEdgeX );
	float firstIntersectionY = static_cast<float>( currentTileCoords.y + offsetToLeadingEdgeY );
	
	float nextCrossingX = abs( firstIntersectionX - startPosition.x ) * xStep;
	float nextCrossingY = abs( firstIntersectionY - startPosition.y ) * yStep;

	for( ;; )
	{
		if( nextCrossingX <= nextCrossingY )
		{
			if( nextCrossingX > 1.f )
				return RaycastResult( startPosition, direction, maxDistance, startPosition + rayDisplacement, false, maxDistance );

			currentTileCoords.x += xStepSign;
			tile = GetTileByCoords( currentTileCoords );
			if( IsTileSolid( tile ) )
			{
				Vec2 impactPosition = startPosition + ( direction * ( maxDistance * nextCrossingX ) );
				return RaycastResult( startPosition, direction, maxDistance, impactPosition, true, maxDistance * nextCrossingX, Vec2( (float)(-xStepSign), 0.f ) );
			}
			nextCrossingX += xStep;
		}
		else if( nextCrossingY < nextCrossingX )
		{
			if( nextCrossingY > 1.f )
				return RaycastResult( startPosition, direction, maxDistance, startPosition + rayDisplacement, false, maxDistance );

			currentTileCoords.y += yStepSign;
			tile = GetTileByCoords( currentTileCoords );
			if( IsTileSolid( tile ) )
			{
				Vec2 impactPosition = startPosition + ( direction * ( maxDistance * nextCrossingY ) );
				return RaycastResult( startPosition, direction, maxDistance, impactPosition, true, maxDistance * nextCrossingY, Vec2( 0.f, (float)(-yStepSign) ) );
			}
			nextCrossingY += yStep;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
RaycastResult Map::RaycastAgainstActors( Vec2 const& startPosition, Vec2 const& direction, float maxDistance, Actor* ignoredActor )
{
	Entity* impactedEntity = nullptr;
	Vec2 closestOverallImpact;
	float closestOverallDistance = 0.f;

	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		std::vector<Vec2> potentialHitPoints;
		//xy overlap check
		Actor* currentEntity = dynamic_cast<Actor*>( m_entities[ entityIndex ] );
		if( currentEntity == nullptr || currentEntity == ignoredActor )
			continue;

		Vec2 entityPosition = currentEntity->GetCurrentPosition();
		float entityRadius = currentEntity->GetPhysicsRadius();
		
		if( !DoDiscsOverlap( startPosition, maxDistance, entityPosition, entityRadius ) )
			continue;

		Vec2 fwdDirIBasis = direction.GetNormalized();
		Vec2 fwdDirJBasis = fwdDirIBasis.GetRotated90Degrees();

		Vec2 displacementToEntity = entityPosition - startPosition;

		float projectedIDistance = DotProduct2D( displacementToEntity, fwdDirIBasis );
		float distanceToRay = -DotProduct2D( displacementToEntity, fwdDirJBasis );

		float entityRadiusSquared = entityRadius * entityRadius;
		float distanceToRaySquared = distanceToRay * distanceToRay;
		float underRadicalValue = entityRadiusSquared - distanceToRaySquared;
		float plusMinusDistance = sqrtf( underRadicalValue );
		if( underRadicalValue > 0.f )
		{
			float distance1 = projectedIDistance - plusMinusDistance;
			//float distance2 = projectedIDistance + plusMinusDistance;
			potentialHitPoints.push_back( startPosition + ( direction * distance1 ) );
		}
		else if( underRadicalValue == 0.f )
		{
			float distance = projectedIDistance;
			potentialHitPoints.push_back( startPosition + ( direction * distance ) );
		}
		else
		{
			continue;
		}

		if( potentialHitPoints.size() > 0 )
		{
			Vec2 closestImpact = potentialHitPoints[0];
			Vec2 displacmentToImpact = closestImpact - startPosition;
			float signedDistanceToImpact = DotProduct2D( displacmentToImpact, direction );
			if( impactedEntity == nullptr || ( signedDistanceToImpact < closestOverallDistance && signedDistanceToImpact > 0 ) )
			{
				impactedEntity = currentEntity;
				closestOverallImpact = closestImpact;
				closestOverallDistance = signedDistanceToImpact;
			}
		}
	}
	
	if( impactedEntity != nullptr )
	{
		Vec2 impactNormal = impactedEntity->GetCurrentPosition() - closestOverallImpact;
		impactNormal.Normalize();
		return RaycastResult( startPosition, direction, maxDistance, closestOverallImpact, true, closestOverallDistance, impactNormal, impactedEntity );
	}
	return RaycastResult( startPosition, direction, maxDistance, startPosition + ( direction * maxDistance ), false, maxDistance );
}


//---------------------------------------------------------------------------------------------------------
RaycastResult Map::GetBestRaycast( std::vector<RaycastResult> results )
{
	RaycastResult bestRaycast;
	float bestRaycastResultImpactDistance = (float)0xffffffff;
	for( uint resultsIndex = 0; resultsIndex < results.size(); ++resultsIndex )
	{
		RaycastResult result = results[ resultsIndex ];
		if( result.impactDistance < bestRaycastResultImpactDistance )
		{
			bestRaycastResultImpactDistance = result.impactDistance;
			bestRaycast = result;
		}
	}
	return bestRaycast;
}
