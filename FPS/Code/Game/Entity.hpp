#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Entity;
class Map;

enum EntityFaction
{
	FACTION_GOOD,
	FACTION_EVIL,
	FACTION_NEUTRAL,

	NUM_ENTITY_FACTIONS
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
	float			GetSpeed()				const { return m_velocity.GetLength(); }
	bool			CanFly()				const { return m_canFly; }
	bool			CanSwim()				const { return m_canSwim; }
	bool			CanWalk()				const { return m_canWalk; }

	
	bool		IsDead()				const { return m_isDead; }
	bool		IsGarbage()				const { return m_isGarbage; }


	void		SetPosition( const Vec2& newPosition );
	void		SetOrientationDegrees( float newOrientationDegrees );

	bool		IsOffScreen() const;


protected:
	Map*	m_map = nullptr;

	EntityFaction	m_faction			= FACTION_NEUTRAL;
	Vec2			m_position;
	Vec2			m_velocity;
	Vec2			m_acceleration;
	Rgba8			m_color				= Rgba8::WHITE;
	AABB2			m_spriteBox;


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

	bool	m_canWalk				= true;
	bool	m_canFly				= false;
	bool	m_canSwim				= false;
};
