#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/GameObject.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
	m_defaultBorderColor = Rgba8::BLUE;
	m_startFillColor = Rgba8::WHITE;
	m_startFillColor.a = 127;

	m_currentBorderColor = m_defaultBorderColor;
	m_currentFillColor = m_startFillColor;
}


//---------------------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	m_rigidbody->Destroy();
	m_rigidbody = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void GameObject::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	UpdateColors();

	m_rigidbody->SetEnabled( !m_isHeld );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::UpdateColors()
{
	UpdateColorBasedOnPhysics();

	if( m_isHovered )
	{
		m_currentBorderColor = Rgba8::YELLOW;
	}
	else if( !m_isHeld )
	{
		m_currentBorderColor = m_defaultBorderColor;
	}

	if( m_isHeld )
	{
		m_currentBorderColor = Rgba8( 0, 100, 0 );
	}
	else if( !m_isHovered )
	{
		m_currentBorderColor = m_defaultBorderColor;
	}
}


//---------------------------------------------------------------------------------------------------------
void GameObject::UpdateColorBasedOnPhysics()
{
	switch( m_rigidbody->m_simulationMode )
	{
	case SIMULATION_MODE_KINEMATIC:
		m_defaultBorderColor = Rgba8::MAGENTA;
		break;
	case SIMULATION_MODE_DYNAMIC:
		m_defaultBorderColor = Rgba8::BLUE;
		break;
	case SIMULATION_MODE_STATIC:
		m_defaultBorderColor = Rgba8::GRAY;
		break;
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void GameObject::Draw() const
{
	if( m_isDestroyed ) return;

	m_rigidbody->m_collider->DebugRender( g_theRenderer, m_currentBorderColor, m_currentFillColor );
	m_rigidbody->DebugRender( g_theRenderer );
	DrawTooltip();
}


//---------------------------------------------------------------------------------------------------------
void GameObject::DrawTooltip() const
{
	if( !m_isHovered ) return;

	std::string simulationModeAsString = GetSimulationModeAsString();
	std::string velocityAsString = GetVelocityAsString();
	std::string verletVelocityAsString = GetVerletVelocityAsString();

	std::string simulationModeString	= Stringf( "Simulation Mode: %s", simulationModeAsString.c_str() );
	std::string massString				= Stringf( "Mass: %f", m_rigidbody->m_mass );
	std::string velocityString			= Stringf( "Velocity: %s", velocityAsString.c_str() );
	std::string bouncineessString		= Stringf( "Bounciness: %f", GetCollider()->GetPhysicsMaterialBounciness() );
	std::string frictionString			= Stringf( "Friction: %f", GetCollider()->GetPhysicsMaterialFriction() );
	std::string dragString				= Stringf( "Drag: %f", m_rigidbody->GetDrag() );
	std::string verletString			= Stringf( "Verlet Velocity: %s", verletVelocityAsString.c_str() );

	AABB2 tooltipBox;
	std::vector<Vertex_PCU> tooltipBoxVerts;
	
	Collider2D* collider = GetCollider();
	AABB2 worldBounds = collider->GetWorldBounds();
	tooltipBox.mins = worldBounds.maxes;
	tooltipBox.maxes = tooltipBox.mins + Vec2( 0.f, 120.f ) + g_testFont->GetDimensionsForText2D( 10.f, verletString );
	
	AppendVertsForAABB2D( tooltipBoxVerts, tooltipBox, Rgba8::GRAY );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( tooltipBoxVerts );


	std::vector<Vertex_PCU> tooltipVerts;

	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, simulationModeString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, massString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.15f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, velocityString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.30f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, bouncineessString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.45f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, frictionString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.60f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, dragString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.75f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 10.f, verletString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.90f ) );

	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( tooltipVerts );
}


//---------------------------------------------------------------------------------------------------------
std::string GameObject::GetSimulationModeAsString() const
{
	switch( m_rigidbody->m_simulationMode )
	{
	case SIMULATION_MODE_DYNAMIC:
		return "Dynamic";
	case SIMULATION_MODE_KINEMATIC:
		return "Kinematic";
	case SIMULATION_MODE_STATIC:
		return "Static";
	default:
		return "";
	}
}


//---------------------------------------------------------------------------------------------------------
std::string GameObject::GetVelocityAsString() const
{
	Vec2 velocity = m_rigidbody->GetVelocity();
	return Stringf( "x: %f y: %f", velocity.x, velocity.y );
}


//---------------------------------------------------------------------------------------------------------
std::string GameObject::GetVerletVelocityAsString() const
{
	Vec2 verletVelocity = m_rigidbody->GetVerletVelocity();
	return Stringf( "x: %f y: %f", verletVelocity.x, verletVelocity.y );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::SetPosition( Vec2 position )
{
	m_rigidbody->SetPosition( position );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::AddBounciness( float bounce )
{
	PhysicsMaterial* physicsMat = m_rigidbody->m_collider->m_physicsMaterial;
	physicsMat->AddBounciness( bounce );

	m_currentFillColor.a = static_cast<unsigned char>( 255.f * physicsMat->GetBounciness() );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::AddMass( float massToAdd )
{
	m_rigidbody->m_mass += massToAdd;
	if( m_rigidbody->m_mass <= 0.0f )
	{
		m_rigidbody->m_mass = 0.001f;
	}
}


//---------------------------------------------------------------------------------------------------------
void GameObject::AddDrag( float dragToAdd )
{
	m_rigidbody->m_drag += dragToAdd;
	if( m_rigidbody->m_drag < 0.0f )
	{
		m_rigidbody->m_drag = 0.0f;
	}
}


//---------------------------------------------------------------------------------------------------------
void GameObject::AddFriction( float frictionToAdd )
{
	PhysicsMaterial* physicsMat = m_rigidbody->m_collider->m_physicsMaterial;
	physicsMat->AddFriction( frictionToAdd );
}


//---------------------------------------------------------------------------------------------------------
Collider2D* GameObject::GetCollider() const
{
	return m_rigidbody->m_collider;
}
