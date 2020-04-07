#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include <vector>

class Entity;
class XboxController;
class Shader;
class GPUMesh;
class Clock;
class NamedStrings;

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

	//Other
	void UpdateBasedOnMouseMovement();
	void UpdateObjectRotations( float deltaSeconds );
	void TranslateCamera( Camera& camera, const Vec3& directionToMove );
	void ChangeClearColor( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );
	void AddAmbientLightIntensity( float intensityToAdd );
	void AddPointLightIntensity( float intensityToAdd );


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

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 1.f;
	Light m_pointLight;

	bool m_isPointLightFollowCamera = false;

	float m_distanceFromCamera = -1.f;

	Texture* m_testImage		= nullptr;
	Texture* m_pokeball			= nullptr;
	
	Shader* m_currentShaderToUse	= nullptr;
	Shader* m_invertColorShader		= nullptr;
	Shader* m_litShader				= nullptr;
	Shader* m_defaultShader			= nullptr;
	Shader* m_normalsShader			= nullptr;
	Shader* m_tangentsShader		= nullptr;
	Shader* m_bitangentsShader		= nullptr;
	Shader* m_sufaceNormalsShader	= nullptr;

	Rgba8	m_clearColor = Rgba8::BLACK;
	float	m_colorChangeDelay = 1.f;
	
	Camera*	m_worldCamera = nullptr;
	Camera* m_UICamera = nullptr;
	bool	m_isQuitting = false;
};
