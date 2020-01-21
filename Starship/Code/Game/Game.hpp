#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"

class Camera;
class Entity;
class PlayerShip;
class Bullet;
class Asteroid;
class Beetle;
class Wasp;
class Debris;
enum  GameState;

enum Waves
{
	INVALID_WAVE_NUM = 0,

	WAVE_1,
	WAVE_2,
	WAVE_3,
	WAVE_4,
	WAVE_5,

	NUM_OF_WAVES
};

class Game
{
public:
	~Game();
	Game();

	void StartUp();
	void ShutDown();

	void SpawnWave();

	void DeleteAllAsteroids();
	void DeleteAllBullets();
	void DeleteAllBeetles();
	void DeleteAllWasps();
	void DeleteAllDebris();

	void SpawnBullet( Vec2 location, float rotation );
	void SpawnAsteroid();
	void SpawnBeetle();
	void SpawnWasp();
	void SpawnDebris( const Entity* dyingEntity, const Entity* killingEntity );

	void AddScreenShakeIntensity( float screenShakeFractionToAdd );

	void Render() const;
	void RenderEntities() const;
	void RenderUI() const;

	void Update( float deltaSeconds );
	void UpdateEntities( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void CheckCollisions();
	void CheckAsteroidCollisionsWithEntity( Entity* entityArrayA );
	void CheckBeetleCollisionsWithEntity( Entity* collider );
	void CheckWaspCollisionsWithEntity( Entity* collider );
	bool DoEntitiesOverlap( Entity* A, Entity* B );
	void DrawLineBetweenEntities( Entity* A, Entity* B ) const;
	void DeleteGarbageEntities();
	void DecrementEnemyCount();

	const GameState GetGameState() { return m_gameState; }
	void SetGameState( GameState newGameState);

	void RenderLifeIcon( const Vec2& position ) const;
	void RenderStarshipTitle() const;

public:
	Waves m_wave = INVALID_WAVE_NUM;
	GameState m_gameState = INVALID_GAME_STATE;

	PlayerShip *m_playerShip = nullptr;
	Bullet *m_bullets[ MAX_BULLET_COUNT ] = { nullptr };
	Asteroid *m_asteroids[ MAX_ASTEROID_COUNT ] = { nullptr };
	Beetle *m_beetles[ MAX_BEETLE_COUNT ] = { nullptr };
	Wasp *m_wasps[ MAX_WASP_COUNT ] = { nullptr };
	Debris *m_debris[ MAX_DEBRIS_COUNT ] = { nullptr };

	Camera m_worldCamera;
	Camera m_uiCamera;

	float m_screenShakeIntensity = 0.f;
	int m_enemiesRemaining = 0;
	float m_restartDelaySeconds = RESTART_DELAY;
};
