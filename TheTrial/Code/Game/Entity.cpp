#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame )
{
	m_theGame = theGame;
}


//---------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	DrawCircleAtPoint( m_currentPosition, m_physicsRadius, PHYSICS_RADIUS_COLOR, DEBUG_THICKNESS );
	DrawAABB2AtPoint( m_currentPosition, m_renderBounds, RENDER_BOUNDS_COLOR, DEBUG_THICKNESS );
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetCurrentPosition( Vec2 const& position )
{
	m_currentPosition = position;
}