#pragma once
#include "Game/Entity.hpp"

class Map;

class NpcTurret : public Entity
{
public:
	~NpcTurret() {};
	explicit NpcTurret( Map* theMap, const Vec2& position, EntityFaction faction );

	virtual void TakeDamage( int damageDealt ) override;
	virtual void Die() override;
	virtual void Update( float deltaSeconds ) override;

	virtual void	Render() const override;
	void			RenderBase() const;
	void			RenderTurret() const;

	void Shoot();
	void TurnTowardsEntityIfWithinRange( Entity* entity, float deltaSeconds );

	bool IsEntityVisible( Entity* entity );
	void ShootAtEntity( Entity* entityToShootAt, float deltaSeconds );
	bool IsEntityInShootingAperature( Entity* entityToShootAt );

private:
	float	m_viewRange = 1.f;
	float	m_shootCooldown = 0.f;
	float	m_shootAperatureDegrees = 360.f;
	int		m_swivelDirection = -1;

	Vec2 m_bulletTrajectoryEndPoint;
	Vec2 m_fwdDir;
	Vec2 m_enemyLastKnownLocation = Vec2( -1.f, -1.f );

};