#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"

class Camera;
class Entity;
class World;

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
	void UpdateGameStates();
	void UpdateCameras( float deltaSeconds );

	void ShakeCamera( Camera& cameraToShake, float deltaSeconds );

public:
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;

	World *m_world = nullptr;

	Camera m_worldCamera;
	Camera m_uiCamera;

	float	m_screenShakeIntensity = 0.f;
	int		m_enemiesRemaining = 0;
	float	m_restartDelaySeconds = RESTART_DELAY;
};
