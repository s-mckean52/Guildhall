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
struct WorldData;
struct ConnectionData;
struct SpawnData;
struct CameraData;

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
	virtual void StartUp();
	virtual void ShutDown();
	virtual void Render();
	virtual void Update();

	//---------------------------------------------------------------------------------------------------------
	// START UP
	//---------------------------------------------------------------------------------------------------------
	//Load Assets
	void LoadTextures();
	void LoadShaders();
	void LoadAudio();

	//---------------------------------------------------------------------------------------------------------

	//Accessors
	float GetDeltaSeconds() const;

	//Upadate
	void UpdateWorld();

	//Input
	void UpdateFromInput( float deltaSeconds );
	Vec3 MoveWorldCamera( float deltaSeconds, float yawDegrees, InputSystem* input = g_theInput );
	Vec3 MoveEntity( Entity* entityToMove, float yawDegrees, InputSystem* input = g_theInput );
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
	Entity**	GetPossessedEntityPointer()		{ return &m_possessedEntity; }
	bool	IsQuitting() const			{ return m_isQuitting; }
	void	PlaySpawnSound();
	void	TogglePossessEntity();
	void	TryTogglePossessEntity( Entity** in_possessionEntity, Vec3 const& cameraPosition, Vec3 const& cameraForward );
	void	SetPossessedEntity( Entity** in_possessionEntity, Entity* entityToPosses );
	void	MoveCameraToEntityEye( Entity* entity, Vec3& out_position, float& out_yaw );
	void	DebugRaycast( Vec3 const& startPosition, Vec3 const& forwardDir, float maxDistance, float duration = 0.f );
	
	WorldData	GetWorldData();
	ConnectionData GetConnectionData();
	void SetCurrentMapByName( std::string const& mapName );
	void SpawnEntitiesFromSpawnData( SpawnData const& spawnData );
	void UpdateEntitiesFromWorldData( WorldData const& worldData );
	void SetWorldCameraFromCameraData( CameraData const& cameraData );

	//Static
	static void GainFocus( EventArgs* args );
	static void LoseFocus( EventArgs* args );


public:
	void set_current_map( EventArgs* args );

protected:
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
	
	Entity* m_possessedEntity = nullptr;

	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;

	bool m_isFogEnabled = true;
};