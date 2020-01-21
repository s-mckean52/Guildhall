#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Game.hpp"

class PlayerEntity : public Entity
{
public:
	PlayerEntity( Game* theGame, Map* theMap, const Vec2& position, int controllerID );
	~PlayerEntity() {};

	virtual void	Update( float deltaSeconds ) override;

	virtual void	Render() const override;
	void			RenderBase() const;
	void			RenderTurret() const;

	int				GetRemainingLives() const { return m_remainingLives; }

	virtual void	TakeDamage( int damageDealt ) override;
	virtual void	Die() override;
	void			Respawn();

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromJoystick( float deltaSeconds );
	void Shoot();

private:
	Game*	m_theGame = nullptr; 
	Vec2	m_startPosition;
	int		m_controllerID = -1;
	float	m_thrustFraction = 0.f;
	float	m_turretOrientationOffsetDegrees = 0.f;
	float	m_turretAngularVelocity = 180.f;

	int		m_remainingLives = 0;

	bool	m_hasShot = false;
};