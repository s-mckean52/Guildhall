#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
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
	void MoveWorldCamera( float deltaSeconds );

	//Rendering
	void RenderWorld() const;
	void RenderRingOfSpheres() const;

	//Other
	void UpdateBasedOnMouseMovement();
	void UpdateObjectRotations( float deltaSeconds );
	void TranslateCamera( Camera& camera, const Vec3& directionToMove );
	void ChangeClearColor( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );


	//Static
	static Vec3 ParabolaEquation( float x, float y );
	static void GainFocus( NamedStrings* args );
	static void LoseFocus( NamedStrings* args );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Clock* m_gameClock = nullptr;

	GPUMesh* m_meshCube = nullptr;
	GPUMesh* m_uvSphere = nullptr;
	GPUMesh* m_plane = nullptr;
	Transform* m_cubeTransform = nullptr;
	Transform* m_sphereTransform = nullptr;
	Transform* m_ringTransform = nullptr;

	float m_distanceFromCamera = -1.f;

	Texture* m_testImage		= nullptr;
	Texture* m_pokeball			= nullptr;
	Shader* m_invertColorShader	= nullptr;

	Rgba8	m_clearColor = Rgba8::RED;
	float	m_colorChangeDelay = 1.f;
	
	Camera*	m_worldCamera = nullptr;
	//Camera*	m_devConsoleCamera = nullptr;
	bool	m_isQuitting = false;
};
