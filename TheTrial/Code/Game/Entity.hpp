#pragma once
#include "Engine/Math/Vec2.hpp"

class Game;

class Entity
{
public:
	Entity( Game* theGame );
	Entity( Game* theGame, float movementSpeed, float attacksPerSecond, int attackDamage, float critChanceFraction );
	~Entity();

	//Virtual Methods
	virtual void Update( float deltaSeconds )		= 0;
	virtual void Render() const						= 0;

	virtual void SetCurrentPosition( Vec2 const& position );
	
		//Accessors
	Vec2 GetCurrentPosition() const		{ return m_currentPosition; }
	int GetCritDamage() const;

protected:
	//Misc. - For now
	Game* m_theGame = nullptr;
	Vec2 m_currentPosition;

	//Stats
	const double	m_critDamageMultiplier	= 1.5;
	float			m_movementSpeedPerSecond			= 1.f;
	float			m_attacksPerSecond		= 1.f;
	int				m_attackDamage			= 20;
	float			m_critChanceFraction	= 0.1f;
};