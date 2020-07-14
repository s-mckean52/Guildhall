#pragma once
#include "Game/Entity.hpp"

class Actor;

class Projectile : public Entity
{
public:
	Projectile( Game* theGame, int damage, float movementSpeed, Actor* target );
	Projectile( Game* theGame, int damage, float movementSpeed, Vec2 const& direction, float range );
	virtual ~Projectile();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	bool HasTarget() const;
	void DealDamageToActor( Actor* actorToDealDamageTo );

private:
	float	m_movementSpeed		= 0.f;
	int		m_damage			= 0;
	float	m_rangeRemaining				= 0.f;
	Actor*	m_targetEntity		= nullptr;
	Vec2	m_direction			= Vec2::RIGHT;
};