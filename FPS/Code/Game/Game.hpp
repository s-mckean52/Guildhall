#pragma once
#include "Game/GameCommon.hpp"
#include "Game/AnimatedLight.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Transform.hpp"
#include <vector>

class Entity;
class XboxController;
class Shader;
class GPUMesh;
class Clock;
class NamedStrings;


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

	//Input
	void UpdateFromInput( float deltaSeconds );
	void UpdateDrawDebugObjects();
	void UpdateInputLights( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );

	//Rendering
	void RenderWorld() const;
	void RenderRingOfSpheres() const;
	void RenderUI() const;
	void EnableLightsForRendering() const;

	//Other
	void UpdateBasedOnMouseMovement();
	void UpdateObjectRotations( float deltaSeconds );
	void UpdateLightPositions();
	void CycleLightToModify();

	void TranslateCamera( Camera& camera, const Vec3& directionToMove );
	void ChangeClearColor( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );
	void AddAmbientLightIntensity( float intensityToAdd );
	void AddPointLightIntensity( float intensityToAdd );
	void AddGamma( float gammaToAdd );
	void AddSpecFactor( float factorToAdd );
	void AddSpecPower( float powerToAdd );
	void CycleAttenuationMode();
	void ChangeShader( int direction );
	void AddShader( std::string shaderName, Shader* shader );


	//Static
	static Vec3 ParabolaEquation( float x, float y );
	static void GainFocus( NamedStrings* args );
	static void LoseFocus( NamedStrings* args );
	static void light_set_ambient_color( NamedStrings* args );
	static void light_set_color( NamedStrings* args );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Clock* m_gameClock = nullptr;

	GPUMesh* m_meshCube = nullptr;
	GPUMesh* m_uvSphere = nullptr;
	GPUMesh* m_plane = nullptr;
	GPUMesh* m_quad = nullptr;

	Vec3 m_p0;
	Vec3 m_p1;
	Vec3 m_p2;
	Vec3 m_p3;

	Transform* m_quadTransform = nullptr;
	Transform* m_cubeTransform = nullptr;
	Transform* m_sphereTransform = nullptr;
	Transform* m_ringTransform = nullptr;

	unsigned int m_selectedLight = 0;
	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 1.f;
	AnimatedLight m_animatedLights[MAX_LIGHTS];

	float m_distanceFromCamera = -1.f;

	Texture* m_testImage		= nullptr;
	Texture* m_pokeball			= nullptr;
	Texture* m_normalMap		= nullptr;
	Texture* m_dissolveImage	= nullptr;
	
	int m_currentShaderIndex = 0;
	std::vector<std::string> m_shaderNames;
	std::vector<Shader*> m_shadersToUse;

	Shader* m_invertColorShader		= nullptr;
	Shader* m_litShader				= nullptr;
	Shader* m_defaultShader			= nullptr;
	Shader* m_normalsShader			= nullptr;
	Shader* m_tangentsShader		= nullptr;
	Shader* m_bitangentsShader		= nullptr;
	Shader* m_surfaceNormalsShader	= nullptr;
	Shader* m_fresnelShader			= nullptr;
	Shader* m_dissolveShader		= nullptr;

	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;

	Rgba8	m_clearColor = Rgba8::BLACK;
	float	m_colorChangeDelay = 1.f;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;

	float m_dissolveAmount = 0.f;
};
