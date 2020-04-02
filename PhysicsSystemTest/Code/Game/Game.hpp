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
class Clock;
class NamedStrings;

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

	void Update();
	void UpdateCameras( float deltaSeconds );


	void LoadAssets();
	void LoadTextures();
	void LoadAudio();

	//---------------------------------------------------------------------------------------------------------
	// Input
	void UpdateGameStatesFromInput( float deltaSeconds );
	void UpdateMousePos( const Camera& camera );
	void MoveWorldCameraPosition( float deltaSeconds );
	void ResetCameraPosition( float deltaSeconds );
	void CreateNewGameObject( float deltaSeconds );
	void ChangeDraggedObjectSimulationMode( float deltaSeconds );
	void DropDraggedObject( float deltaSeconds );
	void ModifyPhysicsGravity( float deltaSeconds );
	void ModifyDraggedObjectBounciness( float deltaSeconds );
	void DeleteDraggedObject( float deltaSeconds );
	void ModifyMassOfDraggedObject( float deltaSeconds );
	void ModifyDragOfDraggedObject( float deltaSeconds );
	void ModifyFrictionOfDraggedObject( float deltaSeconds );
	void UpdatePhysicsInput();
	void ModifyDraggedObjectRotation( float deltaSeconds );
	void ModifyDraggedObjectRotationalVelocity( float deltaSeconds );
	//---------------------------------------------------------------------------------------------------------
	
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
	
	Physics2D* GetPhysicsSystem() { return m_physics2D; }

	//---------------------------------------------------------------------------------------------------------
	static void SetPhysicsUpdate( NamedStrings* args );

private:
	Clock* m_gameClock = nullptr;

	Rgba8 m_polygonLineColor = Rgba8::BLUE;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	Vec2 m_mousePos = Vec2();
	Vec3 m_focalPoint = Vec3();
	float m_cameraHeight = CAMERA_SIZE_Y;
	float m_cameraMinHeight = CAMERA_SIZE_Y * 0.5f;
	float m_cameraMaxHeight = CAMERA_SIZE_Y * 1.5f;
	float m_cameraMoveAmountPerFrame = 0.05f;

	Vec2 m_draggedObjectOffset;
	GameObject* m_draggedObject;

	bool	m_isQuitting = false;
	bool	m_isCreatingPolygon = false;

	Physics2D* m_physics2D = nullptr;

	std::vector<Vec2> m_newPolygonToDraw;
	std::vector<GameObject*> m_gameObjects;
};
