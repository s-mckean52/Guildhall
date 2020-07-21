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
Player::Player( Game* theGame, std::string const& characterType )
	: Actor( theGame )
{
	m_movementSpeedPerSecond = 2.f;
	m_characterType = characterType;

	AssignAbilityToSlot( "Blink", 0 );
	AssignAbilityToSlot( "Enrage", 1 );
	AssignAbilityToSlot( "Blink", 2 );
	AssignAbilityToSlot( "Blink", 3 );

	m_abilityKeys[0] = g_gameConfigBlackboard.GetValue( "ability0Key", m_abilityKeys[0] );
	m_abilityKeys[1] = g_gameConfigBlackboard.GetValue( "ability1Key", m_abilityKeys[1] );
	m_abilityKeys[2] = g_gameConfigBlackboard.GetValue( "ability2Key", m_abilityKeys[2] );
	m_abilityKeys[3] = g_gameConfigBlackboard.GetValue( "ability3Key", m_abilityKeys[3] );

	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Walk/down.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Walk/left.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Walk/right.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Walk/up.png"		, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Idle/down.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Idle/left.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Idle/right.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Idle/up.png"		, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Attack/down.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Attack/left.png"	, characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Attack/right.png" , characterType.c_str() ).c_str() );
	CreateSpriteAnimFromPath( Stringf( "Data/Images/Player/%s/Attack/up.png"	, characterType.c_str() ).c_str() );

	SoundID walkingSound = g_theAudio->CreateOrGetSound( "Data/Audio/walking.wav" );
	m_walkingPlayback = g_theAudio->PlaySound( walkingSound, true, m_theGame->GetSFXVolume(), 0.f, GetMoveSpeed(), true );
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

	if( g_theInput->WasKeyJustPressed( m_abilityKeys[0] ) )
	{
		Ability* abilityToUse = m_abilities[0];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( m_abilityKeys[1] ) )
	{
		Ability* abilityToUse = m_abilities[1];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( m_abilityKeys[2] ) )
	{
		Ability* abilityToUse = m_abilities[2];
		abilityToUse->Use();
	}
	if( g_theInput->WasKeyJustPressed( m_abilityKeys[3] ) )
	{
		Ability* abilityToUse = m_abilities[3];
		abilityToUse->Use();
	}

	if( m_actorState == ACTOR_STATE_WALK )
	{
		MoveTowardsPosition( deltaSeconds );
	}
	else if( m_actorState == ACTOR_STATE_ATTACK_MOVE )
	{
		m_enemyTarget = m_currentMap->GetDiscOverlapEnemy( m_currentPosition, m_attackRange );
		if( m_enemyTarget == nullptr )
		{
			MoveTowardsPosition( deltaSeconds );
		}
	}

	if( m_enemyTarget != nullptr )
	{
		if( m_enemyTarget->IsDead() )
		{
			m_actorState = ACTOR_STATE_IDLE;
		}
		else if( !DoDiscsOverlap( m_currentPosition, m_attackRange, m_enemyTarget->GetCurrentPosition(), m_enemyTarget->GetPhysicsRadius() ) )
		{
			m_actorState = ACTOR_STATE_WALK;
			m_positionToMoveTo = m_enemyTarget->GetCurrentPosition();
		}
		else
		{
			m_actorState = ACTOR_STATE_ATTACK;
			BasicAttack( m_enemyTarget );
		}
	}

	SetAudioPlayback();
	UpdateAnimSpriteBasedOnMovementDirection( Stringf( "Player/%s", m_characterType.c_str() ).c_str() );
}


//---------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	if( m_actorState == ACTOR_STATE_WALK && m_enemyTarget == nullptr )
	{
		DrawCircleAtPoint( m_positionToMoveTo, 0.1f, Rgba8::YELLOW, 0.1f );
	}
	else if( m_actorState == ACTOR_STATE_ATTACK_MOVE && m_enemyTarget == nullptr )
	{
		DrawCircleAtPoint( m_positionToMoveTo, 0.1f, Rgba8::RED, 0.1f );
	}

	Vec2 uvMin;
	Vec2 uvMax;
	std::vector<Vertex_PCU> verts;
	AABB2 worldSpriteBounds = m_renderBounds;
	worldSpriteBounds.SetCenter( m_currentPosition );

	float animPlaySpeed = 0.f;
	if( m_actorState == ACTOR_STATE_ATTACK )
	{
		animPlaySpeed = GetAttackSpeed();
	}
	else
	{
		animPlaySpeed = GetMoveSpeed();
	}

	Clock* gameClock = g_theGame->GetGameClock();
	float elapsedTime = static_cast<float>( gameClock->GetTotalElapsedSeconds() );
	SpriteDefinition const& spriteDef = m_anim->GetSpriteDefAtTime( elapsedTime * animPlaySpeed );
	spriteDef.GetUVs( uvMin, uvMax );
	
	AppendVertsForAABB2D( verts, worldSpriteBounds, Rgba8::WHITE, uvMin, uvMax );
	g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( verts );

	RenderHealthBar( Rgba8::GREEN );

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
		
		abilityToRender->Render( abilityMinPos, m_abilityKeys[ abilityIndex ] );
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
	m_currentPosition = position;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetMovePosition( Vec2 const& positionToMoveTo )
{
	m_enemyTarget = nullptr;
	m_positionToMoveTo = positionToMoveTo;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetAttackMovePosition( Vec2 const& positionToMoveTo )
{
	SetMovePosition( positionToMoveTo );
	m_actorState = ACTOR_STATE_ATTACK_MOVE;
}


//---------------------------------------------------------------------------------------------------------
void Player::SetCurrentMap( Map* map )
{
	m_currentMap = map;
}


//---------------------------------------------------------------------------------------------------------
void Player::AttackEnemy( Enemy* enemyToAttack )
{
	m_enemyTarget = enemyToAttack;
}


//---------------------------------------------------------------------------------------------------------
void Player::BasicAttack( Enemy* target )
{
	if( !m_attackTimer.HasElapsed() )
		return;

	SoundID attackSound = g_theAudio->CreateOrGetSound( "Data/Audio/playerAttack.wav" );
	g_theAudio->PlaySound( attackSound, false, m_theGame->GetSFXVolume() );

	Projectile* newBasicAttack = new Projectile( m_theGame, GetDamageToDeal(), 5.f, target );
	newBasicAttack->SetCurrentPosition( m_currentPosition );
	m_currentMap->AddEntityToList( newBasicAttack );

	float attackCooldownSeconds = 1.f / GetAttackSpeed();
	m_attackTimer.SetSeconds( m_theGame->GetGameClock(), attackCooldownSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Player::SetAudioPlayback()
{
	switch( m_actorState )
	{
	case ACTOR_STATE_WALK:
	case ACTOR_STATE_ATTACK_MOVE:
		g_theAudio->SoundIsPaused( m_walkingPlayback, false );
		break;
	case ACTOR_STATE_IDLE:
	case ACTOR_STATE_ATTACK:
	default:
		g_theAudio->SoundIsPaused( m_walkingPlayback, true );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Player::IsWalkSoundPaused( bool isPaused )
{
	g_theAudio->SoundIsPaused( m_walkingPlayback, isPaused );
}

