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

class Game
{
public:
	~Game();
	Game();

	//Basic Game
	void StartUp();
	void ShutDown();
	void Render() const;
	void Update( float deltaSeconds );

	//Input
	void UpdateFromInput( float deltaSeconds );
	void MoveWorldCamera( float deltaSeconds );

	//Rendering
	void RenderWorld() const;

	//Other
	void UpdateBasedOnMouseMovement();
	void UpdateCubeRotation( float deltaSeconds );
	void TranslateCamera( Camera& camera, const Vec3& directionToMove );
	void ChangeClearColor( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	static void GainFocus();
	static void LoseFocus();

	bool IsQuitting() const { return m_isQuitting; }

private:
	GPUMesh* m_meshCube = nullptr;
	Transform* m_cubeTransform = nullptr;

	float m_distanceFromCamera = -1.f;

	Texture* m_image			= nullptr;
	Shader* m_invertColorShader	= nullptr;

	Rgba8	m_clearColor = Rgba8::RED;
	float	m_colorChangeDelay = 1.f;
	
	Camera*	m_worldCamera = nullptr;
	Camera*	m_devConsoleCamera = nullptr;
	bool	m_isQuitting = false;
};
