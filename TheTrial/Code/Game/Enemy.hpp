#pragma once
#include "Game/Actor.hpp"
#include "Game/RaycastResult.hpp"

class World;

class Enemy : public Actor
{
public:
	Enemy( Game* theGame, Vec2 const& position );
	virtual ~Enemy();

	void Update( float deltaSeconds )				override;
	void Render() const								override;
	void DebugRender() const						override;
	void TakeDamage( int damageToTake )				override;

	void DropRandomItem();
	void AttackActor( Actor* actor );

public:
	World* m_theWorld = nullptr;

private:
	RaycastResult lineOfSight;
	float m_baseItemDropChance	= 0.1f;
	int m_scoreValue			= 25;
};