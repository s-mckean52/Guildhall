#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/Actor.hpp"
#include "Game/Entity.hpp"
#include "Game/Projectile.hpp"
#include "Game/Portal.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, World* theWorld, std::string const& name )
{
	m_game = theGame;
	m_world = theWorld;
	m_name = name;
}


//---------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_mapMesh;
	m_mapMesh = nullptr;

	//DeleteEntites();
}


//---------------------------------------------------------------------------------------------------------
MapData Map::GetMapData()
{
	MapData mapData;
	mapData.m_numEntities = m_entities.size();
	GetEntityDataFromArray( mapData.m_entities, m_entities );
	return mapData;
}


//---------------------------------------------------------------------------------------------------------
SpawnData Map::GetEntitySpawnData()
{
	SpawnData spawnData;
	for( int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		EntitySpawnData& entitySpawnData = spawnData.m_entitiesToSpawn[entityIndex];
		entitySpawnData.m_isUsed = true;
		entitySpawnData.m_data = m_entities[entityIndex]->GetEntityData();

		std::string entityName = m_entities[entityIndex]->GetEntityName();
		memcpy( &entitySpawnData.m_entityDefName[0], &entityName[0], entityName.size() );
	}
	return spawnData;
}


//---------------------------------------------------------------------------------------------------------
void Map::GetEntityDataFromArray( EntityData entityData[], std::vector<Entity*> const& entities )
{
	for( int entityIndex = 0; entityIndex < entities.size(); ++entityIndex )
	{
		entityData[entityIndex] = entities[entityIndex]->GetEntityData();
	}
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnEntityFromSpawnData( EntitySpawnData const& entitySpawnData )
{
	EntityDef* entityDef = EntityDef::GetEntityDefByName( entitySpawnData.m_entityDefName );
	
	EntityType entityType = entityDef->GetEntityType();
	Entity* spawnedEntity = nullptr;
	switch( entityType )
	{
	case ENTITY_TYPE_ENTITY:		spawnedEntity = new Entity(		m_game, m_world, this, *entityDef ); break;
	case ENTITY_TYPE_ACTOR:			spawnedEntity = new Actor(		m_game, m_world, this, *entityDef ); break;
	case ENTITY_TYPE_PROJECTILE:	spawnedEntity = new Projectile(	m_game, m_world, this, *entityDef ); break;
	case ENTITY_TYPE_PORTAL:		spawnedEntity = new Portal(		m_game, m_world, this, *entityDef ); break;
	default:
		ERROR_AND_DIE( "Tried to spawn an unsupported entity on map" )
		break;
	}

	AddEntityToMap( spawnedEntity );
	spawnedEntity->SetValuesFromEntityData( entitySpawnData.m_data );
	return spawnedEntity;
}


//---------------------------------------------------------------------------------------------------------
void Map::UpdateEntitiesFromMapData( MapData const& mapData )
{
	for( int entityIndex = 0; entityIndex < mapData.m_numEntities; ++ entityIndex )
	{
		m_entities[entityIndex]->SetValuesFromEntityData( mapData.m_entities[entityIndex] );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnPlayer( Camera* playerCamera )
{
	Entity** entityForPossession = g_theGame->GetPossessedEntityPointer();
	g_theGame->SetPossessedEntity( entityForPossession, m_playerStartEntity );
	g_theGame->PlaySpawnSound();

	if( m_playerStartEntity == nullptr || *g_theGame->GetPossessedEntityPointer() == nullptr )
	{
		playerCamera->SetPosition( Vec3( m_playerStartPositionXY, PLAYER_HEIGHT ) );
		playerCamera->SetYawDegrees( m_playerStartYawDegrees );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnNewEntityOnMap( XmlElement const& element )
{
	std::string entityTypeAsString = element.Name();
	EntityType entityType = EntityDef::GetEntityTypeFromString( entityTypeAsString );

	std::string entityDefName = ParseXmlAttribute( element, "name", "MISSING" );
	if( entityDefName == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse \"name\" of entity at line %i", element.GetLineNum() );
		return;
	}

	EntityDef* entityDefToSpawn = EntityDef::GetEntityDefByName( entityDefName );
	if( entityDefToSpawn == nullptr )
	{
		g_theConsole->ErrorString( "Entity Def for \"%s\" at line %i does not exist", entityDefName.c_str(), element.GetLineNum() );
		return;
	}
	else if( entityType != entityDefToSpawn->GetEntityType() )
	{
		g_theConsole->ErrorString( "Entity Def \"%s\" at line %i is \"%s\" not \"%s\"", entityDefName.c_str(), element.GetLineNum(), entityDefToSpawn->GetEntityTypeAsString().c_str(), entityTypeAsString.c_str() );
		return;
	}

	SpawnNewEntityOfType( *entityDefToSpawn, element );
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( std::string const& entityDefName, XmlElement const& element )
{
	EntityDef* entityDef = EntityDef::GetEntityDefByName( entityDefName );
	if( entityDef == nullptr )
	{
		return nullptr;
	}
	return SpawnNewEntityOfType( *entityDef, element );
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( EntityDef const& entityDef, XmlElement const& element )
{
	EntityType entityType = entityDef.GetEntityType();
	Entity* spawnedEntity = nullptr;
	switch( entityType )
	{
	case ENTITY_TYPE_ENTITY:		spawnedEntity = new Entity( m_game, m_world, this, entityDef, element ); break;
	case ENTITY_TYPE_ACTOR:			spawnedEntity = new Actor( m_game, m_world, this, entityDef, element ); break;
	case ENTITY_TYPE_PROJECTILE:	spawnedEntity = new Projectile( m_game, m_world, this, entityDef, element ); break;
	case ENTITY_TYPE_PORTAL:		spawnedEntity = new Portal( m_game, m_world, this, entityDef, element ); break;
	default:
		ERROR_AND_DIE( "Tried to spawn an unsupported entity on map" )
		break;
	}

	AddEntityToMap( spawnedEntity );
	return spawnedEntity;
}


//---------------------------------------------------------------------------------------------------------
void Map::AddEntityToMap( Entity* entityToAdd )
{
	entityToAdd->SetMap( this );
	EntityType entityType = entityToAdd->GetEntityType();
	switch( entityType )
	{
	case ENTITY_TYPE_ENTITY:		break;
	case ENTITY_TYPE_ACTOR:			AddEntityToList( m_actors, entityToAdd ); break;
	case ENTITY_TYPE_PROJECTILE:	AddEntityToList( m_projectiles, entityToAdd ); break;
	case ENTITY_TYPE_PORTAL:		AddEntityToList( m_portals, entityToAdd ); break;
	default:
		ERROR_AND_DIE( "Could not add entity to map" );
		break;
	}

	AddEntityToList( m_entities, entityToAdd );
}


//---------------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromMap( Entity* entityToRemove )
{
	entityToRemove->SetMap( nullptr );
	EntityType entityType = entityToRemove->GetEntityType();
	switch( entityType )
	{
	case ENTITY_TYPE_ENTITY:		break;
	case ENTITY_TYPE_ACTOR:			RemoveEntityFromList( m_actors, entityToRemove );		break;
	case ENTITY_TYPE_PROJECTILE:	RemoveEntityFromList( m_projectiles, entityToRemove );	break;
	case ENTITY_TYPE_PORTAL:		RemoveEntityFromList( m_portals, entityToRemove );		break;
	default:
		ERROR_AND_DIE( "Could not remove entity from map" );
		break;
	}

	RemoveEntityFromList( m_entities, entityToRemove );

	if( m_playerStartEntity == entityToRemove )
	{
		m_playerStartEntity = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::DeleteAllEntities()
{
	for( int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* entityToDelete = m_entities[entityIndex];
		RemoveEntityFromMap( entityToDelete );
		delete entityToDelete;
	}
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::GetClosestEntityInForwardSector( Vec3 const& sectorStartPosition, float maxDistanceToCheck, Vec3 const& forwardDirNormalized, float aperatureDegrees )
{
	float distanceToClosestEntity = 100.f;
	Entity* closestEntity = nullptr;

	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntityToCheck = m_entities[entityIndex];
		if( currentEntityToCheck != nullptr )
		{
			Vec3 entityPosition = currentEntityToCheck->GetPosition();
			if( IsPointInForwardSector3D( entityPosition, sectorStartPosition, maxDistanceToCheck, forwardDirNormalized, aperatureDegrees ) )
			{
				Vec3 displacementToEntity = entityPosition - sectorStartPosition;
				float distanceToEntity = DotProduct3D( forwardDirNormalized, displacementToEntity );
				if( distanceToEntity < distanceToClosestEntity )
				{
					distanceToClosestEntity = distanceToEntity;
					closestEntity = currentEntityToCheck;
				}
			}
		}
	}
	return closestEntity;
}


//---------------------------------------------------------------------------------------------------------
void Map::AddEntityToList( std::vector<Entity*>& listToAddTo, Entity* entityToAdd )
{
	for( uint listIndex = 0; listIndex < listToAddTo.size(); ++listIndex )
	{
		if( listToAddTo[ listIndex ] == nullptr )
		{
			listToAddTo[listIndex] = entityToAdd;
			return;
		}
	}
	listToAddTo.push_back( entityToAdd );
}


//---------------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromList( std::vector<Entity*>& listToRemoveFrom, Entity* entityToRemove )
{
	for( uint listIndex = 0; listIndex < listToRemoveFrom.size(); ++listIndex )
	{
		if( listToRemoveFrom[ listIndex ] == entityToRemove )
		{
			listToRemoveFrom[ listIndex ] = nullptr;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleEntityVEntityCollisions()
{
	for( uint effectedEntityIndex = 0; effectedEntityIndex < m_entities.size(); ++effectedEntityIndex )
	{
		Entity* effectedEntity = m_entities[ effectedEntityIndex ];
		if( effectedEntity != nullptr )
		{
			HandleEntityCollisions( effectedEntity );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleEntityCollisions( Entity* effectedEntity )
{
	for( uint effectorEntityIndex = 0; effectorEntityIndex < m_entities.size(); ++effectorEntityIndex )
	{
		Entity* effectorEntity = m_entities[ effectorEntityIndex ];
		if( effectorEntity != nullptr && effectorEntity != effectedEntity )
		{
			HandleEntityVEntityCollision( effectedEntity, effectorEntity );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandleEntityVEntityCollision( Entity* effectedEntity, Entity* effectorEntity )
{
	if( ( !effectedEntity->IsPushedByEntities() && !effectorEntity->IsPushedByEntities() )	||
		( !effectedEntity->CanPushEntities() && !effectorEntity->CanPushEntities() )		||
		( !effectedEntity->IsPushedByEntities() && !effectedEntity->CanPushEntities() )		||
		( !effectorEntity->IsPushedByEntities() && !effectorEntity->CanPushEntities() )	)	
		return;

	Vec3 effectedEntityPosition = effectedEntity->GetPosition();
	Vec3 effectorEntityPosition = effectorEntity->GetPosition();
	
	Vec2 effectedEntityPositionXY = Vec2( effectedEntityPosition.x, effectedEntityPosition.y );
	Vec2 effectorEntityPositionXY = Vec2( effectorEntityPosition.x, effectorEntityPosition.y );

	float effectedEntityRadius = effectedEntity->GetPhysicsRadius();
	float effectorEntityRadius = effectorEntity->GetPhysicsRadius();

	if( DoDiscsOverlap( effectedEntityPositionXY, effectedEntityRadius, effectorEntityPositionXY, effectorEntityRadius ) )
	{
		float effectedMass = effectedEntity->GetMass();
		float effectorMass = effectorEntity->GetMass();
		float effectedMovePercent = effectorMass / ( effectedMass + effectorMass );
		float effectorMovePercent = 1.0f - effectedMovePercent;

		if( ( !effectedEntity->IsPushedByEntities() && effectedEntity->CanPushEntities() ) && effectorEntity->IsPushedByEntities() )
		{
			effectorMovePercent = 1.f;
			effectedMovePercent = 0.f;
		}
		else if( ( !effectorEntity->IsPushedByEntities() && effectorEntity->CanPushEntities() ) && effectedEntity->IsPushedByEntities() )
		{
			effectorMovePercent = 0.f;
			effectedMovePercent = 1.f;
		}
		else if( ( effectedEntity->IsPushedByEntities() && !effectedEntity->CanPushEntities() ) && effectorEntity->CanPushEntities() )
		{
			effectorMovePercent = 0.f;
			effectedMovePercent = 1.f;
		}
		else if( ( effectorEntity->IsPushedByEntities() && !effectorEntity->CanPushEntities() ) && effectedEntity->CanPushEntities() )
		{
			effectorMovePercent = 1.f;
			effectedMovePercent = 0.f;
		}

		Vec2 correctionDirection = effectedEntityPositionXY - effectorEntityPositionXY;
		Vec2 correctionDirectionNormalized = correctionDirection.GetNormalized();
		float overlapDistance = ( effectedEntityRadius + effectorEntityRadius ) - correctionDirection.GetLength();

		float effectedCorrectionDistance = overlapDistance * effectedMovePercent;
		float effectorCorrectionDistance = overlapDistance * effectorMovePercent;

		effectedEntity->Translate( Vec3( correctionDirectionNormalized * effectedCorrectionDistance, 0.f ) );
		effectorEntity->Translate( Vec3( -correctionDirectionNormalized * effectorCorrectionDistance, 0.f ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandlePortalVEntityCollisions()
{
	for( int portalIndex = 0; portalIndex < m_portals.size(); ++portalIndex )
	{
		Entity* portalToUseAsEntity = m_portals[portalIndex];
		for( int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
		{
			Entity* entityToCheck = m_entities[entityIndex];
			if( entityToCheck != nullptr )
			{
				HandlePortalVEntityCollision( portalToUseAsEntity, entityToCheck );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Map::HandlePortalVEntityCollision( Entity* portalEntity, Entity* entity )
{
	if( dynamic_cast<Portal*>( entity ) != nullptr )
		return;

	Vec3 portalPosition = portalEntity->GetPosition();
	Vec3 entityPosition = entity->GetPosition();

	Vec2 portalPositionXY = Vec2( portalPosition.x, portalPosition.y );
	Vec2 entityPositionXY = Vec2( entityPosition.x, entityPosition.y );

	float portalRadius = portalEntity->GetPhysicsRadius();
	float entityRadius = entity->GetPhysicsRadius();

	if( DoDiscsOverlap( portalPositionXY, portalRadius, entityPositionXY, entityRadius ) )
	{
		Portal* portal = static_cast<Portal*>( portalEntity );
		portal->UsePortal( entity );
	}
}
