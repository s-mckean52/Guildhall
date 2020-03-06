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
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Core/Clock.hpp"
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
	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );
	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionPerspective( 60.f, -0.1f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );

	m_devConsoleCamera = new Camera( g_theRenderer );
	m_devConsoleCamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	m_meshCube = new GPUMesh( g_theRenderer );
	AddVerticiesAndIndiciesForCubeMesh( m_meshCube, 2.f );

	std::vector< Vertex_PCU > uvSphereVerticies;
	std::vector< unsigned int > uvSphereIndicies;
	m_uvSphere = new GPUMesh( g_theRenderer );
	AddUVSphereToIndexedVertexArray( uvSphereVerticies, uvSphereIndicies, Vec3(), 2.f, 32, 64, Rgba8::WHITE );
	m_uvSphere->UpdateVerticies( static_cast<unsigned int>( uvSphereVerticies.size() ), &uvSphereVerticies[ 0 ] );
	m_uvSphere->UpdateIndicies( static_cast<unsigned int>( uvSphereIndicies.size() ), &uvSphereIndicies[ 0 ] );

	std::vector<Vertex_PCU> planeVerts;
	std::vector<unsigned int> planeIndicies;
	m_plane = new GPUMesh( g_theRenderer );
// 	AddPlaneToIndexedVertexArray(	planeVerts, planeIndicies, Vec3(), Rgba8::GREEN,
// 									Vec3::RIGHT, -1.f, 1.f,
// 									Vec3::UP, -1.f, 1.f, 3 );
	AddSurfaceToIndexedVertexArray( planeVerts, planeIndicies, Vec3(), Rgba8::GREEN,
									-1.f, 1.f, 10,
									-1.f, 1.f, 10, ParabolaEquation );
	m_plane->UpdateVerticies( static_cast<unsigned int>( planeVerts.size() ), &planeVerts[ 0 ] );
	m_plane->UpdateIndicies( static_cast<unsigned int>( planeIndicies.size() ), &planeIndicies[ 0 ] );

	m_cubeTransform = new Transform();
	m_sphereTransform = new Transform();
	m_ringTransform = new Transform();
	m_cubeTransform->SetPosition( Vec3( 1.f, 0.5f, -12.f ) );

	m_image				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_invertColorShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/invertColor.hlsl" );
	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_devConsoleCamera;
	m_devConsoleCamera = nullptr;

	delete m_meshCube;
	m_meshCube = nullptr;

	delete m_cubeTransform;
	m_cubeTransform = nullptr;

	delete m_sphereTransform;
	m_sphereTransform = nullptr;

	delete m_ringTransform;
	m_ringTransform = nullptr;

	delete m_uvSphere;
	m_uvSphere = nullptr;

	delete m_plane;
	m_plane = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LEQUAL, true );
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
	AppendVertsForAABB2D( aabb2, AABB2( -1.0f, -1.0f, 1.0f, 1.0f ), Rgba8::WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	RenderRingOfSpheres();

	Mat44 model = m_cubeTransform->ToMatrix();
	g_theRenderer->SetModelMatrix( model );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawMesh( m_meshCube );

	TranslateVertexArray( aabb2 , Vec3( 0.f, 0.f, -10.f ) );
	g_theRenderer->SetModelMatrix( Mat44::IDENTITY );
	g_theRenderer->BindTexture( m_image );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( aabb2 );

	model = m_cubeTransform->ToMatrix();
	model.SetTranslation2D( Vec2( 10.f, 0.f ) );
	g_theRenderer->SetModelMatrix( model );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawMesh( m_plane );

}


//---------------------------------------------------------------------------------------------------------
void Game::RenderRingOfSpheres() const
{
/*	Mat44 model = m_sphereTransform->ToMatrix();*/
/*	g_theRenderer->SetModelMatrix( model );*/
	g_theRenderer->BindTexture( m_image );
	g_theRenderer->BindShader( (Shader*)nullptr );
/*	g_theRenderer->DrawMesh( m_uvSphere );*/

	float ringRadius = 50.f;
	int numberOfSpheres = 64;
	float degreeStep = 360.f / numberOfSpheres;

	float currentAngleDegrees = 0.f;
	for( int sphereNum = 0; sphereNum < numberOfSpheres; ++sphereNum )
	{
		Mat44 ringMat = m_ringTransform->ToMatrix();
		ringMat.RotateZDegrees( currentAngleDegrees );

		Mat44 sphereMat = m_sphereTransform->ToMatrix();
		Vec3 worldPosition = ringMat.TransformPosition3D( Vec3( ringRadius, 0.f, 0.f ) );
		sphereMat.SetTranslation3D( worldPosition );

		g_theRenderer->SetModelMatrix( sphereMat );
		g_theRenderer->DrawMesh( m_uvSphere );

		currentAngleDegrees += degreeStep;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateBasedOnMouseMovement()
{
	Vec2 relativeMovement = g_theInput->GetCursorRelativeMovement();
	m_worldCamera->AddPitchYawRollRotationDegrees( relativeMovement.y, relativeMovement.x, 0.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateObjectRotations( float deltaSeconds )
{
	m_cubeTransform->AddRotationPitchYawRollDegrees( 0.f, 90.f * deltaSeconds, 0.f );
	m_sphereTransform->AddRotationPitchYawRollDegrees( 0.f, 90.f * deltaSeconds, 0.f );
	m_ringTransform->AddRotationPitchYawRollDegrees( 0.f, 0.f, 10.f * deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::TranslateCamera( Camera& camera, const Vec3& directionToMove )
{
	//Mat44 cameraView = camera.GetViewMatrix();
	Mat44 cameraView = camera.GetTransform().ToMatrix();
	Vec3 translation = cameraView.TransformVector3D( directionToMove );
	camera.Translate( translation );
}


//---------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	if( !g_theConsole->IsOpen() )
	{
		UpdateFromInput( deltaSeconds );
	}
	UpdateObjectRotations( deltaSeconds );

	ChangeClearColor( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFromInput( float deltaSeconds )
{
	UpdateBasedOnMouseMovement();

	MoveWorldCamera( deltaSeconds );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}

	if( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_worldCamera->SetPosition( Vec3( 0.0f ) );
		m_worldCamera->SetPitchYawRollRotationDegrees( 0.f, 0.f, 0.0f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCamera( float deltaSeconds )
{
	float moveSpeed = 5.f * deltaSeconds;
	if( g_theInput->IsKeyPressed( KEY_CODE_SHIFT ) )
	{
		moveSpeed *= 2.f;
	}

	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( 0.f, 0.f, -moveSpeed ) );
	}	
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( 0.f, 0.f, moveSpeed ) );
	}	

	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( -moveSpeed, 0.f, 0.f ) );
	}	
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( moveSpeed, 0.f, 0.f ) );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_SPACEBAR ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( 0.f, moveSpeed, 0.f ) );
	}
	if( g_theInput->IsKeyPressed( 'C' ) )
	{
		TranslateCamera( *m_worldCamera, Vec3( 0.f, -moveSpeed, 0.f ) );
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

	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, m_clearColor, 1.0f, 0 );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
STATIC Vec3 Game::ParabolaEquation( float x, float y )
{
	UNUSED( y );
	x -= 0.5f;
	x *= 2.f;
	return Vec3( 0.f, 0.f, x * x );
}

//---------------------------------------------------------------------------------------------------------
STATIC void Game::GainFocus( NamedStrings* args )
{
	UNUSED( args );
	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::LoseFocus( NamedStrings* args )
{
	UNUSED( args );
	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );
}