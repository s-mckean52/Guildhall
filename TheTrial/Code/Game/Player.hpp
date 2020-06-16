#pragma once
#include "Game/Actor.hpp"
#include "Game/Ability.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Timer.hpp"
#include <string>

class Ability;
class Enemy;

class Player : public Actor
{
public:
	Player( Game* theGame );
	~Player();

	void Update( float deltaSeconds )				override;
	void Render() const								override;
	void SetCurrentPosition( Vec2 const& position )	override;

	void RenderAbilities( Vec2 const& abilityMinStartPos, float distanceBetweenAbilities ) const;

	void AssignAbilityToSlot( std::string abilityName, int slotNumber );
	void MoveTowardsPosition( float deltaSeconds );
	void SetMovePosition( Vec2 const& positionToMoveTo );
	void SetIsMoving( bool isMoving );
	void AttackEnemy( Enemy* enemyToAttack );
	void BasicAttack( Enemy* target );

private:
	char ABILITY_0_KEY = 'Q';
	char ABILITY_1_KEY = 'W';
	char ABILITY_2_KEY = 'E';
	char ABILITY_3_KEY = 'R';

	Ability* m_abilities[ 4 ] = {};

	Timer m_attackTimer;

	bool m_isMoving = false;
	Enemy* m_enemyTarget = nullptr;
	Vec2 m_positionToMoveTo;
};