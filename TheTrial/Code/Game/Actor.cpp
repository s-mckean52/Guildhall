#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


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
}


//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame )
	: Entity( theGame )
{
}

//---------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
}


//---------------------------------------------------------------------------------------------------------
void Actor::RenderHealthBar( Rgba8 const& healthBarColor ) const
{
	const float healthBarHeight = 0.1f;
	float healthBarWidth = m_physicsRadius * 2.f;
	Vec2 healthBarMin = m_currentPosition + Vec2( -healthBarWidth * 0.5f, m_renderBounds.GetDimensions().y * 0.5f + healthBarHeight );
	Vec2 healthBarMax = healthBarMin + Vec2( healthBarWidth, healthBarHeight );
	AABB2 healthBarBackGround = AABB2( healthBarMin, healthBarMax );

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
int Actor::GetCritDamage() const
{
	return static_cast<int>( m_attackDamage * m_critDamageMultiplier );
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
ActorStat Actor::GetStatTypeFromString( std::string statTypeAsString )
{
	if( statTypeAsString == "CritMultiplier" )		{ return STAT_CRIT_MULTIPLIER; }
	else if( statTypeAsString == "CritChance" )		{ return STAT_CRIT_CHANCE; }
	else if( statTypeAsString == "MoveSpeed" )		{ return STAT_MOVEMENT_SPEED; }
	else if( statTypeAsString == "AttackSpeed" )	{ return STAT_ATTACK_SPEED; }
	else if( statTypeAsString == "AttackDamage" )	{ return STAT_ATTACK_DAMAGE; }
	else
	{
		ERROR_AND_DIE( "Read an unsupported string for actor stat" );
		return STAT_ATTACK_DAMAGE;
	}
}