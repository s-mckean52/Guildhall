#include "Game/PlayerEntity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Ability.hpp"
#include "Game/Ability_Blink.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"


//---------------------------------------------------------------------------------------------------------
PlayerEntity::PlayerEntity( Game* theGame )
	: Entity( theGame )
{
	m_movementSpeedPerSecond = 2.f;
	m_abilities[0] = new Blink( theGame, this );
}


//---------------------------------------------------------------------------------------------------------
PlayerEntity::~PlayerEntity()
{
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Update( float deltaSeconds )
{
	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_RIGHT ) )
	{
		m_isMoving = true;
		Vec2 cursorPosition = m_theGame->GetCursorPosition();
		SetMovePosition( cursorPosition );
	}

	if( g_theInput->WasKeyJustPressed( 'Q' ) )
	{
		Ability* abilityToUse = m_abilities[0];
		abilityToUse->Use();
	}

	if( m_isMoving )
	{
		MoveTowardsPosition(deltaSeconds);
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::MoveTowardsPosition( float deltaSeconds )
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
void PlayerEntity::Render() const
{
	g_theRenderer->BindMaterial( nullptr );
	DrawCircleAtPoint( m_currentPosition, 0.5f, Rgba8::MAGENTA, 0.1f );

	if( m_isMoving )
	{
		DrawCircleAtPoint( m_positionToMoveTo, 0.1f, Rgba8::RED, 0.1f );
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::RenderAbilities() const
{
	m_abilities[0]->Render();
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::SetCurrentPosition( Vec2 const& position )
{
	m_isMoving = false;
	m_currentPosition = position;
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::SetMovePosition( Vec2 const& positionToMoveTo )
{
	m_positionToMoveTo = positionToMoveTo;
}

