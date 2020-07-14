#pragma once
#include "Game/Item.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Timer.hpp"
#include <string>
#include <vector>
#include <map>

class Game;
class Item;
class SpriteAnimDefinition;
class SpriteSheet;
struct Rgba8;

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
enum ActorState
{
	ACTOR_STATE_IDLE,
	ACTOR_STATE_WALK,
	ACTOR_STATE_ATTACK,
	ACTOR_STATE_ATTACK_MOVE,
};

//---------------------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( Game* theGame );
	Actor( Game* theGame, float movementSpeed, float attacksPerSecond, int attackDamage, float critChanceFraction );
	virtual ~Actor();

	virtual void RenderHealthBar( Rgba8 const& healthBarColor ) const;
	virtual void TakeDamage( int damageToTake );

	int		GetCritDamage() const;
	int		GetAttackDamage() const;
	float	GetCritMultiplier() const;
	float	GetMoveSpeed() const;
	float	GetAttackSpeed() const;
	float	GetCritChanceFraction() const;

	int		GetDamageToDeal() const;

	void	AddStatusEffect( StatMod const& statModifier, float duration );
	void	PickUpItem( Item const& itemToPickUp );
	void	SetIsMoving( bool isMoving );

protected:
	virtual void UpdateStatusEffects();
	virtual void ApplyStatusEffect( StatusEffect* statusEffect );
	virtual void RemoveStatusEffect( StatusEffect* statusEffect );
	virtual void SetMovePosition( Vec2 const& positionToMoveTo );

	void CalculateBonusStats();
	void ZeroBonusStats();

	SpriteAnimDefinition* GetSpriteAnimByPath( std::string const& animName );
	void UpdateAnimSpriteBasedOnMovementDirection( char const* pathToAnims );
	void CreateSpriteAnimFromPath( char const* filepath );
	void DetermineDirection( float& directionValue, std::string& currentDirection, std::string const& newDirection, Vec2 const& directionVector );
	void MoveTowardsPosition( float deltaSeconds );
	

public:
	static char const* GetActorStateAsString( ActorState playerState );

protected:
	//Stats
	int		m_currentHealth				= 100;
	int		m_maxHealth					= 100;
	int		m_bonusMaxHealth			= 0;

	float	m_critDamageMultiplier		= 1.5f;	
	float	m_bonusCritDamageMultiplier	= 0.f;

	float	m_movementSpeedPerSecond		= 1.f;
	float	m_bonusMovementSpeedPerSecond	= 0.f;
	
	float	m_attacksPerSecond			= 1.f;
	float	m_bonusAttacksPerSecond		= 0.f;
	
	int		m_attackDamage				= 20;
	int		m_bonusAttackDamage			= 0;
	
	float	m_critChanceFraction		= 0.1f;
	float	m_bonusCritChanceFraction	= 0.f;
	
	float	m_attackRange				= 2.f;
	float	m_bonusAttackRange			= 0.f;

	Timer m_attackTimer;

	Vec2		m_positionToMoveTo;
	ActorState	m_actorState = ACTOR_STATE_IDLE;

	SpriteAnimDefinition* m_anim = nullptr;
	std::vector<SpriteSheet*> m_spriteSheets;
	std::map<std::string, SpriteAnimDefinition*> m_spriteAnimsBySheetName;

	//Other
	std::vector<StatusEffect> m_statusEffects;
	std::map<std::string, int> m_heldItems;
};