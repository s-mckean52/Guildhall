#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

class Game;
class SpriteSheet;

class Entity
{
public:
	Entity( Game* theGame );
	virtual ~Entity();

	//Virtual Methods
	virtual void Update( float deltaSeconds )		= 0;
	virtual void Render() const						= 0;
	virtual void DebugRender() const;

	virtual void SetCurrentPosition( Vec2 const& position );
	
	//Accessors
	Vec2	GetCurrentPosition() const		{ return m_currentPosition; }
	float	GetPhysicsRadius() const		{ return m_physicsRadius; }

protected:
	//Misc. - For now
	Vec2			m_currentPosition;
	Game*			m_theGame			= nullptr;
	float			m_physicsRadius		= 0.5f;
	AABB2			m_renderBounds		= AABB2( -0.5f, -0.5f, 0.5f, 0.5f );
	int				m_spriteIndex		= 0;
	SpriteSheet*	m_spriteSheet		= nullptr;

};