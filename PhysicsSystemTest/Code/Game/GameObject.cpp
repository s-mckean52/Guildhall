#include "Game/GameObject.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Vec2.hpp"


//---------------------------------------------------------------------------------------------------------
GameObject::GameObject()
{
	m_startBorderColor = Rgba8::BLUE;
	m_startFillColor = Rgba8::WHITE;
	m_startFillColor.a = 127;

	m_currentBorderColor = m_startBorderColor;
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
}


//---------------------------------------------------------------------------------------------------------
void GameObject::UpdateColors()
{
	if( m_isHovered )
	{
		m_currentBorderColor = Rgba8::YELLOW;
	}
	else if( !m_isHeld )
	{
		m_currentBorderColor = m_startBorderColor;
	}

	if( m_isHeld )
	{
		m_currentBorderColor = Rgba8( 0, 100, 0 );
	}
	else if( !m_isHovered )
	{
		m_currentBorderColor = m_startBorderColor;
	}

	if( m_isOverlapping )
	{
		m_currentFillColor = Rgba8( 255, 0, 0, 127 );
	}
	else
	{
		m_currentFillColor = m_startFillColor;
	}
}


//---------------------------------------------------------------------------------------------------------
void GameObject::Draw() const
{
	if( m_isDestroyed ) return;

	m_rigidbody->m_collider->DebugRender( g_theRenderer, m_currentBorderColor, m_currentFillColor );
}


//---------------------------------------------------------------------------------------------------------
void GameObject::SetPosition( Vec2 position )
{
	m_rigidbody->SetPosition( position );
}

