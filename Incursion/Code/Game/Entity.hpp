#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Map;
class Entity;

enum EntityFaction
{
	INVALID_ENTITY_FACTION = -1,

	FACTION_GOOD,
	FACTION_EVIL,
	FACTION_NEUTRAL,

	NUM_ENTITY_FACTIONS
};

enum EntityType
{
	INVALID_ENTITY_TYPE = -1,

	ENTITY_TYPE_BOULDER,
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_BAD_BULLET,
	ENTITY_TYPE_NPC_TANK,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_NPC_TURRET,
	ENTITY_TYPE_EXPLOSION,	

	NUM_ENTITY_TYPES
};

typedef std::vector<Entity*> EntityList;

class Entity
{
public:
	Entity() {};
	~Entity() {};

	explicit Entity( Map* theMap, const Vec2& startPosition, EntityFaction faction );
	
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void DebugDraw() const;
	virtual void TakeDamage( int damageDealt );

	const Vec2		GetForwardVector()		const { return m_velocity.GetNormalized(); }
	float			GetPhysicsRadius()		const { return m_physicsRadius; }
	float			GetCosmeticRadius()		const { return m_cosmeticRadius; }
	const Vec2		GetPosition()			const { return m_position; }
	const Rgba8		GetColor()				const { return m_color; }
	int				GetNumberOfDebris()		const { return m_numberOfDebris; }
	EntityFaction	GetFaction()			const { return m_faction; }
	EntityType		GetType()				const { return m_entityType; }
	float			GetSpeed()				const { return m_velocity.GetLength(); }
	
	bool		IsDead()				const { return m_isDead; }
	bool		IsGarbage()				const { return m_isGarbage; }
	
	bool		IsPushedByEntities()	const { return m_pushedByOtherEntities; }
	bool		IsPushedByWalls()		const { return m_pushedByWalls; }
	bool		DoesPushEntities()		const { return m_pushesOtherEntities; }
	bool		IsHitByBullet()			const { return m_isHitByBullet; }


	void		SetPosition( const Vec2& newPosition );
	void		SetOrientationDegrees( float newOrientationDegrees );
	void		SetMap( Map* theMap );
	float		GetSlowFractionOnMud();
	
	void		HealthRegen( float deltaSeconds );
	void		RenderHealthBar( const Rgba8& healthColor, const Rgba8& backgroundColor = Rgba8::BLACK ) const;


	bool		IsOffScreen() const;


protected:
	Map* m_map	= nullptr;

	Vec2	m_position;
	Vec2	m_velocity;
	Vec2	m_acceleration;
	Rgba8	m_color = Rgba8::WHITE;
	AABB2	m_spriteBox;

	EntityType m_entityType = INVALID_ENTITY_TYPE;
	EntityFaction m_faction = INVALID_ENTITY_FACTION;

	float	m_healthRegenTimer		= 0.f;
	int		m_maxHealth				= 1;
	int		m_health				= 1;
	bool	m_isDead				= false;
	bool	m_isGarbage				= false;

	float	m_orientationDegrees	= 0.f;
	float	m_angularVelocity		= 0.f;
	float	m_physicsRadius			= 10.f;		//Noticeably Big
	float	m_cosmeticRadius		= 20.f;		//Noticeably Big
	float	m_scale					= 1.f;
	int		m_numberOfDebris		= 3;

	bool	m_pushedByOtherEntities = true;
	bool	m_pushedByWalls			= true;
	bool	m_pushesOtherEntities	= true;

	bool	m_isHitByBullet			= true;
};
