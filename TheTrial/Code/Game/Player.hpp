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

class Player : public Actor
{
public:
	Player( Game* theGame, std::string const& characterType );
	~Player();

	void Update( float deltaSeconds )						override;
	void Render() const										override;
	void DebugRender() const								override;
	void SetCurrentPosition( Vec2 const& position )			override;
	void SetMovePosition( Vec2 const& positionToMoveTo )	override;

	void RenderAbilities( Vec2 const& abilityMinStartPos, float distanceBetweenAbilities ) const;

	void AssignAbilityToSlot( std::string abilityName, int slotNumber );
	void SetAttackMovePosition( Vec2 const& positionToMoveTo );
	void SetCurrentMap( Map* map );
	void AttackEnemy( Enemy* enemyToAttack );
	void BasicAttack( Enemy* target );
	void SetAudioPlayback();
	void IsWalkSoundPaused( bool isPaused );

private:
	Ability*	m_abilities[ 4 ]		= {};
	char		m_abilityKeys[ 4 ]		= { 'Q', 'W', 'E', 'R' };
	std::string m_characterType			= "char_one";

	Map*	m_currentMap	= nullptr;
	Enemy*	m_enemyTarget	= nullptr;
	
	SoundPlaybackID m_walkingPlayback;
};