#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/World.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/World.hpp"
#include "Game/PlayerEntity.hpp"


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
	g_RNG = new RandomNumberGenerator();

	m_world = new World( this, NUM_MAPS, MAP_SIZE_X, MAP_SIZE_Y );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_world;
	m_world = nullptr;
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
	g_theRenderer->BeginCamera( m_worldCamera );
	m_world->Render();
	g_theRenderer->EndCamera( m_worldCamera );

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
	UpdateGameStates();

	if( m_isPaused ) return;

	if( m_isSlowMo )
	{
		deltaSeconds *= 0.1f;
	}
	
	if( m_isFastMo )
	{
		deltaSeconds *= 4.f;
	}

	m_world->Update( deltaSeconds );

	UpdateCameras( deltaSeconds );

}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStates()
{
	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		m_isPaused = !m_isPaused;
	}

	if( g_theInput->IsKeyPressed( 'T' ) )
	{
		m_isSlowMo = true;
	}
	else
	{
		m_isSlowMo = false;
	}

	if( g_theInput->IsKeyPressed( 'Y' ) )
	{
		m_isFastMo = true;
	}
	else
	{
		m_isFastMo = false;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	//Update m_worldCamera
	if( g_isDebugCamera )
	{
		if( MAP_SIZE_X < MAP_SIZE_Y )
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( MAP_SIZE_Y * CLIENT_ASPECT, MAP_SIZE_Y ) );
		}
		else
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( MAP_SIZE_X, MAP_SIZE_X / CLIENT_ASPECT ) );
		}
	}
	else
	{
		Vec2 mapDimensions = Vec2( static_cast<float>(m_world->GetCurrentMapDimensions().x), static_cast<float>(m_world->GetCurrentMapDimensions().y) );

		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );

		Vec2 player1Positon = m_world->GetCurrentMapPlayer( 0 )->GetPosition();
		float worldCameraPositionX = player1Positon.x - HALF_SCREEN_X;
		float worldCameraPositionY = player1Positon.y - HALF_SCREEN_Y;

		Clamp( worldCameraPositionX, 0.f, mapDimensions.x - CAMERA_SIZE_X );
		Clamp( worldCameraPositionY, 0.f, mapDimensions.y - CAMERA_SIZE_Y );

		m_worldCamera.Translate2D( Vec2( worldCameraPositionX, worldCameraPositionY ) );
	}
	
	ShakeCamera( m_worldCamera, deltaSeconds);
	
	//Update m_uiCamera
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShakeCamera( Camera& cameraToShake, float deltaSeconds )
{
	m_screenShakeIntensity -= SCREEN_SHAKE_ABBERATION * deltaSeconds;
	m_screenShakeIntensity = GetClamp( m_screenShakeIntensity, 0.f, 1.f );
	float maxShakeIntensity = MAX_SCREEN_SHAKE_DISPLACEMENT * m_screenShakeIntensity;
	float cameraTranslateX = g_RNG->RollRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	float cameraTranslateY = g_RNG->RollRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	cameraToShake.Translate2D( Vec2( cameraTranslateX, cameraTranslateY ) );
}