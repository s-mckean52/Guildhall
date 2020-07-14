#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>

class Entity;
class XboxController;
class Shader;
class GPUMesh;
class Clock;
class Player;
class Enemy;
class World;
class Map;
class Cursor;
class NamedProperties;
class UIButton;
struct Vertex_PCUTBN;
struct AABB3;

enum CameraViewOrientation
{
	RIGHT_HAND_X_RIGHT_Y_UP,
	RIGHT_HAND_X_FORWARD_Y_LEFT,
};

enum GameState
{
	GAME_STATE_LOADING,
	GAME_STATE_MENU,
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_DEAD,
};

enum MenuState
{
	MENU_STATE_MAIN,
	MENU_STATE_ABILITY_SELECT,
	MENU_STATE_OPTIONS,
};


class Game
{
public:
	~Game();
	Game();

	//Basic Game
	void StartUp();
	void ShutDown();
	void Render();
	void Update();

	//Input
	void UpdateFromInput();
	void MoveWorldCamera();
	void UpdateCursor();
	void UpdateCameras();

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_RIGHT_Y_UP );
	void RenderBasedOnState() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;

	//Accessors
	Vec2	GetCursorPosition() const;
	Map*	GetCurrentMap() const;
	Player* GetPlayer() const			{ return m_player; }
	Camera*	GetPlayerCamera() const		{ return m_worldCamera; }
	Camera*	GetUICamera() const			{ return m_UICamera; }
	Clock*	GetGameClock() const		{ return m_gameClock; }
	bool	IsQuitting() const			{ return m_isQuitting; }
	Enemy*	GetHoveredEnemy() const		{ return m_hoveredEnemy; }

	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );


	void UpdateStateBasedOnGameState();
	void LoadGame();
	void LoadAssets();
	void RenderLoading() const;
	
	void UpdateMenuFromInput();
	void UpdateMainMenu();
	void UpdateAbilitySelect();
	void CreateMenuButtons();
	void RenderMenu() const;
	void NewGameOnClick();
	void BackOnClick();
	void ReturnToMenuOnClick();
	void StartGameOnClick();
	void ResumeGameOnClick();
	void QuitOnClick();

	void UpdatePauseFromInput();
	void RenderPause() const;

	void UpdateDeadFromInput();
	void RenderDead() const;

private:
	GameState m_gameState = GAME_STATE_LOADING;
	MenuState m_menuState = MENU_STATE_MAIN;

	Clock* m_gameClock = nullptr;

	Enemy* m_hoveredEnemy	= nullptr;
	Cursor* m_cursor		= nullptr;
	Player* m_player		= nullptr;
	World* m_world			= nullptr;

	Texture*	m_test			= nullptr;
	Shader*		m_testShader	= nullptr;
	SoundID		m_testSound;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 1.f;

	Rgba8	m_clearColor = Rgba8::BLACK;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;

	std::vector<UIButton*> m_mainMenuButtons;
	std::vector<UIButton*> m_abilityMenuButtons;
	std::vector<UIButton*> m_pauseMenuButtons;
	std::vector<UIButton*> m_deadMenuButtons;

	int m_selectedAbilitySlot = 0;
	int m_selectedAbilityButton = 0;
	int m_firstAbilityToDisplay = 0;
	Strings m_selectedAbilities = { "Blink", "Blink", "Blink", "Blink" };
	Strings m_shownAbilities;
};