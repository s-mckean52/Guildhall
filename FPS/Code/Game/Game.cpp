#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <string>


RandomNumberGenerator*	g_RNG = nullptr;
BitmapFont*				g_devConsoleFont = nullptr;

bool					g_isDebugDraw = false;
bool					g_isDebugCamera = false;


//---------------------------------------------------------------------------------------------------------
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
	m_worldCamera = new Camera( g_theRenderer );
	m_devConsoleCamera = new Camera( g_theRenderer );

	m_worldCamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );
	m_devConsoleCamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	m_image				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	m_invertColorShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/invertColor.hlsl" );
	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );
	RenderWorld();
	g_theRenderer->EndCamera( *m_worldCamera );

	g_theRenderer->BeginCamera( *m_devConsoleCamera );
	g_theConsole->Render( *g_theRenderer, *m_devConsoleCamera, DEV_CONSOLE_LINE_HEIGHT, g_devConsoleFont );
	g_theRenderer->EndCamera( *m_devConsoleCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderWorld() const
{
	std::vector<Vertex_PCU> aabb2;
	AppendVertsForAABB2D( aabb2, AABB2( 0.0f, 0.0f, 2.0f, 2.0f ), Rgba8::WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( aabb2, 1.f, 0.f, Vec2( -3.f, -0.5f ) );
	g_theRenderer->BindTexture( m_image );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( aabb2 );

	TransformVertexArray( aabb2, 1.f, 0.f, Vec2( 3.f, 0.f ) );
	g_theRenderer->BindShader( m_invertColorShader );
	g_theRenderer->DrawVertexArray( aabb2 );
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	if( !g_theConsole->IsOpen() )
	{
		UpdateFromInput( deltaSeconds );
	}

	ChangeClearColor( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFromInput( float deltaSeconds )
{
	MoveCamera( deltaSeconds );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveCamera( float deltaSeconds )
{
	float moveSpeed = 5.f * deltaSeconds;
	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		m_worldCamera->Translate( Vec3( 0.f, moveSpeed, 0.f ) );
	}	
	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		m_worldCamera->Translate( Vec3( -moveSpeed, 0.f, 0.f ) );
	}	
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		m_worldCamera->Translate( Vec3( 0.f, -moveSpeed, 0.f ) );
	}	
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		m_worldCamera->Translate( Vec3( moveSpeed, 0.f, 0.f ) );
	}
}

//---------------------------------------------------------------------------------------------------------
void Game::ChangeClearColor( float deltaSeconds )
{
	m_colorChangeDelay -= deltaSeconds;

	if( m_clearColor.r > 29 && m_colorChangeDelay <= 0 )
	{
		m_clearColor.r -= 30;
		m_colorChangeDelay = 1.f;
	}
	else if( m_clearColor.r < 30 && m_colorChangeDelay <= 0 )
	{
		m_clearColor.r = 0;
	}

	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, m_clearColor, 0.0f, 0 );
}

//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}