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

	void Render() const;
	void RenderUI() const;

	void Update( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void UpdateGameStatesFromInput();

	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	void UpdateMousePos( const Camera& camera );

	bool IsQuitting() const { return m_isQuitting; }

	GameObject* CreateDisc();
	void DrawGameObjects() const;
	void UpdateGameObjects( float deltaSeconds );
	void CheckGameObjectOverlap( GameObject* gameObject );
	void SetDraggedObject();
	void DestroyGameObjects();

private:
	Vec2 m_mousePos = Vec2();

	Camera m_worldCamera;
	Camera m_uiCamera;
	Vec3 m_focalPoint = Vec3( HALF_SCREEN_X, HALF_SCREEN_Y, 0.f );
	float m_cameraHeight = CAMERA_SIZE_Y;
	float m_cameraMinHeight = CAMERA_SIZE_Y * 0.5f;
	float m_cameraMaxHeight = CAMERA_SIZE_Y * 1.5f;
	float m_cameraMoveAmountPerFrame = 5.f;

	Vec2 m_draggedObjectOffset;
	GameObject* m_draggedObject;

	bool	m_isQuitting = false;

	Physics2D* m_physics2D = nullptr;

	std::vector<GameObject*> m_gameObjects;
};
