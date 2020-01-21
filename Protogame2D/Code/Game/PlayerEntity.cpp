#include "Game/PlayerEntity.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//---------------------------------------------------------------------------------------------------------
PlayerEntity::PlayerEntity( Game* theGame, Vec2 position, int controllerID )
	: Entity( theGame, position )
{
	m_acceleration		= Vec2( PLAYER_ACCELERATION, 0.f );
	m_angularVelocity	= PLAYER_ANGULAR_VELOCITY;
	m_playerBox			= AABB2( Vec2( -0.45f, -0.45f ), Vec2( 0.45f, 0.45f ) );
	m_physicsRadius		= PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius	= PLAYER_COSMETIC_RADIUS;
	m_controllerID		= controllerID;
	m_orientationDegrees = 90.f;
	m_turretOrientationDegrees = m_orientationDegrees;
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
			Respawn();
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
		m_turretOrientationDegrees = GetTurnedToward( m_turretOrientationDegrees, rightStick.GetAngleDegrees(), m_turretAngularVelocity * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Respawn()
{

}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Update( float deltaSeconds )
{
	UpdateFromJoystick( deltaSeconds );
	UpdateFromKeyboard( deltaSeconds );

	Vec2 currentAcceleration = m_acceleration.GetClamped( m_acceleration.GetLength() * m_thrustFraction );
	currentAcceleration.RotateDegrees( m_orientationDegrees );

	m_velocity += currentAcceleration * deltaSeconds;

	float speed = m_velocity.GetLength();
	float speedLoss = PLAYER_FRICTION_PER_SECOND * deltaSeconds;

	//m_thrustFraction here makes friction not work but satisfy project requirement
	speed  = GetClamp( speed - speedLoss, 0.f, PLAYER_MAX_SPEED * m_thrustFraction );
	m_velocity.SetLength( speed );

	m_position += m_velocity * deltaSeconds;

	m_thrustFraction = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Render() const
{
	if( m_isDead ) return;

	RenderBase();
	RenderTurret();

	if( !g_isDebugDraw ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::RenderBase() const
{
	std::vector<Vertex_PCU> tankBaseVerts;

	AppendVertsForAABB2D( tankBaseVerts, m_playerBox, RGBA8_WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>(tankBaseVerts.size()), &tankBaseVerts[0], 1.f, m_orientationDegrees, m_position );

	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindTexture( tankBaseTexture );
	g_theRenderer->DrawVertexArray( tankBaseVerts );
}

//---------------------------------------------------------------------------------------------------------
void PlayerEntity::RenderTurret() const
{
	std::vector< Vertex_PCU > tankTurretVerts;

	AppendVertsForAABB2D( tankTurretVerts, m_playerBox, RGBA8_WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>(tankTurretVerts.size()), &tankTurretVerts[0], 1.f, m_turretOrientationDegrees, m_position );

	Texture* tankTurretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->BindTexture( tankTurretTexture );
	g_theRenderer->DrawVertexArray( tankTurretVerts );
}


//---------------------------------------------------------------------------------------------------------
void PlayerEntity::Die()
{
	m_isDead = true;
}