#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/PlayerEntity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"


//---------------------------------------------------------------------------------------------------------
PlayerEntity::PlayerEntity( Game* theGame, Map* theMap, const Vec2& position, int controllerID )
	: Entity( theMap, position, FACTION_GOOD )
	, m_controllerID( controllerID )
{
	m_theGame = theGame;
	m_maxHealth							= PLAYER_HEALTH;
	m_health							= PLAYER_HEALTH;
	m_remainingLives					= EXTRA_PLAYER_LIVES;
	m_startPosition						= m_position;
	m_angularVelocity					= PLAYER_ANGULAR_VELOCITY;
	m_physicsRadius						= PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius					= PLAYER_COSMETIC_RADIUS;
	m_orientationDegrees				= PLAYER_START_ORIENTATION;
	m_turretAngularVelocity				= PLAYER_TURRET_ANGULAR_VELOCITY;
	m_entityType						= ENTITY_TYPE_PLAYER;

	m_acceleration		= Vec2( PLAYER_ACCELERATION, 0.f );
	m_spriteBox			= AABB2( Vec2( -0.45f, -0.45f ), Vec2( 0.45f, 0.45f ) );
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::UpdateFromKeyboard( float deltaSeconds )
{
	if( g_theInput->IsKeyPressed( KEY_CODE_UP_ARROW ) )
	{
		m_thrustFraction = 1.f;
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_ARROW ) )
	{
		m_orientationDegrees += m_angularVelocity * deltaSeconds;
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		m_orientationDegrees -= m_angularVelocity * deltaSeconds;
	}

	if( g_theInput->WasKeyJustPressed( 'P' ) && m_isDead && m_remainingLives > 0 )
	{
		m_theGame->SetIsRespawning( true );
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::UpdateFromJoystick( float deltaSeconds )
{
	if( m_controllerID < 0 )
		return;

	const XboxController& controller = g_theInput->GetXboxController( m_controllerID );
	if( !controller.IsConnected() )
		return;

	if( m_isDead )
	{
		const KeyButtonState& startButton = controller.GetButtonState( XBOX_BUTTON_ID_START );
		if( startButton.WasJustPressed() )
		{
			if( m_remainingLives > 0 )
			{
				m_theGame->SetIsRespawning( true );
			}
		}
		return;
	}

	const AnalogJoyStick& leftStick = controller.GetLeftJoystick();
	float leftStickMagnitude = leftStick.GetMagnitude();
	if( leftStickMagnitude > 0.f )
	{
		m_orientationDegrees = GetTurnedToward( m_orientationDegrees, leftStick.GetAngleDegrees(), m_angularVelocity * deltaSeconds );
		m_thrustFraction = leftStickMagnitude;
	}

	const AnalogJoyStick& rightStick = controller.GetRightJoystick();
	float rightStickMagnitude = rightStick.GetMagnitude();
	if( rightStickMagnitude > 0.f )
	{
		m_turretOrientationOffsetDegrees += m_orientationDegrees;
		m_turretOrientationOffsetDegrees = GetTurnedToward( m_turretOrientationOffsetDegrees, rightStick.GetAngleDegrees(), m_turretAngularVelocity * deltaSeconds );
		m_turretOrientationOffsetDegrees -= m_orientationDegrees;
	}

	if( controller.GetRightTrigger() >= TRIGGER_ACTIVATION_FRACTION && !m_hasShot )
	{
		m_hasShot = true;
		Shoot();
	}
	else if( controller.GetRightTrigger() < TRIGGER_ACTIVATION_FRACTION )
	{
		m_hasShot = false;
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Respawn()
{
	m_remainingLives--;
	m_health = PLAYER_HEALTH;
	m_isDead = false;
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Update( float deltaSeconds )
{
	m_pushedByWalls = !g_isNoClip;

	UpdateFromJoystick( deltaSeconds );
	UpdateFromKeyboard( deltaSeconds );

	if( m_isDead ) return;

	HealthRegen( deltaSeconds );

	Vec2 currentAcceleration = m_acceleration.GetClamped( m_acceleration.GetLength() * m_thrustFraction );
	currentAcceleration.RotateDegrees( m_orientationDegrees );

	m_velocity += currentAcceleration * deltaSeconds;

	float speed = m_velocity.GetLength();
	//float speedLoss = PLAYER_FRICTION_PER_SECOND * deltaSeconds;

	//m_thrustFraction here makes friction not work but satisfy project requirement
	speed = GetClamp( speed /*- speedLoss*/, 0.f, PLAYER_MAX_SPEED * m_thrustFraction * GetSlowFractionOnMud() );
	m_velocity.SetLength( speed );

	m_position += m_velocity * deltaSeconds;

	m_thrustFraction = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Render() const
{
	if( !m_isDead )
	{
		RenderBase();
		RenderTurret();
		RenderHealthBar( Rgba8::GREEN );
	}

	if( !g_isDebugDraw ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::RenderBase() const
{
	std::vector<Vertex_PCU> tankBaseVerts;

	AppendVertsForAABB2D( tankBaseVerts, m_spriteBox, m_color, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f) );

	TransformVertexArray( static_cast<int>(tankBaseVerts.size()), &tankBaseVerts[0], 1.f, m_orientationDegrees, m_position );

	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindTexture( tankBaseTexture );
	g_theRenderer->DrawVertexArray( tankBaseVerts );
}

//---------------------------------------------------------------------------------------------------------
void PlayerEntity::RenderTurret() const
{
	std::vector< Vertex_PCU > tankTurretVerts;

	AppendVertsForAABB2D( tankTurretVerts, m_spriteBox, Rgba8::WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f) );

	TransformVertexArray( static_cast<int>(tankTurretVerts.size()), &tankTurretVerts[0], 1.f, m_orientationDegrees + m_turretOrientationOffsetDegrees, m_position );

	Texture* tankTurretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->BindTexture( tankTurretTexture );
	g_theRenderer->DrawVertexArray( tankTurretVerts );
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::TakeDamage( int damageDealt )
{
	SoundID playerDamagedSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	g_theAudio->PlaySound( playerDamagedSound );

	Entity::TakeDamage( damageDealt );
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Die()
{
	SoundID playerDeathSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_theAudio->PlaySound( playerDeathSound );

	m_map->SpawnExplosion( m_position, m_cosmeticRadius + 1.f, PLAYER_EXPLOSION_DURATION_SECONDS );

	m_isDead = true;

	m_theGame->SetGameState( GAME_STATE_DEATH );
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Shoot()
{
	SoundID playerShootSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	g_theAudio->PlaySound( playerShootSound );

	Vec2 bulletSpawnPosition = m_position + Vec2::MakeFromPolarDegrees( m_orientationDegrees + m_turretOrientationOffsetDegrees, m_cosmeticRadius );
	Entity *firedBullet = m_map->SpawnEntity( ENTITY_TYPE_GOOD_BULLET, bulletSpawnPosition);
	firedBullet->SetOrientationDegrees( m_orientationDegrees + m_turretOrientationOffsetDegrees );
}
