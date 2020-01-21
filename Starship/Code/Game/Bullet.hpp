#pragma once
#include "Game/Entity.hpp"

class Bullet : public Entity
{
public:
	~Bullet() {};
	Bullet( Game *theGame, Vec2 location, float orientation );

	virtual void Render() const override;
	virtual void Update( float deltaSeconds ) override;
};
