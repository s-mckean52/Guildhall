#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Actor::Actor( Game* theGame, Map* theMap, const Vec2& position, std::string actorTypeName )
	: Entity( theMap, position, FACTION_GOOD )
	//, m_controllerID( controllerID )
{
	ActorDefinition* currentActorDef = ActorDefinition::s_actorDefinitions[ actorTypeName ];
	if( actorTypeName == "Player" )
	{
		m_controllerID = 0;
	}

	m_theGame				= theGame;
	m_startPosition			= m_position;
	m_angularVelocity		= 360.f;
	m_orientationDegrees	= 0.f;
	m_maxHealth				= currentActorDef->GetMaxHealth();
	m_health				= currentActorDef->GetStartHealth();
	m_physicsRadius			= currentActorDef->GetPhysicsRadius();
	m_acceleration			= Vec2( currentActorDef->GetSpeed() , 0.f );
	m_spriteBox				= currentActorDef->GetSpriteBounds();
	m_faction				= SetFactionBasedOnXml( currentActorDef->GetFaction() );
	m_defaultSpriteIndex	= currentActorDef->GetDefaultSpriteIndex();
	//m_cosmeticRadius;

	//float spriteBoxHeight = 2 * m_cosmeticRadius * g_actorSpriteSheet->GetSpriteDefinition( 3 ).GetAspect();
	//m_spriteBox = AABB2( -m_cosmeticRadius, -m_physicsRadius, m_cosmeticRadius, spriteBoxHeight + m_physicsRadius );
}


//---------------------------------------------------------------------------------------------------------
void Actor::UpdateFromKeyboard( float deltaSeconds )
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
void Actor::UpdateFromJoystick( float deltaSeconds )
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
}


//---------------------------------------------------------------------------------------------------------
void Actor::Respawn()
{
	m_health = 3;
	m_isDead = false;
}


//---------------------------------------------------------------------------------------------------------
EntityFaction Actor::SetFactionBasedOnXml( std::string factionAsString )
{
	if( factionAsString == "good" )
	{
		return FACTION_GOOD;
	}
	else if( factionAsString == "evil" )
	{
		return FACTION_EVIL;
	}
	return FACTION_NEUTRAL;
}


//---------------------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	UpdateFromJoystick( deltaSeconds );
	//UpdateFromKeyboard( deltaSeconds );

	if( m_isDead ) return;

	Vec2 currentAcceleration = m_acceleration.GetClamped( m_acceleration.GetLength() * m_thrustFraction );
	currentAcceleration.RotateDegrees( m_orientationDegrees );

	m_velocity += currentAcceleration * deltaSeconds;

	float speed = m_velocity.GetLength();
	//float speedLoss = PLAYER_FRICTION_PER_SECOND * deltaSeconds;

	//m_thrustFraction here makes friction not work but satisfy project requirement
	speed = GetClamp( speed /*- speedLoss*/, 0.f, 1.f * m_thrustFraction );
	m_velocity.SetLength( speed );

	m_position += m_velocity * deltaSeconds;

	m_thrustFraction = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void Actor::Render() const
{
	if( !m_isDead )
	{
		RenderBase();
	}

	if( !g_isDebugDraw ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Actor::RenderBase() const
{
	std::vector<Vertex_PCU> tankBaseVerts;

	Vec2 uvAtMins;
	Vec2 uvAtMaxes;

	g_actorSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, m_defaultSpriteIndex );
	AppendVertsForAABB2D( tankBaseVerts, m_spriteBox, m_color, uvAtMins, uvAtMaxes );

	TransformVertexArray( static_cast<int>(tankBaseVerts.size()), &tankBaseVerts[0], 1.f, 0.f, m_position );

	g_theRenderer->BindTexture( &g_actorSpriteSheet->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( tankBaseVerts );
}


//---------------------------------------------------------------------------------------------------------
void Actor::TakeDamage( int damageDealt )
{
	SoundID playerDamagedSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	g_theAudio->PlaySound( playerDamagedSound );

	Entity::TakeDamage( damageDealt );
}


//---------------------------------------------------------------------------------------------------------
void Actor::Die()
{
	SoundID playerDeathSound = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_theAudio->PlaySound( playerDeathSound );

	m_isDead = true;
}