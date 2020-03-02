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

	void UpdateGameStatesFromInput( float deltaSeconds );

	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	void UpdateMousePos( const Camera& camera );

	bool IsQuitting() const { return m_isQuitting; }

	GameObject* CreateDisc();
	GameObject* CreatePolygon( std::vector<Vec2> polygonPoints );
	void AddPointToNewPolygon();
	Vec2 AveragePositions( std::vector<Vec2> points );
	void DrawGameObjects() const;
	void UpdateGameObjects( float deltaSeconds );
	void HandleGameObjectsOutOfBounds( GameObject* gameObject );
	void MoveGameObjectToOppositeSideOfScreen( GameObject* gameObject );
	void BounceGameObjectOffBottomOfScreen( GameObject* gameObject );
	void CheckGameObjectOverlap( GameObject* gameObject );
	void SetDraggedObject();
	void DestroyGameObjects();
	bool IsNextPointValidOnPolygon( const Vec2& point );

	void CreatePolygonFromInput();
	void DrawNewPolygonPoints() const;
	void AddGameObject( GameObject* gameObject );

private:
	Vec2 m_mousePos = Vec2();

	Rgba8 m_polygonLineColor = Rgba8::BLUE;

	Vec2 m_position;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	Vec3 m_focalPoint = Vec3();
	float m_cameraHeight = CAMERA_SIZE_Y;
	float m_cameraMinHeight = CAMERA_SIZE_Y * 0.5f;
	float m_cameraMaxHeight = CAMERA_SIZE_Y * 1.5f;
	float m_cameraMoveAmountPerFrame = 5.f;

	Vec2 m_draggedObjectOffset;
	GameObject* m_draggedObject;

	bool	m_isQuitting = false;
	bool	m_isCreatingPolygon = false;

	Physics2D* m_physics2D = nullptr;

	std::vector<Vec2> m_newPolygonToDraw;
	std::vector<GameObject*> m_gameObjects;
};
