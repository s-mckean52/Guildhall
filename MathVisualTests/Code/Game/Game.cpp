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
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
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

	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	TestImageLoad();
	TestSetFromText();
	TestXmlAttribute();

	TestSubscribe();
	TestFireEvent();
	TestUnsubscribe();

	LoadAssets();

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
	DrawAABB2();
	RenderShapes();
	TestDrawCapsule2D();
	DrawPointArray();
	DrawAtMouse();
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
	m_pointsToDraw.clear();
	UpdateMousePos( m_worldCamera );
	GetNearestPoints( deltaSeconds );

	DoesMouseOBB2OverlapOtherOBB2();

	UpdateGameStatesFromInput();

	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStatesFromInput()
{
	const XboxController& firstPlayerController = g_theInput->GetXboxController( 0 );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_TILDE ) )
	{
		g_theConsole->SetIsOpen( !g_theConsole->IsOpen() );
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F3 ) )
	{
		RandomlyGenerateShapes();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	//Update m_worldCamera
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
	ShakeCamera( m_worldCamera, deltaSeconds);
	
	//Update m_uiCamera
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y) );
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
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawAABB2() const
{
	std::vector< Vertex_PCU > boxVertexArray;
	std::vector< Vertex_PCU > textVertexArray;
	

	g_testFont->AddVertsForTextInBox2D( textVertexArray, m_AABB, 15.f, "Hello, World!", Rgba8::WHITE, 1.f, ALIGN_CENTERED );
	AppendVertsForAABB2D( boxVertexArray, m_AABB, Rgba8::GREEN );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( boxVertexArray );

// 	g_theRenderer->BindTexture( g_testFont->GetTexture() );
// 	g_theRenderer->DrawVertexArray( textVertexArray );
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
void Game::TestXmlAttribute()
{
	std::string startLevelName = g_gameConfigBlackboard.GetValue( "startLevel", "default level name" );
	float aspect = g_gameConfigBlackboard.GetValue( "windowAspect", 100.f );
	bool isFullScreen = g_gameConfigBlackboard.GetValue( "isFullscreen", true );

	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Start Level: %s", startLevelName.c_str() ) );
	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Aspect Ratio: %f", aspect ) );
	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Is Full Screen: %d", isFullScreen ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMousePos( const Camera& camera )
{
	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	AABB2 orthoBounds( camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight() );
	m_mousePos = orthoBounds.GetPointAtUV( mouseNormalizedPos );
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawPointArray() const
{
	for( int pointIndex = 0; pointIndex < m_pointsToDraw.size(); ++pointIndex )
	{
		Vec2 currentPoint = m_pointsToDraw[ pointIndex ];
		DrawLineBetweenPoints( m_mousePos, currentPoint, Rgba8::GRAY, DEBUG_THICKNESS );
		DrawCircleAtPoint( currentPoint, 3.f, Rgba8::RED, 3.f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawAtMouse() const
{
	//DrawCircleAtPoint( m_mousePos, 1.f, Rgba8::RED, 2.f );
	std::vector<Vertex_PCU> obb2Array;

	Rgba8 mouseBoxColor = Rgba8::MAGENTA;
	mouseBoxColor.a = static_cast<unsigned char>( static_cast<float>( mouseBoxColor.a ) * m_mouseBoxAlpha );

	AppendVertsForOBB2D( obb2Array, m_mouseBox, mouseBoxColor );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( obb2Array );

	if( MouseIsInShape() )
	{
		DrawCircleAtPoint( m_mousePos, 2.f, Rgba8::WHITE, 2.f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DoesMouseOBB2OverlapOtherOBB2()
{
	m_mouseBox.SetCenter( m_mousePos );

	if( DoOBBAndOBBOverlap2D( m_mouseBox, m_testBox ) )
	{
		m_mouseBoxAlpha = .3f;
	}
	else if( DoOBBAndCapsuleOverlap2D( m_mouseBox, m_capsuleStart, m_capsuleEnd, m_capsuleRadius ) )
	{
		m_mouseBoxAlpha = .3f;
	}
	else if( DoOBBAndAABBOverlap2D( m_mouseBox, m_AABB ) )
	{
		m_mouseBoxAlpha = .3f;
	}
	else if( DoOBBAndDiscOverlap2D( m_mouseBox, m_discCenter, m_discRadius ) )
	{
		m_mouseBoxAlpha = .3f;
	}
	else if( DoOBBAndLineSegmentOverlap2D( m_mouseBox, m_lineStart, m_lineEnd ) )
	{
		m_mouseBoxAlpha = .3f;
	}
	else
	{
		m_mouseBoxAlpha = 1.f;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::TestDrawCapsule2D() const
{
	std::vector< Vertex_PCU > capsuleVerts;

	AppendVertsForCapsule2D( capsuleVerts, m_capsuleStart, m_capsuleEnd, m_capsuleRadius, Rgba8::BLUE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( capsuleVerts );

// 	DrawCircleAtPoint( m_capsuleStart, 1.f, Rgba8::RED, 2.f );
// 	DrawCircleAtPoint( m_capsuleEnd, 1.f, Rgba8::RED, 2.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::RandomlyGenerateShapes()
{
	float drawBorderProtection = 150.f;
	//Set capsule
	m_capsuleStart.x = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_X - drawBorderProtection );
	m_capsuleStart.y = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_Y - drawBorderProtection );
	m_capsuleEnd.x = m_capsuleStart.x + g_RNG->RollRandomFloatInRange( -300.f, 300.f );
	m_capsuleEnd.y = m_capsuleStart.y + g_RNG->RollRandomFloatInRange( -300.f, 300.f );
	m_capsuleRadius = g_RNG->RollRandomFloatInRange( 20.f, 120.f );

	//Set obb
	Vec2 obbNewCenter;
	obbNewCenter.x = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_X - drawBorderProtection );
	obbNewCenter.y = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_Y - drawBorderProtection );

	float obbNewOrientationDegrees = g_RNG->RollRandomFloatInRange( 0.f, 359.f );

	Vec2 obbNewDimensions;
	obbNewDimensions.x = g_RNG->RollRandomFloatInRange( 100.f, 200.f );
	obbNewDimensions.y = g_RNG->RollRandomFloatInRange( 100.f, 200.f );

	m_testBox.SetCenter( obbNewCenter );
	m_testBox.SetOrientationDegrees( obbNewOrientationDegrees );
	m_testBox.SetDimensions( obbNewDimensions );

	//Set AABB2
	m_AABB.mins.x = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_X - drawBorderProtection );
	m_AABB.mins.y = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_Y - drawBorderProtection );
	m_AABB.maxes.x = m_AABB.mins.x + g_RNG->RollRandomFloatInRange( 100.f, 250.f );
	m_AABB.maxes.y = m_AABB.mins.y + g_RNG->RollRandomFloatInRange( 100.f, 250.f );

	//Set Disc
	m_discCenter.x = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_X - drawBorderProtection );
	m_discCenter.y = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_Y - drawBorderProtection );
	m_discRadius = g_RNG->RollRandomFloatInRange( 40.f, 100.f );

	//Set LineSegment
	m_lineStart.x = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_X - drawBorderProtection );
	m_lineStart.y = g_RNG->RollRandomFloatInRange( drawBorderProtection, CAMERA_SIZE_Y - drawBorderProtection );
	m_lineEnd.y = m_lineStart.y + g_RNG->RollRandomFloatInRange( -100.f, 150.f );
	m_lineEnd.x = m_lineStart.x + g_RNG->RollRandomFloatInRange( -100.f, 150.f );
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


bool Game::MouseIsInShape() const
{
	if( IsPointInsideAABB2D( m_mousePos, m_AABB ) )
	{
		return true;
	}
	else if( IsPointInsideDisk2D( m_mousePos, m_discCenter, m_discRadius ) )
	{
		return true;
	}
	else if( IsPointInsideCapsule2D( m_mousePos, m_capsuleStart, m_capsuleEnd, m_capsuleRadius ) )
	{
		return true;
	}
	else if( IsPointInsideOBB2D( m_mousePos, m_testBox ) )
	{
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------------------------
void Game::TestUnsubscribe()
{
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunrise", DeclareSunrise );
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunset", DeclareSunrise );
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunset", DeclareSunset );
	g_theEventSystem->UnsubscribeEventCallbackFunction( "Sunset", PrintTimeOfEvent );

	g_theEventSystem->FireEvent( "Sunrise" );
	g_theEventSystem->FireEvent( "Sunset" );
}


//---------------------------------------------------------------------------------------------------------
void Game::GetNearestPoints( float deltaSeconds )
{
	//m_testBox.RotateByDegrees( 30.f * deltaSeconds );

	m_pointsToDraw.push_back( m_testBox.GetNearestPoint( m_mousePos ) );
	m_pointsToDraw.push_back( GetNearestPointOnCapsule2D( m_mousePos, m_capsuleStart, m_capsuleEnd, m_capsuleRadius ) );
	m_pointsToDraw.push_back( GetNearestPointOnAABB2D( m_mousePos, m_AABB ) );
	m_pointsToDraw.push_back( GetNearestPointOnDisc2D( m_mousePos, m_discCenter, m_discRadius ) );
	m_pointsToDraw.push_back( GetNearestPointOnLineSegment2D( m_mousePos, m_lineStart, m_lineEnd ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderShapes() const
{
	std::vector<Vertex_PCU> obb2Array;

	AppendVertsForOBB2D( obb2Array, m_testBox, Rgba8::CYAN );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( obb2Array );
	DrawCircleAtPoint( m_discCenter, m_discRadius, Rgba8::YELLOW, DEBUG_THICKNESS );
	DrawLineBetweenPoints( m_lineStart, m_lineEnd, Rgba8::WHITE, DEBUG_THICKNESS );
}