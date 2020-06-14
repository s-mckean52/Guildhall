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
class GPUMesh;
class Clock;
class World;
class NamedProperties;
class SpriteSheet;
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

	//---------------------------------------------------------------------------------------------------------
	// START UP
	//---------------------------------------------------------------------------------------------------------
	//Load Assets
	void LoadTextures();
	void LoadShaders();
	void LoadAudio();

	//---------------------------------------------------------------------------------------------------------

	//Input
	void UpdateFromInput( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );
	void UpdateBasedOnMouseMovement();

	//Rendering
	void UpdateCameraProjection( Camera* camera );
	void UpdateCameraView( Camera* camera, CameraViewOrientation viewOrientation = RIGHT_HAND_X_FORWARD_Y_LEFT );
	void RenderWorld() const;
	void RenderWorldDebug() const;
	void RenderUI() const;
	void RenderHUD() const;
	void RenderUIDebug() const;
	void RenderFPSCounter() const;
	void EnableLightsForRendering() const;

	//Other
	Camera* GetPlayerCamera() const		{ return m_worldCamera; }
	bool	IsQuitting() const			{ return m_isQuitting; }
	void	PlaySpawnSound();
	
	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );


public:
	void set_current_map( EventArgs* args );

private:
	World* m_world = nullptr;

	Clock* m_gameClock = nullptr;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 1.f;

	float m_distanceFromCamera = -1.f;;

	SpriteSheet*	m_viewModelsSpriteSheet	= nullptr;
	Texture*		m_test					= nullptr;
	Texture*		m_HUDBase				= nullptr;
	Shader*			m_testShader			= nullptr;
	SoundID			m_spawnSound;

	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;

	Rgba8	m_clearColor = Rgba8::BLACK;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;

	bool m_isFogEnabled = true;
};