#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Map.hpp"
#include "Game/World.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Input/InputSystem.hpp"


RandomNumberGenerator *g_RNG = nullptr;

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
	m_gameState = PLAY_STATE;
	
	g_RNG = new RandomNumberGenerator();

	m_world = new World( this, NUM_MAPS, MAP_SIZE_X, MAP_SIZE_Y );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity( float screenShakeFractionToAdd )
{
	m_screenShakeIntensity += screenShakeFractionToAdd; 
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{

	
	//Render worldCamera
	if( m_gameState != ATTRACT_STATE )
	{
		g_theRenderer->BeginCamera( m_worldCamera );
		//g_theRenderer->BeginCamera( m_worldCamera1 );
		
		m_world->Render();

		g_theRenderer->EndCamera( m_worldCamera );
		//g_theRenderer->EndCamera( m_worldCamera1 );
	}

	//UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_uiCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F1 ) )
	{
		if( m_gameState == DEBUG_STATE )
		{
			m_gameState = PLAY_STATE;
		}
		else
		{
			m_gameState = DEBUG_STATE;
		}
	}

	m_world->Update( deltaSeconds );

	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	//Update m_worldCamera
	if( m_gameState == DEBUG_STATE )
	{
		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( MAP_SIZE_Y * CLIENT_ASPECT, MAP_SIZE_Y ) );
	}
	else
	{
		Vec2 mapDimensions = Vec2( static_cast<float>(m_world->GetCurrentMapDimensions().x), static_cast<float>(m_world->GetCurrentMapDimensions().y) );
		
		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );

		Vec2 player1Positon = m_world->GetCurrentMapPlayer( 0 )->GetPosition();
		float worldCameraPositionX = player1Positon.x - HALF_SCREEN_X;
		float worldCameraPositionY = player1Positon.y - HALF_SCREEN_Y;

		Clampf( worldCameraPositionX, 0.f, mapDimensions.x - CAMERA_SIZE_X );
		Clampf( worldCameraPositionY, 0.f, mapDimensions.y - CAMERA_SIZE_Y );

		m_worldCamera.Translate2D( Vec2( worldCameraPositionX, worldCameraPositionY ) );


		//Player2 Camera
		m_worldCamera1.SetOrthoView( Vec2( HALF_SCREEN_X, 0 ), Vec2( HALF_SCREEN_X, CAMERA_SIZE_Y ) );

		Vec2 player2Positon = m_world->GetCurrentMapPlayer( 1 )->GetPosition();
		float worldCamera1PositionX = player2Positon.x - HALF_SCREEN_X;
		float worldCamera1PositionY = player2Positon.y - HALF_SCREEN_Y;

		Clampf( worldCamera1PositionX, 0.f, mapDimensions.x - CAMERA_SIZE_X );
		Clampf( worldCamera1PositionY, 0.f, mapDimensions.y - CAMERA_SIZE_Y );

		m_worldCamera1.Translate2D( Vec2( worldCamera1PositionX, worldCamera1PositionY ) );
	}
	
	ShakeCamera( m_worldCamera, deltaSeconds );
	
	//Update m_uiCamera
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::SetGameState( GameState newGameState )
{
	m_gameState = newGameState;
}


//---------------------------------------------------------------------------------------------------------
void Game::ShakeCamera( Camera& cameraToShake, float deltaSeconds )
{
	m_screenShakeIntensity -= SCREEN_SHAKE_ABBERATION * deltaSeconds;
	m_screenShakeIntensity = GetClampf( m_screenShakeIntensity, 0.f, 1.f );
	float maxShakeIntensity = MAX_SCREEN_SHAKE_DISPLACEMENT * m_screenShakeIntensity;
	float cameraTranslateX = g_RNG->GetRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	float cameraTranslateY = g_RNG->GetRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	cameraToShake.Translate2D( Vec2( cameraTranslateX, cameraTranslateY ) );
}