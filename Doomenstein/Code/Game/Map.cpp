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
Map::Map( Game* theGame, World* theWorld )
{
	m_game = theGame;
	m_world = theWorld;
}


//---------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_mapMesh;
	m_mapMesh = nullptr;

	//DeleteEntites();
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnPlayer( Camera* playerCamera )
{
	playerCamera->SetPosition( Vec3( m_playerStartPositionXY, PLAYER_HEIGHT ) );
	playerCamera->SetPitchYawRollRotationDegrees( 0.f, m_playerStartYaw, 0.f );
	g_theGame->PlaySpawnSound();
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnNewEntityOnMap( XmlElement const& element )
{
	const float	invalidYaw = ~0;
	const Vec2	invalidPos = Vec2( -1.f, -1.f );

	std::string entityTypeAsString = element.Name();
	EntityType entityType = EntityDef::GetEntityTypeFromString( entityTypeAsString );

	std::string entityDefName = ParseXmlAttribute( element, "name", "MISSING" );
	if( entityDefName == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse \"name\" of entity at line %i", element.GetLineNum() );
		return;
	}

	Vec2 entityPos = ParseXmlAttribute( element, "pos", invalidPos );
	if( entityPos == invalidPos )
	{
		g_theConsole->ErrorString( "Failed to parse \"pos\" of entity at line %i", element.GetLineNum() );
		return;
	}

	float entityYaw = ParseXmlAttribute( element, "yaw", invalidYaw );
	if( entityYaw == invalidYaw )
	{
		g_theConsole->ErrorString( "Failed to parse \"yaw\" of entity at line %i", element.GetLineNum() );
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

	Entity* spawnedEntity = SpawnNewEntityOfType( *entityDefToSpawn );
	spawnedEntity->SetPosition( Vec3( entityPos, 0.f ) );
	spawnedEntity->SetYaw( entityYaw );
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( std::string const& entityDefName )
{
	EntityDef* entityDef = EntityDef::GetEntityDefByName( entityDefName );
	if( entityDef == nullptr )
	{
		return nullptr;
	}
	return SpawnNewEntityOfType( *entityDef );
}


//---------------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( EntityDef const& entityDef )
{
	EntityType entityType = entityDef.GetEntityType();
	Entity* spawnedEntity = nullptr;
	switch( entityType )
	{
	case ENTITY_TYPE_ENTITY:		spawnedEntity = new Entity( m_game, m_world, this, entityDef ); break;
	case ENTITY_TYPE_ACTOR:			spawnedEntity = new Actor( m_game, m_world, this, entityDef ); break;
	case ENTITY_TYPE_PROJECTILE:	spawnedEntity = new Projectile( m_game, m_world, this, entityDef ); break;
	case ENTITY_TYPE_PORTAL:		spawnedEntity = new Portal( m_game, m_world, this, entityDef ); break;
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
