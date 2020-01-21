#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

class Game;
struct Vec2;

class Debris : public Entity
{
public:
	~Debris() {};
	Debris( Game *theGame, Vec2 position, Vec2 velocity, Rgba8 color, float cosmeticRadius );

	virtual void Render() const override;
	virtual void Update( float deltaSeconds ) override;

	void CreateVertices();

private:
	Vec2 m_debrisVertices[ NUM_DEBRIS_VERTS ];
	float m_alphaPercent = 0.5f;
};