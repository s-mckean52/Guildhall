#pragma once
#include "Game/GameCommon.hpp"
#include "Game/EnvironmentObject.hpp"
#include "Engine/Core/EngineCommon.hpp"
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
class NamedStrings;
class GameObject;
class PlayerObject;
class EnemyObject;
class Lamp;


struct fresnel_t
{
	Vec3 color;
	float power;
	float factor;
	float padding[3];
};


struct dissolve_t
{
	Vec3 edgeStartColor;
	float amount;
	Vec3 edgeEndColor;
	float edgeRange;
};


struct projection_t
{
	Mat44 matrix;
	Vec3 position;
	float intensity;
};


struct parallax_t
{
	float depth;
	float padding[3];
};


enum GameState
{
	GAME_STATE_PLAY,
	GAME_STATE_LOSE,
	GAME_STATE_WIN,
};


class Game
{
public:
	~Game();
	Game();

	//Basic Game
	void StartUp();
	void ShutDown();
	void Render() const;
	void Update();

	//Start Up
	void LoadFonts();
	void LoadTextures();
	void LoadShaders();
	void CreateWorldObjects();
	void SpawnEnvironmentObject( EnvironmentObjectType objectType, Vec3 const& objectPosition = Vec3::ZERO, Vec3 const& objectDimensions = Vec3::UNIT, float objectOrientationAroundYDegrees = 0.f, Rgba8 const& tint = Rgba8::WHITE );

	//Shut Down
	void DeleteEnvironmentObjects();

	//Input
	void UpdateFromInput( float deltaSeconds );
	void UpdateInputLights( float deltaSeconds );
	void UpdateGameState();

	//Rendering
	void RenderWorld() const;
	void RenderUI() const;
	void RenderTiledFloor() const;
	void EnableLightsForRendering() const;

	//Other
	void UpdatePlayerLightIntensity( float deltaSeconds );
	void PushPlayerOutOfObjects();
	void UpdateCameras( float deltaSeconds );
	void TranslateCamera( Camera& camera, const Vec3& directionToMove );

	void AddAmbientLightIntensity( float intensityToAdd );

	//Static
	static void GainFocus( NamedStrings* args );
	static void LoseFocus( NamedStrings* args );
	static void light_set_ambient_color( NamedStrings* args );

	bool IsQuitting() const { return m_isQuitting; }

private:
	//Infastructure
	Clock* m_gameClock = nullptr;

	GPUMesh* m_meshCube = nullptr;
	GPUMesh* m_uvSphere = nullptr;
	GPUMesh* m_quad = nullptr;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.f;

	float m_distanceFromCamera = -1.f;

	Texture* m_test				= nullptr;
	Texture* m_couchDiffuse		= nullptr;
	Texture* m_couchNormal		= nullptr;
	Texture* m_barkDiffuse		= nullptr;
	Texture* m_barkNormal		= nullptr;
	Texture* m_brickDiffuse		= nullptr;
	Texture* m_brickNormal		= nullptr;
	Texture* m_brickHeight		= nullptr;
	Texture* m_dissolveImage	= nullptr;
	Texture* m_projectionImage	= nullptr;
	Texture* m_pokeball			= nullptr;
	Texture* m_sunriseImage		= nullptr;

	Shader* m_invertColorShader		= nullptr;
	Shader* m_litShader				= nullptr;
	Shader* m_defaultShader			= nullptr;
	Shader* m_normalsShader			= nullptr;
	Shader* m_tangentsShader		= nullptr;
	Shader* m_bitangentsShader		= nullptr;
	Shader* m_surfaceNormalsShader	= nullptr;
	Shader* m_fresnelShader			= nullptr;
	Shader* m_dissolveShader		= nullptr;
	Shader* m_triplanarShader		= nullptr;
	Shader* m_fogShader				= nullptr;
	Shader* m_projectionShader		= nullptr;
	Shader* m_parallaxShader		= nullptr;

	Rgba8	m_clearColor = Rgba8::BLACK;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;


	//Game World
	AABB2 m_endZone;
	GameState m_gameState = GAME_STATE_PLAY;
	PlayerObject* m_player;
	std::vector<EnvironmentObject*> m_enviromentObjects;
	std::vector<EnemyObject*> m_enemyObjects;
	Lamp* m_lamps[5];
};
