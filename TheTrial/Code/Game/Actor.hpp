#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Timer.hpp"
#include <string>
#include <vector>

class Game;
struct Rgba8;

//---------------------------------------------------------------------------------------------------------
enum ActorStat
{
	STAT_CRIT_MULTIPLIER,
	STAT_CRIT_CHANCE,
	STAT_MOVEMENT_SPEED,
	STAT_ATTACK_SPEED,
	STAT_ATTACK_DAMAGE,
};

//---------------------------------------------------------------------------------------------------------
struct StatMod
{
public:
	ActorStat statToMod = STAT_ATTACK_DAMAGE;
	float amountToAdd = 0.f;

public:
	StatMod() = default;
	StatMod( ActorStat newStatToMod, float newAmountToAdd );
};


//---------------------------------------------------------------------------------------------------------
struct StatusEffect
{
public:
	StatMod statMod;
	Timer	statusEffectTimer;
	bool	hasBeenApplied		= false;

public:
	StatusEffect() = default;
	~StatusEffect() {};

	bool HasExpired();
};


//---------------------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( Game* theGame );
	Actor( Game* theGame, float movementSpeed, float attacksPerSecond, int attackDamage, float critChanceFraction );
	virtual ~Actor();

	virtual void RenderHealthBar( Rgba8 const& healthBarColor ) const;

	int		GetCritDamage() const;
	int		GetAttackDamage() const			{ return m_attackDamage; }
	float	GetCritMultiplier() const		{ return m_critDamageMultiplier; }
	float	GetMoveSpeed() const			{ return m_movementSpeedPerSecond; }
	float	GetAttackSpeed() const			{ return m_attacksPerSecond; }
	float	GetCritChanceFraction() const	{ return m_critChanceFraction; }
	bool	IsDead() const					{ return m_isDead; }

	void	AddStatusEffect( StatMod const& statModifier, float duration );
	void	TakeDamage( int damageToTake );

public:
	static ActorStat GetStatTypeFromString( std::string statTypeAsString );

protected:
	virtual void UpdateStatusEffects();
	virtual void ApplyStatusEffect( StatusEffect* statusEffect );
	virtual void RemoveStatusEffect( StatusEffect* statusEffect );


protected:
	//Stats
	int		m_currentHealth				= 100;
	int		m_maxHealth					= 100;
	float	m_critDamageMultiplier		= 1.5f;
	float	m_movementSpeedPerSecond	= 1.f;
	float	m_attacksPerSecond			= 1.f;
	int		m_attackDamage				= 20;
	float	m_critChanceFraction		= 0.1f;
	float	m_attackRange				= 2.f;

	//Other
	bool m_isDead = false;
	std::vector<StatusEffect> m_statusEffects;
};