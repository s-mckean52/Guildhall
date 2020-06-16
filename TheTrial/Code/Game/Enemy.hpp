#pragma once
#include "Game/Actor.hpp"

class Enemy : public Actor
{
public:
	Enemy( Game* theGame, Vec2 const& position );
	virtual ~Enemy();

	void Update( float deltaSeconds )				override;
	void Render() const								override;
};