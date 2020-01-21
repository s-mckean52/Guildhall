#pragma once
#include "Game/Entity.hpp"

struct Vec2;
class Game;

class Wasp : public Entity
{
public:
	~Wasp() {};
	Wasp( Game* theGame, Vec2 position );

	void FaceNearestPlayer();

	virtual void Render() const override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Die() override;

private:
	float m_thrustLengthAddition = 0.f;
};
