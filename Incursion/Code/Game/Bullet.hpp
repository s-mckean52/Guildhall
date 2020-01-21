#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

class Game;

class Bullet : public Entity
{
public:
	~Bullet() {};
	explicit Bullet( Map *theMap, Vec2 position, EntityFaction faction );

	virtual void Render() const override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Die() override;

	int GetDamageToDeal() const { return m_damageToDeal; }

private:
	int m_damageToDeal = 1;
};