#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
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

	void AddScreenShakeIntensity( float screenShakeFractionToAdd );

	void Render() const;
	void RenderUI() const;
	void RenderMouseCursor() const;

	void TestSubscribe();
	void Update( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void UpdateTextPosition( float deltaSeconds );
	void UpdateGameStatesFromInput();
	void UpdateMousePos( const Camera& camera );

	void ShakeCamera( Camera& cameraToShake, float deltaSeconds );

	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	void TestTextInBox() const;
	void TestImageLoad();
	void TestSetFromText();
	void TestXmlAttribute();
	void LoadDefinitions();
	void TestUnsubscribe();

	void TestFireEvent();
	static void DeclareSunrise();
	static void DeclareSunset();
	static void PrintTimeOfEvent();

	bool IsQuitting() const { return m_isQuitting; }

private:
	Map* m_map;

	Vec2 m_mousePos;

	Camera m_worldCamera;
	Camera m_uiCamera;

	Vec2	m_textPosition = Vec2();
	float	m_screenShakeIntensity = 0.f;
	bool	m_isQuitting = false;
};
