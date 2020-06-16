#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Blink.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Player::Player( Game* theGame )
	: Actor( theGame )
{
	m_movementSpeedPerSecond = 2.f;
	AssignAbilityToSlot( "Blink", 0 );
	AssignAbilityToSlot( "Enrage", 1 );
	AssignAbilityToSlot( "Blink", 2 );
	AssignAbilityToSlot( "Blink", 3 );

	ABILITY_0_KEY = g_gameConfigBlackboard.GetValue( "ability0Key", ABILITY_0_KEY );
	ABILITY_1_KEY = g_gameConfigBlackboard.GetValue( "ability1Key", ABILITY_1_KEY );
	ABILITY_2_KEY = g_gameConfigBlackboard.GetValue( "ability2Key", ABILITY_2_KEY );
	ABILITY_3_KEY = g_gameConfigBlackboard.GetValue( "ability3Key", ABILITY_3_KEY );

	m_attackTimer.SetSeconds( theGame->GetGameClock(), 0.0 );
}


//---------------------------------------------------------------------------------------------------------
Player::~Player()
{
	for( int i = 0; i < 4; ++i )
	{
		delete m_abilities[i];
		m_abilities[i] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	UpdateStatusEffects();

	if( g_theInput->WasKeyJustPressed( ABILITY_0_KEY ) )
	{
		Ability* abilityToUse = m_abilities[0];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( ABILITY_1_KEY ) )
	{
		Ability* abilityToUse = m_abilities[1];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( ABILITY_2_KEY ) )
	{
		Ability* abilityToUse = m_abilities[2];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( ABILITY_3_KEY ) )
	{
		Ability* abilityToUse = m_abilities[3];
		abilityToUse->Use();
	}

	if( m_enemyTarget )
	{
		if( !DoDiscsOverlap( m_currentPosition, m_attackRange, m_enemyTarget->GetCurrentPosition(), m_enemyTarget->GetPhysicsRadius() ) )
		{
			m_isMoving = true;
			m_positionToMoveTo = m_enemyTarget->GetCurrentPosition();
		}
		else
		{
			m_isMoving = false;
			BasicAttack( m_enemyTarget );
		}
	}

	if( m_isMoving )
	{
		MoveTowardsPosition( deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::MoveTowardsPosition( float deltaSeconds )
{
	Vec2 displacementToDestination = m_positionToMoveTo - m_currentPosition;
	Vec2 directionTowardsDestination = displacementToDestination.GetNormalized();
	Vec2 movementVector = directionTowardsDestination * m_movementSpeedPerSecond * deltaSeconds;

	float displacementProjectedDistance = GetProjectedLength2D( displacementToDestination, directionTowardsDestination );
	float movementProjectedDistance = GetProjectedLength2D( movementVector, directionTowardsDestination );

	if( displacementProjectedDistance < movementProjectedDistance )
	{
		movementVector = displacementToDestination;
	}
	m_currentPosition += movementVector;

	if( m_currentPosition == m_positionToMoveTo )
	{
		m_isMoving = false;
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	g_theRenderer->BindMaterial( nullptr );
	DrawCircleAtPoint( m_currentPosition, m_physicsRadius, Rgba8::MAGENTA, 0.1f );
	DrawCircleAtPoint( m_currentPosition, m_attackRange, Rgba8::CYAN, 0.1f );

	RenderHealthBar( Rgba8::GREEN );

	if( m_isMoving && m_enemyTarget == nullptr )
	{
		DrawCircleAtPoint( m_positionToMoveTo, 0.1f, Rgba8::YELLOW, 0.1f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::RenderAbilities( Vec2 const& abilityMinStartPos, float distanceBetweenAbilites ) const
{
	Vec2 abilityMinPos = abilityMinStartPos;
	for( uint abilityIndex = 0; abilityIndex < MAX_ABILITY_COUNT; ++abilityIndex )
	{
		Ability const* abilityToRender = m_abilities[ abilityIndex ];
		
		if( abilityToRender == nullptr ) 
			continue;
		
		abilityToRender->Render( abilityMinPos );
		abilityMinPos.x += ( ABILITY_UI_WIDTH * 0.5f ) + distanceBetweenAbilites;
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::AssignAbilityToSlot( std::string abilityName, int slotNumber )
{
	Clamp( slotNumber, 0, 3 );
	m_abilities[slotNumber] = Ability::GetNewAbilityByName( abilityName );
	
	if( m_abilities == nullptr )
		ERROR_AND_DIE( "Tried to assign nullptr to actor ability slot " );

	m_abilities[slotNumber]->SetGame( m_theGame );
	m_abilities[slotNumber]->SetOwner( this );
}


//---------------------------------------------------------------------------------------------------------
void Player::SetCurrentPosition( Vec2 const& position )
{
	//m_isMoving = false;
	m_currentPosition = position;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetMovePosition( Vec2 const& positionToMoveTo )
{
	m_enemyTarget = nullptr;
	m_positionToMoveTo = positionToMoveTo;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetIsMoving( bool isMoving )
{
	m_isMoving = isMoving;
}

//---------------------------------------------------------------------------------------------------------
void Player::AttackEnemy( Enemy* enemyToAttack )
{
	m_enemyTarget = enemyToAttack;
}


//---------------------------------------------------------------------------------------------------------
void Player::BasicAttack( Enemy* target )
{
	if( !m_attackTimer.HasElapsed() || target->IsDead() )
		return;

	Projectile* newBasicAttack = new Projectile( m_theGame, m_attackDamage, 5.f, target );
	newBasicAttack->SetCurrentPosition( m_currentPosition );
	m_theGame->AddEntityToList( newBasicAttack );

	float attackCooldownSeconds = 1.f / m_attacksPerSecond;
	m_attackTimer.SetSeconds( m_theGame->GetGameClock(), attackCooldownSeconds );
}

