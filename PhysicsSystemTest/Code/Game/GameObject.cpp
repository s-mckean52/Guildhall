#include "Game/GameObject.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


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
Collider2D* GameObject::GetCollider() const
{
	return m_rigidbody->m_collider;
}
