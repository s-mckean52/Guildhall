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
class Cursor;
class NamedProperties;
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
	void UpdateFromInput( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );
	void UpdateCursor();
	void UpdateCameras();

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_RIGHT_Y_UP );
	void RenderWorld() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;

	//Accessors
	Vec2	GetCursorPosition() const;
	Camera*	GetPlayerCamera() const		{ return m_worldCamera; }
	Camera*	GetUICamera() const			{ return m_UICamera; }
	Clock*	GetGameClock() const		{ return m_gameClock; }
	bool	IsQuitting() const			{ return m_isQuitting; }

	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );


private:
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
};