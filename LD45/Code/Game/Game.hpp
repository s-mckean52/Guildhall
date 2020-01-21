#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"

class Camera;
class Entity;
class World;
class PlayerController;
enum  GameState;

class Game
{
public:
	~Game();
	Game();

	void StartUp();
	void ShutDown();

	void AddScreenShakeIntensity( float screenShakeFractionToAdd );

	void Render() const;
	void RenderUI() const;

	void Update( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	const GameState GetGameState() { return m_gameState; }
	void SetGameState( GameState newGameState);

	void ShakeCamera( Camera& cameraToShake, float deltaSeconds );
public:
	World *m_world = nullptr;
	GameState m_gameState = INVALID_GAME_STATE;

	Camera m_worldCamera;
	Camera m_worldCamera1;
	Camera m_uiCamera;

	float m_screenShakeIntensity = 0.f;
	int m_enemiesRemaining = 0;
	float m_restartDelaySeconds = RESTART_DELAY;
};
