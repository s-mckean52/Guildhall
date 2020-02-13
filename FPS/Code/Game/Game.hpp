#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Entity;
class XboxController;

class Game
{
public:
	~Game();
	Game();

	void StartUp();
	void ShutDown();

	void Render() const;

	void Update( float deltaSeconds );
	void ChangeClearColor( float deltaSeconds );
	void MoveCamera( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Texture* m_image = nullptr;
	Camera	m_worldCamera;
	Rgba8	m_clearColor = Rgba8::RED;
	float	m_colorChangeDelay = 1.f;
	bool	m_isQuitting = false;
};
