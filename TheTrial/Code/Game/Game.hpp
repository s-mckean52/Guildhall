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
class PlayerEntity;
class NamedProperties;
struct Vertex_PCUTBN;
struct AABB3;

enum CameraViewOrientation
{
	RIGHT_HAND_X_RIGHT_Y_UP,
	RIGHT_HAND_X_FORWARD_Y_LEFT,
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
	void UpdateCursorPosition( Camera const& camera );

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_RIGHT_Y_UP );
	void RenderWorld() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;

	//Accessors
	Vec2	GetCursorPosition() const	{ return m_cursorPosition; }
	Clock*	GetGameClock() const		{ return m_gameClock; }

	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Clock* m_gameClock = nullptr;

	Vec2 m_cursorPosition;
	PlayerEntity* m_player;

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