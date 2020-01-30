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
	void UpdateCameras( float deltaSeconds );

	bool IsQuitting() const { return m_isQuitting; }

private:
	Camera	m_worldCamera;
	Rgba8	m_clearColor = Rgba8::RED;
	bool	m_isQuitting = false;
};
