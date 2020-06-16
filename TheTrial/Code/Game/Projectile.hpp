#pragma once
#include "Game/Entity.hpp"

class Actor;

class Projectile : public Entity
{
public:
	Projectile( Game* theGame, int damage, float movementSpeed, Actor* target );
	virtual ~Projectile();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

private:
	bool	m_hasHitTarget		= false;
	float	m_movementSpeed		= 0.f;
	int		m_damage			= 0;
	Actor*	m_targetEntity		= nullptr;
};