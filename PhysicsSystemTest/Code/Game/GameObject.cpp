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

// 	if( m_isOverlapping )
// 	{
// 		m_currentFillColor = Rgba8::RED;
// 	}
// 	else if( !m_isOverlapping )
// 	{
// 		m_currentFillColor = Rgba8::WHITE;
// 	}
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
void GameObject::Draw( Vec2 const& mousePos ) const
{
	if( m_isDestroyed ) return;

	m_rigidbody->m_collider->DebugRender( g_theRenderer, m_currentBorderColor, m_currentFillColor );
	m_rigidbody->DebugRender( g_theRenderer );
	DrawTooltip( mousePos );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::DrawTooltip( Vec2 const& mousePos ) const
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
	std::string momentString			= Stringf( "Moment of Inertia: %f", m_rigidbody->GetMoment() );
	std::string rotationDegreesString	= Stringf( "Rotation Degrees: %f", m_rigidbody->GetRotationDegrees() );
	std::string angularVelocityString	= Stringf( "Angular Velocity: %f", m_rigidbody->GetAngularVelocity() );

	AABB2 tooltipBox;
	std::vector<Vertex_PCU> tooltipBoxVerts;
	
	Collider2D* collider = GetCollider();
	AABB2 worldBounds = collider->GetWorldBounds();
	tooltipBox.mins = mousePos;
	tooltipBox.maxes = tooltipBox.mins + Vec2( 0.f, 2.f ) + g_testFont->GetDimensionsForText2D( 0.1f, verletString );
	
	Rgba8 backgroundColor = Rgba8::GRAY;
	backgroundColor.a = 100;
	AppendVertsForAABB2D( tooltipBoxVerts, tooltipBox, backgroundColor );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( tooltipBoxVerts );


	std::vector<Vertex_PCU> tooltipVerts;

	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, simulationModeString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, massString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.1f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, velocityString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.2f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, bouncineessString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.3f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, frictionString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.4f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, dragString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.5f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, verletString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.6f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, momentString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.7f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, rotationDegreesString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.8f ) );
	g_testFont->AddVertsForTextInBox2D( tooltipVerts, tooltipBox, 0.1f, angularVelocityString, Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT - Vec2( 0.f, 0.9f ) );

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
	float newRBMass = m_rigidbody->m_mass + massToAdd;
	if( newRBMass <= 0.0f )
	{
		newRBMass = 0.001f;
	}
	m_rigidbody->SetMass( newRBMass );
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
void GameObject::AddRotationDegrees(float rotationDegrees)
{
	m_rigidbody->AddRotationDegrees( rotationDegrees );
}


//---------------------------------------------------------------------------------------------------------
Collider2D* GameObject::GetCollider() const
{
	return m_rigidbody->m_collider;
}
