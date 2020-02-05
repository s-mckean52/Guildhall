#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"

class Entity;
class XboxController;
class Physics2D;
class GameObject;

class Game
{
public:
	~Game();
	Game();

	void StartUp();
	void ShutDown();
	void BeginFrame();
	void EndFrame();

	void AddScreenShakeIntensity( float screenShakeFractionToAdd );

	void Render() const;
	void RenderUI() const;

	void Update( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void UpdateGameStatesFromInput();

	void ShakeCamera( Camera& cameraToShake, float deltaSeconds );

	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	void DrawAABB2() const;
	void TestImageLoad();
	void TestSetFromText();
	void TestXmlAttribute();

	void UpdateMousePos( const Camera& camera );
	void DrawPointArray() const;
	void DrawAtMouse() const;
	void DoesMouseOBB2OverlapOtherOBB2();
	void TestDrawCapsule2D() const;
	void RandomlyGenerateShapes();

	bool IsQuitting() const { return m_isQuitting; }

	void TestSubscribe();
	void TestFireEvent();
	void TestUnsubscribe();
	void GetNearestPoints( float deltaSeconds );
	void RenderShapes() const;
	static void DeclareSunrise();
	static void DeclareSunset();
	static void PrintTimeOfEvent();
	bool MouseIsInShape() const;

	GameObject* CreateDisc();
	void DrawGameObjects() const;


private:
	AABB2 m_AABB = AABB2( 800.f, 300.f, 1200.f, 500.f );

	Vec2 m_discCenter = Vec2( 800.f, 200.f );
	float m_discRadius = 50.f;

	Vec2 m_lineStart = Vec2( 1500.f, 700.f );
	Vec2 m_lineEnd = Vec2( 1456.f, 750.f );

	Rgba8 m_mouseBoxColor = Rgba8::MAGENTA;
	float m_mouseBoxAlpha = 1.f;
	Vec2 m_mousePos = Vec2();
	std::vector<Vec2> m_pointsToDraw;
	OBB2 m_testBox = OBB2(  Vec2( 200.f, 200.f ), Vec2( 300.f, 30.f ), 0.f );
	OBB2 m_mouseBox = OBB2(  Vec2(), Vec2( 50.f, 20.f ), 0.f );

	Vec2 m_capsuleStart		= Vec2( 500.f, 500.f );
	Vec2 m_capsuleEnd		= Vec2( 673.f, 703.f );
	float m_capsuleRadius	= 100.f;

	Camera m_worldCamera;
	Camera m_uiCamera;
	Vec3 m_focalPoint = Vec3( HALF_SCREEN_X, HALF_SCREEN_Y, 0.f );

	float	m_screenShakeIntensity = 0.f;
	bool	m_isQuitting = false;

	Physics2D* m_physics2D = nullptr;

	std::vector<GameObject*> m_gameObjects;
};
