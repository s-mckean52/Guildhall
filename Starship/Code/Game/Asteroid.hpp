#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

class Game;

class Asteroid : public Entity
{
private:
	Vec2 m_asteroidVertecies[ NUM_ASTEROID_VERTICIES ];

public:
	~Asteroid() {};
	Asteroid(Game *theGame);

	void CreateVertecies();

	virtual void Render() const override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Die() override;
};
