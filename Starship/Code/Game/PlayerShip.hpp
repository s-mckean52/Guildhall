#pragma once
#include "Game\Entity.hpp"

class Game;

class PlayerShip : public Entity
{
private:
	int m_rotateDirection = 0;
	float m_thrustLengthPercent = 0.f;
	float m_thrustAlphaPercent = 1.f;

	bool m_turningLeft = false;
	bool m_turningRight = false;

	bool m_isAccelerating = false;

	bool m_isShooting = false;

	int m_controllerID = 0;
	float m_thrustFraction = 0.f;

	int m_lives = MAX_PLAYER_LIVES;

public:
	~PlayerShip() {};

	PlayerShip( Game* game, Vec2 pos );

	void Shoot();
	void IsTurningLeft( bool isTurningLeft );
	void IsTurningRight( bool isTurningRight );
	void IsAccelerating( bool isAccelerating );
	void SetThrustFraction( float newThrustFraction );
	void IsShooting( bool isShooting );
	void SetControllerID( int controllerID );
	const Vec2 GetModifiedAcceleration() const;
	void UpdateOrientationIfTurning( float deltaSeconds );
	void BounceOffWalls();
	void Respawn();

	int GetRotateDirection() const	{ return m_rotateDirection; }
	int GetLivesRemaining() const	{ return m_lives; }

	void UpdateFromJoystick( float deltaSeconds );
	void UpdateOnKeyPressed( float deltaSeconds );
	void UpdateOnKeyReleased( float deltaSeconds );
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
};