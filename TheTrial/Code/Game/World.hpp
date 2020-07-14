#pragma once
#include "Engine/Core/Timer.hpp"
#include <vector>

class Map;
class Game;
class Enemy;
class Cursor;
class Player;
struct Vec2;

class World
{
public:
	World( Game* theGame );
	~World();

	void CleanUpDeadEntities();

	Map*	GetCurrentMap()			{ return m_currentMap; }
	int		GetDifficultyLevel()	{ return m_difficultyLevel; }
	float	GetItemDropChance( float entityBaseDropChance );

	void	SetDifficultyLevel( int difficultyLevel );

	void Render() const;
	void RenderProgressBar() const;
	void DebugRender() const;

	void Update( float deltaSeconds );
	void UpdateNewLevelProgress();
	void AddValueToCurrentScore( int value );
	void UnlockCurrentMapExit();
	void LoadNextMap();


	void AddPlayerToCurrentMap( Player* player );
	Enemy* GetDiscOverlapEnemyOnCurrentMap( Vec2 const& discCenterPosition, float discRadius );

private:
	Game* m_theGame		= nullptr;

	int m_difficultyLevel = 0;
	int m_scoreToProceed = 0;
	int m_currentScore = 0;
	int m_maxNumEnemies = 0;

	Timer m_enemySpawnTimer;

	int m_currentMapIndex = 0;
	Map* m_currentMap	= nullptr;
	std::vector<Map*> m_maps;
};