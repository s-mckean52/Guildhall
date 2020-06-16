#include "Game/Enemy.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"

//---------------------------------------------------------------------------------------------------------
Enemy::Enemy( Game* theGame, Vec2 const& position )
	: Actor( theGame )
{
	m_currentPosition = position;
}


//---------------------------------------------------------------------------------------------------------
Enemy::~Enemy()
{
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Render() const
{
	if( m_isDead )
		return;

	g_theRenderer->BindMaterial( nullptr );
	DrawCircleAtPoint( m_currentPosition, m_physicsRadius, Rgba8::RED, 0.1f );
	RenderHealthBar( Rgba8::RED );
}
