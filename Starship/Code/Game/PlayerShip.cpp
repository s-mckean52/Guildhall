#include "Game/PlayerShip.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/App.hpp"


//---------------------------------------------------------------------------------------------------------
PlayerShip::PlayerShip( Game* game, Vec2 pos )
	: Entity( game, pos )
{
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;
	m_acceleration = Vec2( PLAYER_SHIP_ACCELERATION, 0.f );
	m_color = RGBA8_BALI_HAI;
	m_numberOfDebris = NUMBER_PLAYER_SHIP_DEBRIS;
	m_lives--;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::Shoot()
{
	if( m_isShooting || m_isDead ) return;

	m_isShooting = true;

	Vec2 spawnLocation = TransformPosition2D( Vec2( 1.f, 0.f ), m_scale, m_orientationDegrees, m_position );
	m_game->SpawnBullet( spawnLocation, m_orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::IsTurningLeft( bool turningLeft )
{
	m_turningLeft = turningLeft;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::IsTurningRight( bool turningRight )
{
	m_turningRight = turningRight;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::IsAccelerating( bool isAccelerating )
{
	m_isAccelerating = isAccelerating;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::SetThrustFraction( float newThrustFraction )
{
	m_thrustFraction = newThrustFraction;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::IsShooting( bool isShooting )
{
	m_isShooting = isShooting;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::SetControllerID( int controllerID )
{
	m_controllerID = controllerID;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 PlayerShip::GetModifiedAcceleration() const
{
	Vec2 currentAcceleration;
	if( m_thrustFraction > 0.f )
	{
		currentAcceleration = m_acceleration.GetClamped( m_acceleration.GetLength() * m_thrustFraction );
		currentAcceleration.RotateDegrees( m_orientationDegrees );
	}

	return currentAcceleration;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::UpdateOrientationIfTurning( float deltaSeconds )
{
	if( m_turningLeft == m_turningRight ) { m_rotateDirection = 0; } //TODO: Could be enum
	else if( m_turningRight ) { m_rotateDirection = -1; }
	else { m_rotateDirection = 1; }
	m_orientationDegrees += m_angularVelocity * static_cast<float>(m_rotateDirection) * deltaSeconds;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::BounceOffWalls()
{
	if( m_position.x <= m_cosmeticRadius)
	{
		m_position.x = m_cosmeticRadius;
		m_velocity.x = -m_velocity.x;
	}
	else if( m_position.x >=  CAMERA_SIZE_X - m_cosmeticRadius )
	{
		m_position.x = CAMERA_SIZE_X - m_cosmeticRadius;
		m_velocity.x = -m_velocity.x;
	}

	if( m_position.y <= m_cosmeticRadius )
	{
		m_position.y = m_cosmeticRadius;
		m_velocity.y = -m_velocity.y;
	}
	else if( m_position.y >= CAMERA_SIZE_Y - m_cosmeticRadius )
	{
		m_position.y = CAMERA_SIZE_Y - m_cosmeticRadius;
		m_velocity.y = -m_velocity.y;
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::Respawn()
{
	if( !m_isDead || m_lives <= 0 ) return;

	m_isDead = false;
	m_position = Vec2( HALF_SCREEN_X, HALF_SCREEN_Y );
	m_velocity = Vec2( 0, 0 );
	m_orientationDegrees = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::UpdateFromJoystick( float deltaSeconds )
{
	UNUSED( deltaSeconds );

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
			Respawn();
		}
		return;
	}

	const AnalogJoyStick& leftStick = controller.GetLeftJoystick();
	float leftStickMagnitude = leftStick.GetMagnitude();
	if( leftStickMagnitude > 0.f )
	{
		m_orientationDegrees = leftStick.GetAngleDegrees();
		m_thrustFraction = leftStickMagnitude;
	}

	const KeyButtonState& aButton = controller.GetButtonState( XBOX_BUTTON_ID_A );
	if( aButton.WasJustPressed() )
	{
		Shoot();
		m_isShooting = false;
	}

	float rightTrigger = controller.GetRightTrigger();
	if( rightTrigger >= TRIGGER_ACTIVATION_FRACTION )
	{
		Shoot();
	}
	else
	{
		m_isShooting = false;
	}
}

//---------------------------------------------------------------------------------------------------------
void PlayerShip::UpdateOnKeyPressed( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		Respawn();
	}

	if( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		g_theGame->SpawnAsteroid();
	}

	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		g_theApp->m_isPaused = !g_theApp->m_isPaused;
	}

	if( g_theInput->WasKeyJustPressed( 'T' ) )
	{
		g_theApp->m_isSlowMo = true;
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
	{
		Shoot();
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_LEFT ) )
	{
		IsTurningLeft( true );
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_RIGHT ) )
	{
		IsTurningRight( true );
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_UP ) )
	{
		IsAccelerating( true );
		SetThrustFraction( 1.f );
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		if( g_theGame->GetGameState() == DEBUG_STATE )
		{
			g_theGame->SetGameState( PLAY_STATE );
		}
		else
		{
			g_theGame->SetGameState( DEBUG_STATE );
		}
	}

	if( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		g_theGame->SetGameState( RESTARTING_STATE );
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::UpdateOnKeyReleased( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( m_game->GetGameState() != ATTRACT_STATE )
	{
		if( g_theInput->WasKeyJustReleased( 'T' ) )
		{
			g_theApp->m_isSlowMo = false;
		}

		if( g_theInput->WasKeyJustReleased( KEYCODE_SPACE ) )
		{
			IsShooting( false );
		}

		if( g_theInput->WasKeyJustReleased( KEYCODE_LEFT ) )
		{
			IsTurningLeft( false );
		}

		if( g_theInput->WasKeyJustReleased( KEYCODE_RIGHT ) )
		{
			IsTurningRight( false );
		}

		if( g_theInput->WasKeyJustReleased( KEYCODE_UP ) )
		{
			IsAccelerating( false );
		}
	}
}



//---------------------------------------------------------------------------------------------------------
void PlayerShip::Update( float deltaSeconds )
{
	UpdateOnKeyPressed( deltaSeconds );
	UpdateOnKeyReleased( deltaSeconds );

	if( deltaSeconds <= 0.f ) return;

	UpdateFromJoystick( deltaSeconds );
	
	if( m_isDead ) return;

	Vec2 currentAcceleration = GetModifiedAcceleration();

	UpdateOrientationIfTurning( deltaSeconds );

	m_velocity += currentAcceleration * deltaSeconds;
	m_velocity.ClampLength( PLAYER_SHIP_MAX_SPEED );
	m_position += m_velocity * deltaSeconds;

	BounceOffWalls();
	
	m_thrustLengthPercent = g_RNG->GetRandomFloatInRange( m_thrustFraction, m_thrustFraction + ( 0.5f * m_thrustFraction ) );

	if( !m_isAccelerating )
	{
		m_thrustFraction = 0.f;
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::Render() const
{
	Rgba8 tailTipColor = RGBA8_RED;
	Rgba8 tailBaseColor( 255, 140, 0);

	if( !m_isDead )
	{
		Vertex_PCU playerShip[] =
		{
			// Nose
			Vertex_PCU( Vec2( 0.f, -1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, 1.f ), m_color),
			Vertex_PCU( Vec2( 1.f, 0.f ), m_color),

			// Body
			Vertex_PCU( Vec2( -2.f, -1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, -1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, 1.f ), m_color),

			Vertex_PCU( Vec2( -2.f, -1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, 1.f ), m_color),
			Vertex_PCU( Vec2( -2.f, 1.f ), m_color),

			// Left Wing
			Vertex_PCU( Vec2( -2.f, 1.f ), m_color),
			Vertex_PCU( Vec2( 2.f, 1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, 2.f ), m_color),

			//Right Wing
			Vertex_PCU( Vec2( -2.f, -1.f ), m_color),
			Vertex_PCU( Vec2( 0.f, -2.f ), m_color),
			Vertex_PCU( Vec2( 2.f, -1.f ), m_color),

			//Thrust
			Vertex_PCU( Vec2( -2.f, .75f ), tailBaseColor ),
			Vertex_PCU( Vec2( -2 + ( -2.f * m_thrustLengthPercent ), 0.f ), tailTipColor ),
			Vertex_PCU( Vec2( -2.f, -.75f ), tailBaseColor ),
		};

		constexpr int SHIP_VERT_SIZE = sizeof( playerShip ) / sizeof( playerShip[0] );

		TransformVertexArray( SHIP_VERT_SIZE, playerShip, m_scale, m_orientationDegrees, m_position );

		g_theRenderer->DrawVertexArray( SHIP_VERT_SIZE, playerShip );
	}

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void PlayerShip::Die()
{
	m_game->AddScreenShakeIntensity( PLAYER_DEATH_SCREEN_SHAKE_INTENSITY );
	m_lives--;
	m_isDead = true;
}