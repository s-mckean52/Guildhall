#pragma once
#include "Engine/Math/Vec2.hpp"
#include <string>

class Game;
class World;
class Map;

enum EntityType
{
	ENTITY_PLAYER,
	ENTITY_PLAYER_START,

	NUM_ENTITY_TYPES,
	INVALID_ENTITY_TYPE,
};

class Entity
{
public:
	Entity( Game* theGame, World* theWorld, Map* theMap );
	virtual ~Entity();

	//---------------------------------------------------------------------------------------------------------
	virtual void Update();
	virtual void Render() const;
	virtual void DebugRender() const;

	//---------------------------------------------------------------------------------------------------------
	const Vec2&	GetPosition()			{ return m_position; }
	float		GetYaw()				{ return m_yaw; }

	//---------------------------------------------------------------------------------------------------------
	void SetPosition( Vec2 const& position );
	void SetYaw( float yaw );

	//---------------------------------------------------------------------------------------------------------
	static EntityType GetEntityTypeFromString( std::string entityTypeAsString );

protected:
	Game*	m_theGame	= nullptr;
	World*	m_theWorld	= nullptr;
	Map*	m_theMap	= nullptr;

	Vec2	m_position;
	float	m_yaw = 0.f;
};