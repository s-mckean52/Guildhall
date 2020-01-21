#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"

class Entity;
class World;
class XboxController;

enum GameState
{
	GAME_STATE_LOADING,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_DEATH,
	GAME_STATE_VICTORY,
	GAME_STATE_PAUSED,

	NUM_GAME_STATES
};

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
	void RenderLoadingScreen() const;
	void RenderAttractScreen() const;
	void RenderPauseScreen() const;
	void RenderVictoryScreen() const;
	void RenderDeathScreen() const;
	void RenderLives() const;
	void RenderTankIcon( const Vec2& position, const Rgba8& tint ) const;

	void Update( float deltaSeconds );
	void UpdateLoadingState( float deltaSeconds );
	void UpdateAttractState( float deltaSeconds );
	void UpdateVictoryState( float deltaSeconds );
	void UpdateDeathState( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void UpdateGameStatesFromInput();
	void UpdateVictoryFromInput( const XboxController& firstPlayerController );
	void UpdateDeathFromInput( const XboxController& firstPlayerController );
	void UpdatePausedFromInput( const XboxController& firstPlayerController );
	void UpdateAttractFromInput( const XboxController& firstPlayerController );
	void UpdatePlayingFromInput( const XboxController& firstPlayerController );

	void ShakeCamera( Camera& cameraToShake, float deltaSeconds );

	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	void SetGameState( GameState newGameState );
	void DeleteCurrentWorld();
	void SetIsRespawning( bool param1 );

	bool IsQuitting() const { return m_isQuitting; } 

private:
	int	 m_loadingFrame = 0;
	GameState m_gameState = GAME_STATE_LOADING;

	bool m_isSlowMo = false;
	bool m_isFastMo = false;

	World* m_world = nullptr;

	Camera m_worldCamera;
	Camera m_uiCamera;

	float	m_deathOverlayAlphaFraction = 0.f;
	float	m_screenShakeIntensity = 0.f;
	bool	m_isQuitting = false;
	bool	m_isRespawning = false;
	float	m_respawnDelaySeconds = 0.f;
	float	m_restartDelaySeconds = RESTART_DELAY;
	float	m_deathStateTimeSeconds = 0.f;
	float	m_victoryOverlayTimer = 0.f;
	float	m_victoryOverlayAlphaFraction = 0.f;
};
