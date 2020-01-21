#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB2.hpp"

class Game;

class PlayerEntity : public Entity
{
private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromJoystick( float deltaSeconds );
	void Respawn();

public:
	PlayerEntity( Game* theGame, Vec2 position, int controllerID );
	~PlayerEntity() {};

	virtual void	Update( float deltaSeconds ) override;

	virtual void	Render() const override;
	void			RenderBase() const;
	void			RenderTurret() const;

	virtual void	Die() override;

private:
	AABB2 m_playerBox;

	int m_controllerID = -1;
	float m_thrustFraction = 0.f;
	float m_turretOrientationDegrees = 0.f;
	float m_turretAngularVelocity = 180.f;

	int m_lives = MAX_PLAYER_LIVES;
};