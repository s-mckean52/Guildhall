#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/GameObject.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include <string>


RandomNumberGenerator* g_RNG = nullptr;
BitmapFont* g_testFont = nullptr;


Game::~Game()
{
}

//---------------------------------------------------------------------------------------------------------
Game::Game()
{
}


//---------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	g_theConsole->PrintString( Rgba8::RED, "Game Start Up" );
	g_RNG = new RandomNumberGenerator();

	m_physics2D = new Physics2D();

	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	LoadAssets();

	m_worldCamera.SetOutputSize( Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
	m_worldCamera.SetPosition( m_focalPoint );
	m_worldCamera.SetProjectionOrthographic( m_cameraHeight );

	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_physics2D;
	m_physics2D = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void Game::EndFrame()
{
	DestroyGameObjects();
	m_physics2D->EndFrame();
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Render worldCamera
	g_theRenderer->BeginCamera( m_worldCamera );
	DrawNewPolygonPoints();
	DrawGameObjects();
	g_theRenderer->EndCamera( m_worldCamera );


	//UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_uiCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( g_theConsole->IsOpen() )
	{
		g_theConsole->Render( *g_theRenderer, m_uiCamera, 30.f, g_testFont );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	UpdateGameStatesFromInput();

	m_physics2D->Update( deltaSeconds );

	UpdateGameObjects( deltaSeconds );

	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStatesFromInput()
{
	UpdateMousePos( m_worldCamera );

	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		m_focalPoint.y += m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		m_focalPoint.x -= m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		m_focalPoint.y -= m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		m_focalPoint.x += m_cameraMoveAmountPerFrame;
	}

	if( g_theInput->IsKeyPressed( 'O' ) )
	{
		m_focalPoint = Vec2( HALF_SCREEN_X, HALF_SCREEN_Y );
		m_cameraHeight = CAMERA_SIZE_Y;
	}

	if( m_isCreatingPolygon )
	{
		CreatePolygonFromInput();
		return;
	}

	if( g_theInput->WasKeyJustPressed( '1' ) && !m_draggedObject )
	{
		GameObject* newGameObject = CreateDisc();
		AddGameObject( newGameObject );
	}
	else if( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	}

	if( g_theInput->WasKeyJustPressed( '2' ) && !m_draggedObject )
	{
		AddPointToNewPolygon();
		m_isCreatingPolygon = true;
	}
	else if( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_KINEMATIC );
	}

	if( g_theInput->WasKeyJustPressed( '3' ) && m_draggedObject )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
	}

	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
	{
		SetDraggedObject();
	}

	if( g_theInput->WasMouseButtonJustReleased( MOUSE_BUTTON_LEFT ) )
	{
		if( m_draggedObject )
		{
			m_draggedObject->m_isHeld = false;
			m_draggedObject = nullptr;
		}
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_DELETE ) || g_theInput->WasKeyJustPressed( KEY_CODE_BACKSPACE ) )
	{
		if( m_draggedObject )
		{
			m_draggedObject->m_isDestroyed = true;
			m_draggedObject = nullptr;
		}
	}

	float scrollAmount = g_theInput->GetScrollAmount();
	if( scrollAmount != 0.f )
	{
		m_cameraHeight += 100.f * -scrollAmount;
		Clamp( m_cameraHeight, m_cameraMinHeight, m_cameraMaxHeight );
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_TILDE ) )
	{
		g_theConsole->SetIsOpen( !g_theConsole->IsOpen() );
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	//Update m_worldCamera
	m_worldCamera.SetPosition( m_focalPoint );
	m_worldCamera.SetProjectionOrthographic( m_cameraHeight );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	LoadTextures();
	LoadAudio();
	g_theConsole->PrintString( Rgba8::GREEN, "Assets Loaded" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadTextures()
{
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGl.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMousePos( const Camera& camera )
{
	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	AABB2 orthoBounds( camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight() );
	m_mousePos = orthoBounds.GetPointAtUV( mouseNormalizedPos );

	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGO = m_gameObjects[ goIndex ];
		if( currentGO )
		{
			Rigidbody2D* currentGORigidbody = currentGO->m_rigidbody;
			currentGO->m_isHovered = currentGORigidbody->m_collider->Contains( m_mousePos );
		}
	}

	if( m_draggedObject )
	{
		m_draggedObject->SetPosition( m_mousePos + m_draggedObjectOffset );
	}
}


//---------------------------------------------------------------------------------------------------------
GameObject* Game::CreateDisc()
{
	Vec2 mousePos = g_theInput->GetMouseNormalizedClientPosition();
	mousePos = m_worldCamera.ClientToWorldPosition( mousePos );

	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody2D();
	gameObject->m_rigidbody->SetPosition( mousePos );

	float radius = g_RNG->RollRandomFloatInRange( 20.f, 50.f );
	DiscCollider2D* collider = m_physics2D->CreateDiscCollider2D( Vec2( 0.f, 0.f ), radius );
	gameObject->m_rigidbody->TakeCollider( collider );

	return gameObject;
}


//---------------------------------------------------------------------------------------------------------
GameObject* Game::CreatePolygon( std::vector<Vec2> polygonPoints )
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody2D();
	
	Vec2 position = AveragePositions( polygonPoints );
	gameObject->m_rigidbody->SetPosition( position );

	for( int pointIndex = 0; pointIndex < polygonPoints.size(); ++pointIndex )
	{
		polygonPoints[ pointIndex ] -= position;
	}

	PolygonCollider2D* collider = m_physics2D->CreatePolygonCollider2D( polygonPoints, Vec2( 0.f, 0.f ) );
	gameObject->m_rigidbody->TakeCollider( collider );

	return gameObject;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddPointToNewPolygon()
{
	Vec2 mousePos = g_theInput->GetMouseNormalizedClientPosition();
	mousePos = m_worldCamera.ClientToWorldPosition( mousePos );

	m_newPolygonToDraw.push_back( mousePos );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Game::AveragePositions( std::vector<Vec2> points )
{
	Vec2 averagePosition;
	for( int pointIndex = 0; pointIndex < points.size(); ++pointIndex )
	{
		averagePosition += points[ pointIndex ];
	}
	averagePosition /= points.size();
	return averagePosition;
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawGameObjects() const
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[ goIndex ] )
		{
			m_gameObjects[ goIndex ]->Draw();
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameObjects( float deltaSeconds )
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject )
		{
			currentGameObject->Update( deltaSeconds );
			CheckGameObjectOverlap( currentGameObject );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckGameObjectOverlap( GameObject* gameObject )
{
	bool isOverlapping = false;
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject && gameObject && currentGameObject != gameObject )
		{
			if( gameObject->m_rigidbody->m_collider->Intersects( currentGameObject->m_rigidbody->m_collider ) )
			{
				isOverlapping = true;
			}
		}
	}
	gameObject->m_isOverlapping = isOverlapping;
}


//---------------------------------------------------------------------------------------------------------
void Game::SetDraggedObject()
{
	int gameObjectSize = static_cast<int>( m_gameObjects.size() ) - 1;
	for( int goIndex = gameObjectSize; goIndex >= 0; --goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject && currentGameObject->m_isHovered )
		{
			m_draggedObject = currentGameObject;
			m_draggedObjectOffset = currentGameObject->m_rigidbody->m_worldPosition - m_mousePos;

			currentGameObject->m_isHeld = true;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DestroyGameObjects()
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[ goIndex] && m_gameObjects[goIndex]->m_isDestroyed )
		{
			delete m_gameObjects[ goIndex ];
			m_gameObjects[ goIndex ] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CreatePolygonFromInput()
{
	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
	{
		AddPointToNewPolygon();
	}

	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_RIGHT ) )
	{
		GameObject* newGameObject = CreatePolygon( m_newPolygonToDraw );
		AddGameObject( newGameObject );
		m_newPolygonToDraw.clear();
		m_isCreatingPolygon = false;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_newPolygonToDraw.clear();
		m_isCreatingPolygon = false;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawNewPolygonPoints() const
{
	for( int pointIndex = 0; pointIndex < m_newPolygonToDraw.size(); ++pointIndex )
	{
		if( pointIndex != m_newPolygonToDraw.size() - 1 )
		{
			Vec2 start = m_newPolygonToDraw[ pointIndex ];
			Vec2 end = m_newPolygonToDraw[ pointIndex + 1 ];
			DrawLineBetweenPoints( start, end, Rgba8::BLUE, 5.f );
		}
		DrawCircleAtPoint( m_newPolygonToDraw[ pointIndex ], 5.f, Rgba8::YELLOW, 3.f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::AddGameObject( GameObject* gameObject )
{
	bool wasPlaced = false;
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[goIndex] == nullptr )
		{
			m_gameObjects[goIndex] = gameObject;
			wasPlaced = true;
			break;
		}
	}
	if( !wasPlaced )
	{
		m_gameObjects.push_back( gameObject );
	}
}