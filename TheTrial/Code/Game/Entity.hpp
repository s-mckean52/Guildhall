#pragma once
#include "Engine/Math/Vec2.hpp"

class Game;

class Entity
{
public:
	Entity( Game* theGame );
	virtual ~Entity();

	//Virtual Methods
	virtual void Update( float deltaSeconds )		= 0;
	virtual void Render() const						= 0;

	virtual void SetCurrentPosition( Vec2 const& position );
	
	//Accessors
	Vec2	GetCurrentPosition() const		{ return m_currentPosition; }
	float	GetPhysicsRadius() const		{ return m_physicsRadius; }

protected:
	//Misc. - For now
	Game* m_theGame = nullptr;
	Vec2 m_currentPosition;
	float m_physicsRadius = 0.5f;
};