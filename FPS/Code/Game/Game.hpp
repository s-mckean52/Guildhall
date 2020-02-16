#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Entity;
class XboxController;
class Shader;

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
	void MoveCamera( float deltaSeconds );

	//Rendering
	void RenderWorld() const;

	//Other
	void ChangeClearColor( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Texture* m_image			= nullptr;
	Shader* m_invertColorShader	= nullptr;

	Rgba8	m_clearColor = Rgba8::RED;
	float	m_colorChangeDelay = 1.f;
	
	Camera	m_worldCamera;
	bool	m_isQuitting = false;
};
