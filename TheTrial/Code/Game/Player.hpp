#pragma once
#include "Game/Actor.hpp"
#include "Game/Ability.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Timer.hpp"
#include <string>

class Map;
class Enemy;
class Ability;
class SpriteAnimDefinition;

enum PlayerState
{
	PLAYER_STATE_IDLE,
	PLAYER_STATE_WALK,
	PLAYER_STATE_ATTACK,
};

class Player : public Actor
{
public:
	Player( Game* theGame );
	~Player();

	void Update( float deltaSeconds )				override;
	void Render() const								override;
	void DebugRender() const						override;
	void SetCurrentPosition( Vec2 const& position )	override;


	void UpdateAnimSpriteBasedOnMovementDirection();
	void CreateSpriteAnimFromPath( char const* filepath );
	void DetermineDirection( float& directionValue, std::string& currentDirection, std::string const& newDirection, Vec2 const& directionVector );
	void RenderAbilities( Vec2 const& abilityMinStartPos, float distanceBetweenAbilities ) const;

	SpriteAnimDefinition* GetSpriteAnimByPath( std::string const& animName );

	void AssignAbilityToSlot( std::string abilityName, int slotNumber );
	void MoveTowardsPosition( float deltaSeconds );
	void SetMovePosition( Vec2 const& positionToMoveTo );
	void SetCurrentMap( Map* map );
	void SetIsMoving( bool isMoving );
	void AttackEnemy( Enemy* enemyToAttack );
	void BasicAttack( Enemy* target );

public:
	static char const* GetPlayerStateAsString( PlayerState playerState );

private:
	char ABILITY_0_KEY = 'Q';
	char ABILITY_1_KEY = 'W';
	char ABILITY_2_KEY = 'E';
	char ABILITY_3_KEY = 'R';
	Ability* m_abilities[ 4 ] = {};

	Timer m_attackTimer;
	
	SpriteAnimDefinition* m_anim = nullptr;
	std::vector<SpriteSheet*> m_spriteSheets;
	std::map<std::string, SpriteAnimDefinition*> m_spriteAnimsBySheetName;

	Map* m_currentMap = nullptr;

	Vec2		m_positionToMoveTo;
	PlayerState m_playerState		= PLAYER_STATE_IDLE;
	Enemy*		m_enemyTarget		= nullptr;
};