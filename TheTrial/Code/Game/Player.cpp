#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Blink.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Clock.hpp"


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

	CreateSpriteAnimFromPath( "Data/Images/Player/Walk/down.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Walk/left.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Walk/right.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Walk/up.png" );

	CreateSpriteAnimFromPath( "Data/Images/Player/Idle/down.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Idle/left.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Idle/right.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Idle/up.png" );

	CreateSpriteAnimFromPath( "Data/Images/Player/Attack/down.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Attack/left.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Attack/right.png" );
	CreateSpriteAnimFromPath( "Data/Images/Player/Attack/up.png" );
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
			m_playerState = PLAYER_STATE_WALK;
			m_positionToMoveTo = m_enemyTarget->GetCurrentPosition();
		}
		else
		{
			m_playerState = PLAYER_STATE_ATTACK;
			BasicAttack( m_enemyTarget );
		}
	}

	if( m_playerState == PLAYER_STATE_WALK )
	{
		MoveTowardsPosition( deltaSeconds );
	}

	UpdateAnimSpriteBasedOnMovementDirection();
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
		m_playerState = PLAYER_STATE_IDLE;
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 worldSpriteBounds = m_renderBounds;
	worldSpriteBounds.SetCenter( m_currentPosition );

	Vec2 uvMin;
	Vec2 uvMax;
	Clock* gameClock = g_theGame->GetGameClock();
	float elapsedTime = static_cast<float>( gameClock->GetTotalElapsedSeconds() );
	SpriteDefinition const& spriteDef = m_anim->GetSpriteDefAtTime( elapsedTime * m_movementSpeedPerSecond );
	spriteDef.GetUVs( uvMin, uvMax );
	
	AppendVertsForAABB2D( verts, worldSpriteBounds, Rgba8::WHITE, uvMin, uvMax );
	g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( verts );


	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( (Shader*)nullptr );
	RenderHealthBar( Rgba8::GREEN );

	if( m_playerState == PLAYER_STATE_WALK && m_enemyTarget == nullptr )
	{
		DrawCircleAtPoint( m_positionToMoveTo, 0.1f, Rgba8::YELLOW, 0.1f );
	}

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::DebugRender() const
{
	Entity::DebugRender();

	DrawCircleAtPoint( m_currentPosition, m_attackRange, Rgba8::CYAN, DEBUG_THICKNESS );
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
SpriteAnimDefinition* Player::GetSpriteAnimByPath( std::string const& animName )
{
	return m_spriteAnimsBySheetName[ animName ];
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
void Player::CreateSpriteAnimFromPath( char const* filepath )
{
	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( filepath );
	SpriteSheet* newSpriteSheet = new SpriteSheet( *spriteTexture, IntVec2( 6, 1 ) );

	m_spriteSheets.push_back( newSpriteSheet );
	m_spriteAnimsBySheetName[ filepath ] = new SpriteAnimDefinition( *newSpriteSheet, 0, 5, 1.f );
}


//---------------------------------------------------------------------------------------------------------
void Player::UpdateAnimSpriteBasedOnMovementDirection()
{
	std::string currentDirection = "down";

	float directionValue = -1000.f;
	DetermineDirection( directionValue, currentDirection, "down",	Vec2(  0.f, -1.f )	);
	DetermineDirection( directionValue, currentDirection, "right",	Vec2(  1.f,  0.f )	);
	DetermineDirection( directionValue, currentDirection, "left",	Vec2( -1.f,  0.f )	);
	DetermineDirection( directionValue, currentDirection, "up",		Vec2(  0.f,  1.f )	);

	char const* playerStateAsString = GetPlayerStateAsString( m_playerState );
	SpriteAnimDefinition* directionAnim = GetSpriteAnimByPath( Stringf( "Data/Images/Player/%s/%s.png", playerStateAsString, currentDirection.c_str() ) );
	m_anim = directionAnim;
}


//---------------------------------------------------------------------------------------------------------
void Player::DetermineDirection( float& directionValue, std::string& currentDirection, std::string const& newDirection, Vec2 const& directionVector )
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
void Player::SetMovePosition( Vec2 const& positionToMoveTo )
{
	m_enemyTarget = nullptr;
	m_positionToMoveTo = positionToMoveTo;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetCurrentMap( Map* map )
{
	m_currentMap = map;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetIsMoving( bool isMoving )
{
	if( isMoving )
	{
		m_playerState = PLAYER_STATE_WALK;
	}
	else
	{
		m_playerState = PLAYER_STATE_IDLE;
	}
}

//---------------------------------------------------------------------------------------------------------
void Player::AttackEnemy( Enemy* enemyToAttack )
{
	m_enemyTarget = enemyToAttack;
}


//---------------------------------------------------------------------------------------------------------
void Player::BasicAttack( Enemy* target )
{
	if( target->IsDead() )
	{
		m_playerState = PLAYER_STATE_IDLE;
		return;
	}

	if( !m_attackTimer.HasElapsed() )
		return;

	Projectile* newBasicAttack = new Projectile( m_theGame, m_attackDamage, 5.f, target );
	newBasicAttack->SetCurrentPosition( m_currentPosition );
	m_currentMap->AddEntityToList( newBasicAttack );

	float attackCooldownSeconds = 1.f / m_attacksPerSecond;
	m_attackTimer.SetSeconds( m_theGame->GetGameClock(), attackCooldownSeconds );
}


//---------------------------------------------------------------------------------------------------------
STATIC char const* Player::GetPlayerStateAsString( PlayerState playerState )
{
	switch (playerState)
	{
	case PLAYER_STATE_IDLE: return "Idle";
	case PLAYER_STATE_WALK: return "Walk";
	case PLAYER_STATE_ATTACK: return "Attack";
	default:
		ERROR_AND_DIE( "Invalid PlayerState" );
		break;
	}
}

