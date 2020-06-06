#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/ColorString.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Material.hpp"
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
	EnableDebugRendering();

	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );

	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );
	g_theInput->ShowSystemCursor( false );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionOrthographic( CAMERA_SIZE_Y, -0.09f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, m_clearColor, 1.0f, 0 );
	g_theRenderer->DisableFog();

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	m_test				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_testShader		= g_theRenderer->GetOrCreateShader( "Data/Shaders/WorldOpaque.hlsl" );
	m_testSound			= g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_UICamera;
	m_UICamera = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render()
{
	UpdateCameraView( m_worldCamera );

	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );

	g_theRenderer->BindSampler( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_BACK );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LEQUAL, true );

	g_theRenderer->DisableAllLights();
	g_theRenderer->SetAmbientLight( m_ambientColor, m_ambientIntensity );
	EnableLightsForRendering();

	RenderWorld();

	g_theRenderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );


	//Render UI
	g_theRenderer->BeginCamera( *m_UICamera );
	RenderUI();
	g_theRenderer->EndCamera( *m_UICamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderWorld() const
{
	g_theRenderer->BindTexture( m_test );
	g_theRenderer->BindShader( (Shader*)nullptr );

	DrawCircleAtPoint( Vec2::ZERO, 1.f, Rgba8::GREEN, 0.1f );
	DrawCircleAtPoint( m_cursorPosition, 1.f, Rgba8::CYAN, 0.1f );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	const float textHeight = 0.15f;
	const float paddingFromLeft = 0.015f;
	const float paddingFromTop = 0.05f;

	std::vector<ColorString> strings;
	std::vector<Vertex_PCU> textVerts;

	Transform cameraTransform = m_worldCamera->GetTransform();
	Vec3 cameraPosition = cameraTransform.GetPosition();
	Vec3 cameraRotationPitchYawRollDegrees = cameraTransform.GetRotationPitchYawRollDegrees();
	Mat44 cameraView = m_worldCamera->GetViewMatrix();
	MatrixInvertOrthoNormal( cameraView );

// 	strings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "Camera - Rotation (Pitch, Yaw, Roll): (%.2f, %.2f, %.2f)", cameraRotationPitchYawRollDegrees.x, cameraRotationPitchYawRollDegrees.y, cameraRotationPitchYawRollDegrees.z ) ) );
// 	strings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "       - Position (x, y, z):          (%.2f, %.2f, %.2f)", cameraPosition.x, cameraPosition.y, cameraPosition.z ) ) );
// 	strings.push_back( ColorString( Rgba8::RED,		Stringf( "iBasis (forward, +x world-east when identity): (%.2f, %.2f, %.2f)", cameraView.Ix, cameraView.Iy, cameraView.Iz ) ) );
// 	strings.push_back( ColorString( Rgba8::GREEN,	Stringf( "jBasis (left, +y world-north when identity  ): (%.2f, %.2f, %.2f)", cameraView.Jx, cameraView.Jy, cameraView.Jz ) ) );
// 	strings.push_back( ColorString( Rgba8::BLUE,	Stringf( "kBasis (up, +z world-up when identity       ): (%.2f, %.2f, %.2f)", cameraView.Kx, cameraView.Ky, cameraView.Kz ) ) );

	Vec2 cameraTopLeft = Vec2( m_UICamera->GetOrthoBottomLeft().x, m_UICamera->GetOrthoTopRight().y );
	Vec2 textStartPos = cameraTopLeft + Vec2( paddingFromLeft, -paddingFromTop - textHeight );
	for( int stringIndex = 0; stringIndex < strings.size(); ++stringIndex )
	{
		ColorString& coloredString = strings[ stringIndex ];
		g_devConsoleFont->AddVertsForText2D( textVerts, textStartPos, textHeight, coloredString.m_text, coloredString.m_color );
		textStartPos -= Vec2( 0.f, textHeight + paddingFromTop );
	}

// 	g_theRenderer->BindShader( (Shader*)nullptr );
// 	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
// 	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
// 	g_theRenderer->DrawVertexArray( textVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::EnableLightsForRendering() const
{
// 	for( unsigned int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
// 	{
// 		g_theRenderer->EnableLight( lightIndex, m_animatedLights[lightIndex].light );
// 	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameraProjection( Camera* camera )
{
	float mat[] = {
		 1,  0,  0,  0,
		 0,  1,  0,  0,
		 0,  0,  1,  0,
		 0,  0,  0,  1
	};
	Mat44 transformationMatrix = Mat44( mat );
	Mat44 cameraProjection = camera->GetProjectionMatrix();
	cameraProjection.TransformBy( transformationMatrix );
	camera->SetProjectionMatrix( cameraProjection );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameraView( Camera* camera, CameraViewOrientation cameraOrientation )
{
	Mat44 viewMatrix = Mat44::CreateTranslationXYZ( camera->GetPosition() );
	
	switch( cameraOrientation )
	{
	case RIGHT_HAND_X_FORWARD_Y_LEFT:
		viewMatrix.RotateZDegrees( camera->GetYawDegrees() );
		viewMatrix.RotateYDegrees( camera->GetPitchDegrees() );
		viewMatrix.RotateXDegrees( camera->GetRollDegrees() );
		break;

	case RIGHT_HAND_X_RIGHT_Y_UP:
		viewMatrix.RotateYDegrees( camera->GetYawDegrees() );
		viewMatrix.RotateXDegrees( camera->GetPitchDegrees() );
		viewMatrix.RotateZDegrees( camera->GetRollDegrees() );
		break;

	}

	viewMatrix.ScaleNonUniform3D( camera->GetScale() );
	MatrixInvertOrthoNormal( viewMatrix );
	camera->SetViewMatrix( viewMatrix );
}


//---------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	if( !g_theConsole->IsOpen() )
	{
		UpdateFromInput( deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFromInput( float deltaSeconds )
{
	UpdateCursorPosition( *m_worldCamera );

	MoveWorldCamera( deltaSeconds );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCamera( float deltaSeconds )
{
	float rightMoveAmount = 0.f;
	float upMoveAmount = 0.f;
	float moveSpeed = 5.f * deltaSeconds;
	if( g_theInput->IsKeyPressed( KEY_CODE_SHIFT ) )
	{
		moveSpeed *= 2.f;
	}

	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		upMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		upMoveAmount -= moveSpeed;
	}	

	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		rightMoveAmount -= moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		rightMoveAmount += moveSpeed;
	}


	Vec3 cameraTranslation = Vec3( rightMoveAmount, upMoveAmount, 0.f );
	m_worldCamera->Translate( cameraTranslation );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCursorPosition( Camera const& camera )
{
	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	Vec3 newMousePos = camera.ClientToWorldPosition( mouseNormalizedPos );
	m_cursorPosition = Vec2( newMousePos.x, newMousePos.y );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::GainFocus( EventArgs* args )
{
	UNUSED( args );
	//g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );
	g_theInput->ShowSystemCursor( false );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::LoseFocus( EventArgs* args )
{
	UNUSED( args );
	//g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );
	g_theInput->ShowSystemCursor( true );
}
