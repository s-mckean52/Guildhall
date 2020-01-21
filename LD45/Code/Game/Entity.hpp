#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"

class Game;

class Entity
{
protected:
	Game	*m_game	= nullptr;

	Vec2	m_position;
	Vec2	m_velocity;
	Vec2	m_acceleration;
	Rgba8	m_color;

	int		m_health				= 1;
	bool	m_isDead				= false;
	bool	m_isGarbage				= false;
	float	m_orientationDegrees	= 0.f;
	float	m_angularVelocity		= 0.f;
	float	m_physicsRadius			= 10.f;		//Noticeably Big
	float	m_cosmeticRadius		= 20.f;		//Noticeably Big
	float	m_scale					= 1.f;
	int		m_numberOfDebris		= 3;

public:
	Entity() {};
	~Entity() {};

	Entity( Game* theGame, const Vec2& startPosition);

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

	const Vec2	GetForwardVector()	const { return m_velocity; }
	float		GetPhysicsRadius()	const { return m_physicsRadius; }
	float		GetCosmeticRadius()	const { return m_cosmeticRadius; }
	const Vec2	GetPosition()		const { return m_position; }
	const Rgba8	GetColor()			const { return m_color; }
	int			GetNumberOfDebris()	const { return m_numberOfDebris; }

	void		SetPosition( Vec2 newPosition );

	void		TakeDamage( int damageDealt );
	void		DebugDraw() const;

	bool		IsOffScreen() const;
	bool		IsDead()	const { return m_isDead; }
	bool		IsGarbage() const { return m_isGarbage; }

};