#include "Game/PlayerController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
PlayerController::PlayerController( Game* theGame, Vec2 position, int controllerID )
	: Entity( theGame, position )
{
	m_acceleration = Vec2( PLAYER_ACCELERATION, 0.f );
	m_angularVelocity = 180.f;
	m_playerBox = AABB2( Vec2(-0.5f, -0.5f), Vec2( 0.5f, 0.5f ) );
	m_physicsRadius = PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_COSMETIC_RADIUS;
	m_controllerID = controllerID;
	m_orientationDegrees = 90.f;
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::UpdateFromKeyboard( float deltaSeconds )
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
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::UpdateFromJoystick( float deltaSeconds )
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
	else
	{
		m_thrustFraction = 0.f;
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::Respawn()
{
	
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::Update( float deltaSeconds )
{
	UpdateFromJoystick( deltaSeconds );
	//UpdateFromKeyboard( deltaSeconds );

	Vec2 currentAcceleration = m_acceleration.GetClamped( m_acceleration.GetLength() * m_thrustFraction );
	currentAcceleration.RotateDegrees( m_orientationDegrees );

	m_velocity += currentAcceleration * deltaSeconds;

	float speed = m_velocity.GetLength();
	float speedLoss = PLAYER_FRICTION_PER_SECOND * deltaSeconds;
	speed  = GetClampf( speed - speedLoss, 0.f, PLAYER_MAX_SPEED );
	m_velocity.SetLength( speed );

	m_position += m_velocity * deltaSeconds;

	m_thrustFraction = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::Render() const
{
	if( m_isDead ) return;

	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindTexture( tankBaseTexture );
	std::vector<Vertex_PCU> tankBaseVerts;
	g_theRenderer->AppendVertsForAABB2D( tankBaseVerts, m_playerBox, RGBA8_WHITE );

	TransformVertexArray( static_cast<int>( tankBaseVerts.size() ), &tankBaseVerts[ 0 ], 1.f, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertextArray( tankBaseVerts );

	if( m_game->GetGameState() != DEBUG_STATE ) return;
	
	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void PlayerController::Die()
{
	m_isDead = true;
}