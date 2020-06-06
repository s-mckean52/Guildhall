#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

class Ability;

class PlayerEntity : public Entity
{
public:
	PlayerEntity( Game* theGame );
	~PlayerEntity();

	void Update( float deltaSeconds ) override;
	void SetCurrentPosition( Vec2 const& position ) override;
	void Render() const override;

	void RenderAbilities() const;

	void MoveTowardsPosition( float deltaSeconds );

	void SetMovePosition( Vec2 const& positionToMoveTo );

private:
	Ability* m_abilities[ 4 ] = {};

	bool m_isMoving = false;
	Vec2 m_positionToMoveTo;
};