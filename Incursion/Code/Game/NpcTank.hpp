#pragma once
#include "Game/Entity.hpp"
#include "Game/RaycastResult.hpp"

class Map;

class NpcTank : public Entity
{
public:
	~NpcTank() {}
	explicit NpcTank( Map* theMap, const Vec2& position, EntityFaction faction );

	virtual void TakeDamage( int damageDealt ) override;
	virtual void Die() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	void AvoidWalls();
	void Shoot();
	void TurnTowardsEntityIfWithinRange( Entity* entityToTurnTowards, float deltaSeconds );
	void ShootAtEntity( Entity* entityToShootAt, float deltaSeconds );
	void SetThrustFraction( Entity* entityToMoveTowards );
	
	bool IsEntityVisible( Entity* entity );
	bool IsEntityInShootingAperature( Entity* entityToShootAt );
	bool IsEntityInDriveAperature( Entity* entityToShootAt );

private:
	float m_thrustFraction			= 1.f;
	float m_viewRange				= 1.f;
	float m_driveAperatureDegrees	= 360.f;
	float m_shootAperatureDegrees	= 360.f;
	float m_goalOrientationDegrees	= 0.f;

	float m_shootCooldown			= 0.f;
	float m_newOrientationCooldown	= 0.f;
	Vec2 m_fwdDir;
	Vec2 m_enemyLastKnownLocation = Vec2( -1.f, -1.f );

	Vec2 m_goalCenterRaycastStart;
	Vec2 m_goalLeftRaycastStart;
	Vec2 m_goalRightRaycastStart;

	RaycastResult m_goalCenterRaycast;
	RaycastResult m_goalLeftRaycast;
	RaycastResult m_goalRightRaycast;
};