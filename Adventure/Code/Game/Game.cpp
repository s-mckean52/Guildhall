#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
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
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include <string>


RandomNumberGenerator*	g_RNG = nullptr;
BitmapFont*				g_testFont = nullptr;
SpriteSheet*			g_tileSpriteSheet = nullptr;
SpriteSheet*			g_actorSpriteSheet = nullptr;

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
// 	g_theConsole->PrintString( Rgba8::RED, "Game Start Up" );
// 	g_RNG = new RandomNumberGenerator();
// 
// 	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
// 
// 	LoadAssets();
// 	
// 	TestImageLoad();
// 	TestSetFromText();
// 	TestXmlAttribute();
// 	TestSubscribe();
// 	TestFireEvent();
// 	TestUnsubscribe();
// 	TestFireEvent();
// 	LoadDefinitions();
	
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
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
	g_theRenderer->BeginCamera( m_worldCamera );
	//m_map->Render();
	//RenderMouseCursor();
	g_theRenderer->EndCamera( m_worldCamera );


	//UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );
	//RenderUI();
	g_theRenderer->EndCamera( m_uiCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( g_theConsole->IsOpen() )
	{
		g_theConsole->Render( *g_theRenderer, m_uiCamera, 0.2f, g_testFont );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	UpdateMousePos( m_worldCamera );
	m_map->Update( deltaSeconds );

	//UpdateTextPosition( deltaSeconds );

	UpdateGameStatesFromInput();

	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStatesFromInput()
{
	const XboxController& firstPlayerController = g_theInput->GetXboxController( 0 );
	UNUSED( firstPlayerController );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_TILDE ) )
	{
		g_theConsole->SetIsOpen( !g_theConsole->IsOpen() );
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F1 ) )
	{
		g_isDebugDraw = !g_isDebugDraw;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	//Update m_worldCamera
	IntVec2 currentMapDimensions = m_map->GetMapDefinition()->GetDimensions();
	if( g_isDebugCamera )
	{
		if( currentMapDimensions.x < currentMapDimensions.y )
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( static_cast<float>(currentMapDimensions.y) * CLIENT_ASPECT, static_cast<float>(currentMapDimensions.y) ) );
		}
		else
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( static_cast<float>(currentMapDimensions.x), static_cast<float>(currentMapDimensions.x) / CLIENT_ASPECT ) );
		}
	}
	else
	{
		Vec2 mapDimensions = Vec2( static_cast<float>(currentMapDimensions.x), static_cast<float>(currentMapDimensions.y) );

		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );

		Vec2 player1Positon = m_map->GetPlayerPosition();
		float worldCameraPositionX = player1Positon.x - HALF_SCREEN_X;
		float worldCameraPositionY = player1Positon.y - HALF_SCREEN_Y;

		Clamp( worldCameraPositionX, 0.f, mapDimensions.x - CAMERA_SIZE_X );
		Clamp( worldCameraPositionY, 0.f, mapDimensions.y - CAMERA_SIZE_Y );

		m_worldCamera.Translate2D( Vec2( worldCameraPositionX, worldCameraPositionY ) );
	}
	
	//Update m_uiCamera
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y) );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateTextPosition( float deltaSeconds )
{
	if( m_textPosition.y == 0.f && m_textPosition.x < 1.f )
	{
		m_textPosition.x += deltaSeconds * 0.5f;
	}
	else if( m_textPosition.x == 1.f && m_textPosition.y < 1.f )
	{
		m_textPosition.y += deltaSeconds * 0.5f;
	}
	else if( m_textPosition.y == 1.f && m_textPosition.x <= 1.f && m_textPosition.x != 0.f )
	{
		m_textPosition.x -= deltaSeconds * 0.5f;
	}
	else if( m_textPosition.x == 0.f && m_textPosition.y <= 1.f && m_textPosition.y != 0.f )
	{
		m_textPosition.y -= deltaSeconds * 0.5f;
	}
	

	m_textPosition.x = GetClamp( m_textPosition.x, 0.f, 1.f );
	m_textPosition.y = GetClamp( m_textPosition.y, 0.f, 1.f );
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

	Texture* terrainTileTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_32x32.png" );
	g_tileSpriteSheet = new SpriteSheet( *terrainTileTexture, IntVec2( 32, 32 ) );

	Texture* actorSpritesTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaCharacters_12x53.png" );
	g_actorSpriteSheet = new SpriteSheet( *actorSpritesTexture, IntVec2( 12, 53 ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestTextInBox() const
{
	std::vector< Vertex_PCU > boxVertexArray;
	std::vector< Vertex_PCU > textVertexArray;
	AABB2 box( 8.f, 3.f, 12.f, 5.f );

	g_testFont->AddVertsForTextInBox2D( textVertexArray, box, .15f, "Hello, World!", Rgba8::WHITE, 1.f, m_textPosition );
	AppendVertsForAABB2D( boxVertexArray, box, Rgba8::GREEN );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( boxVertexArray );

	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVertexArray );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestImageLoad()
{
	Image newImage( "Data/Images/Test_StbiFlippedAndOpenGl.png" );
	IntVec2 imageDimensions = newImage.GetDimensions();

	Rgba8 texelData = newImage.GetTexelColor( 0, imageDimensions.y - 1 );
	g_theConsole->PrintString( texelData, "Top Left Color - Original" );

	newImage.SetTexelColor( 0, imageDimensions.y - 1, Rgba8::RED );

	texelData = newImage.GetTexelColor( 0, imageDimensions.y - 1 );
	g_theConsole->PrintString( texelData, "Top Left Color - New" );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestXmlAttribute()
{
	std::string startLevelName	= g_gameConfigBlackboard.GetValue( "startLevel", "default level name" );
	float aspect				= g_gameConfigBlackboard.GetValue( "windowAspect", 100.f );
	bool isFullScreen			= g_gameConfigBlackboard.GetValue( "isFullscreen", true );

	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Start Level: %s", startLevelName.c_str() ) );
	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Aspect Ratio: %f", aspect ) );
	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Is Full Screen: %d", isFullScreen ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestSetFromText()
{
	Vec2 newVec = Vec2();
	newVec.SetFromText( " 3,1" );
	newVec.SetFromText( "6.789,3.123" );
	newVec.SetFromText( " 6 , 3.123        " );
	//newVec.SetFromText( " 6, 3, 2" );

	IntVec2 newVec1 = IntVec2();
	newVec1.SetFromText( " 3,1" );
	newVec1.SetFromText( "6.789,3.123" );
	newVec1.SetFromText( " 6 , 4.123        " );
	/*	newVec1.SetFromText( " 6, 3, 2" );*/

	Rgba8 newRgba8 = Rgba8();
	newRgba8.SetFromText( " 100, 100, 100 " );
	newRgba8.SetFromText( "100,100,100,100" );
	newRgba8.SetFromText( "   150.567392   ,   150.190238 , 150.3333   ,   150.23    " );
	//newRgba8.SetFromText( "100,100" );
	//newRgba8.SetFromText( "100,100,100,100,100" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadDefinitions()
{
	TileDefinition::InitializeTileDefinitions();
	MapDefinition::InitializeMapDefinitions();
	ActorDefinition::InitializeActorDefinitions();

	m_map = new Map( "Island" );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMousePos( const Camera& camera )
{
	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	AABB2 orthoBounds( camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight() );
	m_mousePos = orthoBounds.GetPointAtUV( mouseNormalizedPos );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderMouseCursor() const
{
	DrawCircleAtPoint( m_mousePos, .3f, Rgba8::RED, DEBUG_THICKNESS );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestSubscribe()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "Sunrise", DeclareSunrise );
	g_theEventSystem->SubscribeEventCallbackFunction( "Sunrise", PrintTimeOfEvent );
	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	// 	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	g_theEventSystem->SubscribeEventCallbackFunction( "Sunset", PrintTimeOfEvent );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestFireEvent()
{
	g_theEventSystem->FireEvent( "Sunrise" );
	g_theEventSystem->FireEvent( "Sunset" );
}


//---------------------------------------------------------------------------------------------------------
void Game::DeclareSunrise()
{
	g_theConsole->PrintString( Rgba8::YELLOW, "It is sunrise" );
}


//---------------------------------------------------------------------------------------------------------
void Game::DeclareSunset()
{
	g_theConsole->PrintString( Rgba8::YELLOW, "It is sunset" );
}


//---------------------------------------------------------------------------------------------------------
void Game::PrintTimeOfEvent()
{
	g_theConsole->PrintString( Rgba8::YELLOW, "The time is 12:00" );
}


//---------------------------------------------------------------------------------------------------------
void Game::TestUnsubscribe()
{
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunset", PrintTimeOfEvent );
}