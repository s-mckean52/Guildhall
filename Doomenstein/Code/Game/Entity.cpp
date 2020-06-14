#include "Game/Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, World* theWorld, Map* theMap )
{
	m_theGame = theGame;
	m_theWorld = theWorld;
	m_theMap = theMap;
}


//---------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Update()
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetPosition( Vec2 const& position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetYaw( float yaw )
{
	m_yaw = yaw;
}


//---------------------------------------------------------------------------------------------------------
STATIC EntityType Entity::GetEntityTypeFromString( std::string entityTypeAsString )
{
	EntityType entityType = INVALID_ENTITY_TYPE;

	if( entityTypeAsString == "Player" )				entityType = ENTITY_PLAYER;
	else if( entityTypeAsString == "PlayerStart" )		entityType = ENTITY_PLAYER_START;

	return entityType;
}

