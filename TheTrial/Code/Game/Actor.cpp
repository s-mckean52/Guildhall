#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//---------------------------------------------------------------------------------------------------------
//	StatMod
//---------------------------------------------------------------------------------------------------------
StatMod::StatMod( ActorStat newStatToMod, float newAmountToAdd )
{
	statToMod = newStatToMod;
	amountToAdd = newAmountToAdd;
}
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//	StatusEffect
//---------------------------------------------------------------------------------------------------------
bool StatusEffect::HasExpired()
{
	return statusEffectTimer.HasElapsed();
}
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// Actor
//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame, float movementSpeed, float attacksPerSecond, int attackDamage, float critChanceFraction )
	: Entity( theGame )
{
	m_movementSpeedPerSecond = movementSpeed;
	m_attacksPerSecond = attacksPerSecond;
	m_attackDamage = attackDamage;
	m_critChanceFraction = critChanceFraction;
	
	m_attackTimer.SetSeconds( theGame->GetGameClock(), 0.0 );
}


//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame )
	: Entity( theGame )
{
	m_attackTimer.SetSeconds( theGame->GetGameClock(), 0.0 );
}

//---------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
}


//---------------------------------------------------------------------------------------------------------
void Actor::RenderHealthBar( Rgba8 const& healthBarColor ) const
{
	const float healthBarHeight = 0.1f;
	const float healthBarBackGroundPadding = 0.05f;
	float healthBarWidth = m_physicsRadius * 2.f;
	Vec2 healthBarMin = m_currentPosition + Vec2( -healthBarWidth * 0.5f, m_renderBounds.GetDimensions().y * 0.5f + healthBarHeight );
	Vec2 healthBarMax = healthBarMin + Vec2( healthBarWidth, healthBarHeight );
	AABB2 healthBarBackGround = AABB2( healthBarMin, healthBarMax );
	healthBarBackGround.SetDimensions( Vec2( healthBarWidth + healthBarBackGroundPadding, healthBarHeight + healthBarBackGroundPadding ) );

	float currentHealthFraction = static_cast<float>( m_currentHealth ) / static_cast<float>( m_maxHealth );
	float currentHealthLength = healthBarWidth * currentHealthFraction;
	Vec2 currentHealthMax = healthBarMin + Vec2( currentHealthLength, healthBarHeight );
	AABB2 currentHealth = AABB2( healthBarMin, currentHealthMax );
	
	std::vector<Vertex_PCU> healthBarVerts;
	AppendVertsForAABB2D( healthBarVerts, healthBarBackGround, RGBA8_DARK_GREY );
	AppendVertsForAABB2D( healthBarVerts, currentHealth, healthBarColor );

	g_theRenderer->BindMaterial( nullptr );
	g_theRenderer->DrawVertexArray( healthBarVerts );
}


//---------------------------------------------------------------------------------------------------------
void Actor::UpdateStatusEffects()
{
	for( uint statusEffectIndex = 0; statusEffectIndex < m_statusEffects.size(); ++statusEffectIndex )
	{
		StatusEffect* currentStatusEffect = &m_statusEffects[statusEffectIndex];
		if( currentStatusEffect->HasExpired() )
		{
			RemoveStatusEffect( currentStatusEffect );
			m_statusEffects.erase( m_statusEffects.begin() + statusEffectIndex );
			continue;
		}
		else if( !currentStatusEffect->hasBeenApplied )
		{ 
			ApplyStatusEffect( currentStatusEffect );
			currentStatusEffect->hasBeenApplied = true;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Actor::ApplyStatusEffect( StatusEffect* statusEffect )
{
	StatMod* statMod	= &statusEffect->statMod;
	ActorStat statToMod = statMod->statToMod;
	float amountToAdd	= statMod->amountToAdd;
	
	switch( statToMod )
	{
	case STAT_CRIT_MULTIPLIER:	m_critDamageMultiplier		+= amountToAdd;						break;
	case STAT_CRIT_CHANCE:		m_critChanceFraction		+= amountToAdd;						break;
	case STAT_MOVEMENT_SPEED:	m_movementSpeedPerSecond	+= amountToAdd;						break;
	case STAT_ATTACK_SPEED:		m_attacksPerSecond			+= amountToAdd;						break;
	case STAT_ATTACK_DAMAGE:	m_attackDamage				+= static_cast<int>( amountToAdd );	break;

	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Actor::RemoveStatusEffect( StatusEffect* statusEffect )
{
	StatMod* statMod	= &statusEffect->statMod;
	ActorStat statToMod = statMod->statToMod;
	float amountToAdd	= statMod->amountToAdd;
	
	switch( statToMod )
	{
	case STAT_CRIT_MULTIPLIER:	m_critDamageMultiplier		-= amountToAdd;						break;
	case STAT_CRIT_CHANCE:		m_critChanceFraction		-= amountToAdd;						break;
	case STAT_MOVEMENT_SPEED:	m_movementSpeedPerSecond	-= amountToAdd;						break;
	case STAT_ATTACK_SPEED:		m_attacksPerSecond			-= amountToAdd;						break;
	case STAT_ATTACK_DAMAGE:	m_attackDamage				-= static_cast<int>( amountToAdd );	break;

	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Actor::CalculateBonusStats()
{
	ZeroBonusStats();
	for( auto iter = m_heldItems.begin(); iter != m_heldItems.end(); ++iter )
	{
		Item const& itemStatsToAdd = Item::GetItemDefByName( iter->first );
		for( int statModIndex = 0; statModIndex < itemStatsToAdd.GetNumStatMods(); ++statModIndex )
		{
			StatMod const& statModToAdd = itemStatsToAdd.GetStatModAtIndex( statModIndex );
			switch( statModToAdd.statToMod )
			{
			case STAT_CRIT_MULTIPLIER:	m_bonusCritDamageMultiplier		+= statModToAdd.amountToAdd * iter->second;						break;
			case STAT_CRIT_CHANCE:		m_bonusCritChanceFraction		+= statModToAdd.amountToAdd * iter->second;						break;
			case STAT_MOVEMENT_SPEED:	m_bonusMovementSpeedPerSecond	+= statModToAdd.amountToAdd * iter->second;						break;
			case STAT_ATTACK_SPEED:		m_bonusAttacksPerSecond			+= statModToAdd.amountToAdd * iter->second;						break;
			case STAT_ATTACK_DAMAGE:	m_bonusAttackDamage				+= static_cast<int>( statModToAdd.amountToAdd * iter->second );	break;
			default:
				break;
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Actor::ZeroBonusStats()
{
	m_bonusMaxHealth				= 0; //NotImplemented
	m_bonusCritDamageMultiplier		= 0.f;
	m_bonusCritChanceFraction		= 0.f;
	m_bonusMovementSpeedPerSecond	= 0.f;
	m_bonusAttacksPerSecond			= 0.f;
	m_bonusAttackDamage				= 0;
	m_bonusAttackRange				= 0.f; //NotImplemented
}


//---------------------------------------------------------------------------------------------------------
SpriteAnimDefinition* Actor::GetSpriteAnimByPath( std::string const& animName )
{
	return m_spriteAnimsBySheetName[animName];
}


//---------------------------------------------------------------------------------------------------------
void Actor::UpdateAnimSpriteBasedOnMovementDirection( char const* pathToAnims )
{
	std::string currentDirection = "down";

	float directionValue = -1000.f;
	DetermineDirection( directionValue, currentDirection, "down",	Vec2(  0.f, -1.f )	);
	DetermineDirection( directionValue, currentDirection, "right",	Vec2(  1.f,  0.f )	);
	DetermineDirection( directionValue, currentDirection, "left",	Vec2( -1.f,  0.f )	);
	DetermineDirection( directionValue, currentDirection, "up",		Vec2(  0.f,  1.f )	);

	char const* playerStateAsString = GetActorStateAsString( m_actorState );
	SpriteAnimDefinition* directionAnim = GetSpriteAnimByPath( Stringf( "Data/Images/%s/%s/%s.png", pathToAnims, playerStateAsString, currentDirection.c_str() ) );
	m_anim = directionAnim;
}


//---------------------------------------------------------------------------------------------------------
void Actor::CreateSpriteAnimFromPath( char const* filepath )
{
	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( filepath );
	SpriteSheet* newSpriteSheet = new SpriteSheet( *spriteTexture, IntVec2( 6, 1 ) );

	m_spriteSheets.push_back( newSpriteSheet );
	m_spriteAnimsBySheetName[ filepath ] = new SpriteAnimDefinition( *newSpriteSheet, 0, 5, 1.f );
}

//---------------------------------------------------------------------------------------------------------
void Actor::DetermineDirection( float& directionValue, std::string& currentDirection, std::string const& newDirection, Vec2 const& directionVector )
{
	Vec2 movementDirection = m_positionToMoveTo - m_currentPosition;
	movementDirection.Normalize();

	float newDirectionValue = DotProduct2D( movementDirection, directionVector );
	if( newDirectionValue > directionValue )
	{
		directionValue = newDirectionValue;
		currentDirection = newDirection;
	}
}

//---------------------------------------------------------------------------------------------------------
void Actor::MoveTowardsPosition( float deltaSeconds )
{
	Vec2 displacementToDestination = m_positionToMoveTo - m_currentPosition;
	Vec2 directionTowardsDestination = displacementToDestination.GetNormalized();
	Vec2 movementVector = directionTowardsDestination * GetMoveSpeed() * deltaSeconds;

	float displacementProjectedDistance = GetProjectedLength2D( displacementToDestination, directionTowardsDestination );
	float movementProjectedDistance = GetProjectedLength2D( movementVector, directionTowardsDestination );

	if( displacementProjectedDistance < movementProjectedDistance )
	{
		movementVector = displacementToDestination;
	}
	m_currentPosition += movementVector;

	if( m_currentPosition == m_positionToMoveTo )
	{
		m_actorState = ACTOR_STATE_IDLE;
	}
}

//---------------------------------------------------------------------------------------------------------
void Actor::SetMovePosition( Vec2 const& positionToMoveTo )
{
	m_positionToMoveTo = positionToMoveTo;
}

//---------------------------------------------------------------------------------------------------------
void Actor::SetIsMoving( bool isMoving )
{
	if( isMoving )
	{
		m_actorState = ACTOR_STATE_WALK;
	}
	else
	{
		m_actorState = ACTOR_STATE_IDLE;
	}
}

//---------------------------------------------------------------------------------------------------------
int Actor::GetCritDamage() const
{
	return static_cast<int>( GetAttackDamage() * GetCritMultiplier() );
}


//---------------------------------------------------------------------------------------------------------
int Actor::GetAttackDamage() const
{
	return m_attackDamage + m_bonusAttackDamage;
}


//---------------------------------------------------------------------------------------------------------
float Actor::GetCritMultiplier() const
{
	return m_critDamageMultiplier + m_bonusCritDamageMultiplier;
}


//---------------------------------------------------------------------------------------------------------
float Actor::GetMoveSpeed() const
{
	return m_movementSpeedPerSecond + m_bonusMovementSpeedPerSecond;
}


//---------------------------------------------------------------------------------------------------------
float Actor::GetAttackSpeed() const
{
	return m_attacksPerSecond + m_bonusAttacksPerSecond;
}


//---------------------------------------------------------------------------------------------------------
float Actor::GetCritChanceFraction() const
{
	return GetClamp( m_critChanceFraction + m_bonusCritChanceFraction, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
int Actor::GetDamageToDeal() const
{
	if( g_RNG->RollPercentChance( GetCritChanceFraction() ) )
	{
		return GetCritDamage();
	}
	return GetAttackDamage();
}


//---------------------------------------------------------------------------------------------------------
void Actor::AddStatusEffect( StatMod const& statModifier, float duration )
{
	StatusEffect newStatusEffect;
	newStatusEffect.statMod = statModifier;
	newStatusEffect.statusEffectTimer.SetSeconds( m_theGame->GetGameClock(), duration );

	m_statusEffects.push_back( newStatusEffect );
}


//---------------------------------------------------------------------------------------------------------
void Actor::TakeDamage( int damageToTake )
{
	m_currentHealth -= damageToTake;
	if( m_currentHealth <= 0 )
	{
		m_isDead = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Actor::PickUpItem( Item const& itemToPickUp )
{
	auto heldItemsIter = m_heldItems.find( itemToPickUp.GetName() );
	if( heldItemsIter != m_heldItems.end() )
	{
		++heldItemsIter->second;
	}
	else
	{
		m_heldItems[ itemToPickUp.GetName() ] = 1;
	}

	CalculateBonusStats();
}


//---------------------------------------------------------------------------------------------------------
STATIC char const* Actor::GetActorStateAsString( ActorState playerState )
{
	switch (playerState)
	{
	case ACTOR_STATE_IDLE: return "Idle";
	case ACTOR_STATE_ATTACK_MOVE:
	case ACTOR_STATE_WALK: return "Walk";
	case ACTOR_STATE_ATTACK: return "Attack";
	default:
		ERROR_AND_DIE( "Invalid PlayerState" );
		break;
	}
}