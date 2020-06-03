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
//#include "Engine/Audio/AudioSystem.hpp"
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
	float configAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 0.f );
	g_theConsole->PrintString( Rgba8::MAGENTA, Stringf( "Game Config File Aspect: %.3f", configAspect ) );

	g_RNG = new RandomNumberGenerator();

	EnableDebugRendering();

	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );

	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionPerspective( 60.f, -0.09f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, m_clearColor, 1.0f, 0 );
	UpdateCameraProjection( m_worldCamera );
	g_theRenderer->DisableFog();

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	std::vector<Vertex_PCUTBN> litCubeVerticies;
	std::vector<uint> litCubeIndicies;
	AddTestCubeToIndexVertexArray( litCubeVerticies, litCubeIndicies, AABB3( 0.f, 0.f, 0.f, 1.f, 1.f, 1.f ), Rgba8::WHITE );
	m_meshCube = new GPUMesh( g_theRenderer, litCubeVerticies, litCubeIndicies );

	m_cubeTransform = new Transform();
	m_cubeTransform->SetPosition( Vec3( 2.f, 0.f, 0.f ) );

	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	m_test				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );

	m_testShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/WorldOpaque.hlsl" );

	m_testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
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

	delete m_meshCube;
	m_meshCube = nullptr;

	delete m_cubeTransform;
	m_cubeTransform = nullptr;

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
	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 cameraTransformMatrix = Mat44::CreateTranslationXYZ( m_worldCamera->GetPosition() );
	cameraTransformMatrix.RotateZDegrees( m_worldCamera->GetYawDegrees() );
	cameraTransformMatrix.RotateYDegrees( m_worldCamera->GetPitchDegrees() );
	cameraTransformMatrix.RotateXDegrees( m_worldCamera->GetRollDegrees() );
	cameraTransformMatrix.ScaleNonUniform3D( m_worldCamera->GetScale() );

	Vec3 compasStartPosition = m_worldCamera->GetPosition();
	compasStartPosition += cameraTransformMatrix.TransformVector3D( Vec3( 0.1f, 0.f, 0.f ) );

	Mat44 cameraBasisMatrix = Mat44::CreateUniformScaleXYZ( 0.01f );
	cameraBasisMatrix.SetTranslation3D( compasStartPosition );
	DebugAddWorldBasis( cameraBasisMatrix, 0.f, DEBUG_RENDER_ALWAYS );
// 	DebugAddWorldArrow( cameraBasisStartPosition, cameraBasisStartPosition + Vec3( 0.f, 0.f, 0.f ), Rgba8::RED, 0.f, DEBUG_RENDER_ALWAYS );
// 	DebugAddWorldArrow( cameraBasisStartPosition, cameraBasisStartPosition + Vec3( 0.f, 1.f, 0.f ), Rgba8::GREEN, 0.f, DEBUG_RENDER_ALWAYS );
// 	DebugAddWorldArrow( cameraBasisStartPosition, cameraBasisStartPosition + Vec3( 0.f, 0.f, 1.f ), Rgba8::BLUE, 0.f, DEBUG_RENDER_ALWAYS );

	g_theRenderer->BindTexture( m_test );
	g_theRenderer->BindShader( m_testShader );

	m_cubeTransform->SetPosition( Vec3( 2.f, 0.f, 0.f ) );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix() );
	g_theRenderer->DrawMesh( m_meshCube );

	m_cubeTransform->SetPosition( Vec3( 2.f, 2.f, 0.f ) );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix() );
	g_theRenderer->DrawMesh( m_meshCube );

	m_cubeTransform->SetPosition( Vec3( 0.f, 2.f, 0.f ) );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix() );
	g_theRenderer->DrawMesh( m_meshCube );
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

	strings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "Camera - Rotation (Pitch, Yaw, Roll): (%.2f, %.2f, %.2f)", cameraRotationPitchYawRollDegrees.x, cameraRotationPitchYawRollDegrees.y, cameraRotationPitchYawRollDegrees.z ) ) );
	strings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "       - Position (x, y, z):          (%.2f, %.2f, %.2f)", cameraPosition.x, cameraPosition.y, cameraPosition.z ) ) );
	strings.push_back( ColorString( Rgba8::RED,		Stringf( "iBasis (forward, +x world-east when identity): (%.2f, %.2f, %.2f)", cameraView.Ix, cameraView.Iy, cameraView.Iz ) ) );
	strings.push_back( ColorString( Rgba8::GREEN,	Stringf( "jBasis (left, +y world-north when identity  ): (%.2f, %.2f, %.2f)", cameraView.Jx, cameraView.Jy, cameraView.Jz ) ) );
	strings.push_back( ColorString( Rgba8::BLUE,	Stringf( "kBasis (up, +z world-up when identity       ): (%.2f, %.2f, %.2f)", cameraView.Kx, cameraView.Ky, cameraView.Kz ) ) );

	Vec2 cameraTopLeft = Vec2( m_UICamera->GetOrthoBottomLeft().x, m_UICamera->GetOrthoTopRight().y );
	Vec2 textStartPos = cameraTopLeft + Vec2( paddingFromLeft, -paddingFromTop - textHeight );
	for( int stringIndex = 0; stringIndex < strings.size(); ++stringIndex )
	{
		ColorString& coloredString = strings[ stringIndex ];
		g_devConsoleFont->AddVertsForText2D( textVerts, textStartPos, textHeight, coloredString.m_text, coloredString.m_color );
		textStartPos -= Vec2( 0.f, textHeight + paddingFromTop );
	}

	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->DrawVertexArray( textVerts );
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
void Game::AddTestCubeToIndexVertexArray( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indexArray, AABB3 const& box, Rgba8 const& color )
{
// 	Vec3 boxDimensions = box.GetDimensions();
// 	Vec3 boxAdjustedMaxes = Vec3( box.mins.x + boxDimensions.x, box.mins.y + boxDimensions.y, box.mins.z + boxDimensions.z );

	Vec3 frontBottomLeft =	box.mins;
	Vec3 frontBottomRight =	Vec3( box.maxes.x,	box.mins.y,		box.mins.z );
	Vec3 frontTopLeft =		Vec3( box.mins.x,	box.mins.y,		box.maxes.z );
	Vec3 frontTopRight =	Vec3( box.maxes.x,	box.mins.y,		box.maxes.z );

	//looking At Back
	Vec3 backBottomLeft =	Vec3( box.maxes.x,	box.maxes.y,	box.mins.z );
	Vec3 backBottomRight =	Vec3( box.mins.x,	box.maxes.y,	box.mins.z );
	Vec3 backTopLeft =		Vec3( box.maxes.x,	box.maxes.y,	box.maxes.z );
	Vec3 backTopRight =		Vec3( box.mins.x,	box.maxes.y,	box.maxes.z );

	Vec3 frontTangent = ( frontBottomRight - frontBottomLeft ).GetNormalize();
	Vec3 frontBitangent = ( frontTopLeft - frontBottomLeft ).GetNormalize();
	Vec3 frontNormal = CrossProduct3D( frontTangent, frontBitangent );

	std::vector<Vertex_PCUTBN> boxVerticies = {
		//Front
		Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomRight,	color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontTopLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 1.f ) ),

		//Right
		Vertex_PCUTBN( frontBottomRight,	color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomLeft,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopLeft,			color,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Back																									 		    
		Vertex_PCUTBN( backBottomLeft,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopLeft,			color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Left																									 		    
		Vertex_PCUTBN( backBottomRight,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopRight,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontTopLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Top																									 		    
		Vertex_PCUTBN( frontTopLeft,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopRight,		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopLeft, 		color,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 1.f ) ),

		//Bottom
		Vertex_PCUTBN( backBottomRight,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomLeft,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontBottomRight,	color,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 1.f ) ),
	};

	std::vector<unsigned int> boxIndicies = {
		//Front
		0, 1, 3,
		0, 3, 2,

		//Right
		4, 5, 7,
		4, 7, 6,

		//Back
		8, 9, 11,
		8, 11, 10,

		//Right
		12, 13, 15,
		12, 15, 14,

		//Top
		16, 17, 19,
		16, 19, 18,

		//Bottom
		20, 21, 23,
		20, 23, 22,
	};

	unsigned int indexOffset = static_cast<unsigned int>( vertexArray.size() );
	for( int vertexIndex = 0; vertexIndex < boxVerticies.size(); ++vertexIndex )
	{
		vertexArray.push_back( boxVerticies[ vertexIndex ] );
	}

	for( int indexIndex = 0; indexIndex < boxIndicies.size(); ++indexIndex )
	{
		indexArray.push_back( indexOffset + boxIndicies[ indexIndex ] );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateBasedOnMouseMovement()
{
	Vec2 relativeMovement = g_theInput->GetCursorRelativeMovement();
	float newPitch = m_worldCamera->GetPitchDegrees() + -relativeMovement.y;
	float newYaw = m_worldCamera->GetYawDegrees() + relativeMovement.x;
	Clamp( newPitch, -89.9f, 89.9f );

	m_worldCamera->SetPitchYawRollRotationDegrees( newPitch, newYaw, 0.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameraProjection( Camera* camera )
{
	float mat[] = {
		 0, -1,  0,  0,
		 0,  0,  1,  0,
		-1,  0,  0,  0,
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

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F1 ) )
	{
		PlayTestSound();
	}

	if( g_theInput->WasKeyJustPressed( 'I' ) )
	{
		m_worldCamera->SetPitchYawRollRotationDegrees( 0.f, 0.f, 0.f );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCamera( float deltaSeconds )
{
	float forwardMoveAmount = 0.f;
	float leftMoveAmount = 0.f;
	float upMoveAmount = 0.f;
	float moveSpeed = 5.f * deltaSeconds;
	if( g_theInput->IsKeyPressed( KEY_CODE_SHIFT ) )
	{
		moveSpeed *= 2.f;
	}

	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		forwardMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		forwardMoveAmount -= moveSpeed;
	}	

	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		leftMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		leftMoveAmount -= moveSpeed;
	}

	if( g_theInput->IsKeyPressed( 'Q' ) )
	{
		upMoveAmount += moveSpeed;
	}
	if( g_theInput->IsKeyPressed( 'E' ) )
	{
		upMoveAmount -= moveSpeed;
	}

	Vec3 cameraForwardXY0 = Vec3( CosDegrees( m_worldCamera->GetYawDegrees() ), SinDegrees( m_worldCamera->GetYawDegrees() ), 0.f );
	Vec3 cameraLeftXY0 = Vec3( -SinDegrees( m_worldCamera->GetYawDegrees() ), CosDegrees( m_worldCamera->GetYawDegrees() ), 0.f );
	Vec3 cameraTranslation = ( cameraForwardXY0 * forwardMoveAmount ) + ( cameraLeftXY0 * leftMoveAmount );
	cameraTranslation.z = upMoveAmount;
	m_worldCamera->Translate( cameraTranslation );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::GainFocus( EventArgs* args )
{
	UNUSED( args );
	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::LoseFocus( EventArgs* args )
{
	UNUSED( args );
	g_theInput->SetCursorMode( MOUSE_MODE_ABSOLUTE );
}


//---------------------------------------------------------------------------------------------------------
void Game::PlayTestSound()
{
	float volume	= g_RNG->RollRandomFloatInRange(  0.5f,  1.0f );
	float balance	= g_RNG->RollRandomFloatInRange( -1.0f,  1.0f );
	float speed		= g_RNG->RollRandomFloatInRange(  0.5f,  2.0f );
	g_theAudio->PlaySound( m_testSound, false, volume, balance, speed );
}