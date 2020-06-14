#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/MapMaterial.hpp"
#include "Game/MapRegion.hpp"
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
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
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
	MapMaterial::CreateMapMaterialsFromXML( "Data/Definitions/MapMaterialTypes.xml" );
	MapRegion::CreateMapRegionsFromXML( "Data/Definitions/MapRegionTypes.xml" );

	g_RNG = new RandomNumberGenerator();

	EnableDebugRendering();

	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );
	g_theEventSystem->SubscribeEventCallbackMethod( "Map", this, &Game::set_current_map );

	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionPerspective( 60.f, -0.09f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, m_clearColor, 1.0f, 0 );
	UpdateCameraProjection( m_worldCamera );
	g_theRenderer->DisableFog();

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	LoadTextures();
	LoadShaders();
	LoadAudio();

	m_world = new World( this );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete m_world;
	m_world = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_UICamera;
	m_UICamera = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_viewModelsSpriteSheet;
	m_viewModelsSpriteSheet = nullptr;
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
	m_world->Render();
	
	if( g_isDebugDraw )
	{
		RenderWorldDebug();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( g_isDebugDraw )
	{
		RenderUIDebug();
	}
	RenderHUD();
	RenderFPSCounter();
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderHUD() const
{
	Vec2 uiCameraHalfDimensions = m_UICamera->GetCameraDimensions() * 0.5f;
	Vec3 uiCameraBottomLeft = m_UICamera->GetOrthoBottomLeft();
	Vec2 uiCameraBottomLeftXY = Vec2( uiCameraBottomLeft.x, uiCameraBottomLeft.y );
	Vec2 uiCameraCenter = uiCameraBottomLeftXY + uiCameraHalfDimensions;

	//Render HUD Base
	float hudBaseWidth = CAMERA_SIZE_X;
	float hudBaseHeight = hudBaseWidth / m_HUDBase->GetAspect();
	Vec2 hudBaseMin = uiCameraBottomLeftXY;
	Vec2 hudBaseMax = hudBaseMin + Vec2( hudBaseWidth, hudBaseHeight );
	AABB2 hudBaseRect = AABB2( hudBaseMin, hudBaseMax );

	std::vector<Vertex_PCU> hudBaseVerts;
	AppendVertsForAABB2D( hudBaseVerts, hudBaseRect, Rgba8::WHITE );

	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->BindTexture( m_HUDBase );
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->DrawVertexArray( hudBaseVerts );


	//Render Gun Hand
	Vec2 gundHandMinUVs;
	Vec2 gundHandMaxUVs;
	SpriteDefinition const& gundHandSprite = m_viewModelsSpriteSheet->GetSpriteDefinition( 0 );
	gundHandSprite.GetUVs( gundHandMinUVs, gundHandMaxUVs );

	float gundHandHeight = CAMERA_SIZE_Y * 1.25f;
	float gundHandWidth = gundHandHeight * gundHandSprite.GetAspect();
	Vec2 gunHandMins = uiCameraCenter - Vec2( gundHandWidth * 0.5f, HALF_SCREEN_Y );
	gunHandMins.y += hudBaseHeight;
	Vec2 gunHandMaxes = gunHandMins + Vec2( gundHandWidth, gundHandHeight );
	AABB2 gundHandRect = AABB2( gunHandMins, gunHandMaxes );

	std::vector<Vertex_PCU> gunHandVerts;
	AppendVertsForAABB2D( gunHandVerts, gundHandRect, Rgba8::WHITE, gundHandMinUVs, gundHandMaxUVs );
	//AppendVertsForAABB2OutlineAtPoint( gunHandVerts, AABB2( gunHandMins, Vec2( 0.f, 0.f ) ), Rgba8::CYAN, 0.1f );

	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->BindTexture( &gundHandSprite.GetTexture() );
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->DrawVertexArray( gunHandVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderWorldDebug() const
{
	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 cameraTransformMatrix = Mat44::CreateTranslationXYZ( m_worldCamera->GetPosition() );
	cameraTransformMatrix.RotateZDegrees( m_worldCamera->GetYawDegrees() );
	cameraTransformMatrix.RotateYDegrees( m_worldCamera->GetPitchDegrees() );
	cameraTransformMatrix.RotateXDegrees( m_worldCamera->GetRollDegrees() );
	cameraTransformMatrix.ScaleNonUniform3D(m_worldCamera->GetScale());

	Vec3 compasStartPosition = m_worldCamera->GetPosition();
	compasStartPosition += cameraTransformMatrix.TransformVector3D( Vec3( 0.1f, 0.f, 0.f ) );

	Mat44 cameraBasisMatrix = Mat44::CreateUniformScaleXYZ( 0.01f );
	cameraBasisMatrix.SetTranslation3D( compasStartPosition );
	DebugAddWorldBasis( cameraBasisMatrix, 0.f, DEBUG_RENDER_ALWAYS );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUIDebug() const
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
void Game::RenderFPSCounter() const
{
	const float textHeight = 0.15f;
	const float paddingFromRight = 0.015f;
	const float paddingFromTop = 0.05f;

	double deltaSeconds = m_gameClock->GetLastDeltaSeconds();
	double frameTimeMS = deltaSeconds * 1000.0;
	double fps = 1.0 / deltaSeconds;
	std::string frameTimeString = Stringf( "ms/frame: %.f (%.f FPS)", frameTimeMS, fps );
	
	Vec3 orthoTopRight = m_UICamera->GetOrthoTopRight();
	Vec2 cameraTopRight = Vec2( orthoTopRight.x, orthoTopRight.y );
	Vec2 textDimensions = g_devConsoleFont->GetDimensionsForText2D( textHeight, frameTimeString );
	Vec2 textStartPos = cameraTopRight + Vec2( -paddingFromRight - textDimensions.x, -paddingFromTop - textDimensions.y );

	std::vector<Vertex_PCU> textVerts;
	g_devConsoleFont->AddVertsForText2D( textVerts, textStartPos, textHeight, frameTimeString, Rgba8::MakeFromFloats( 0.2f, 0.2f, 0.5f, 1.0f ) );

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

	m_world->Update();
	if( !g_theConsole->IsOpen() )
	{
		UpdateFromInput( deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadTextures()
{
	g_devConsoleFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	m_test = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_HUDBase = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Hud_base.png" );

	Texture* modelSpriteSheet = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/ViewModelsSpriteSheet_8x8.png" );
	m_viewModelsSpriteSheet = new SpriteSheet( *modelSpriteSheet, IntVec2( 8, 8 ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadShaders()
{
	m_testShader = g_theRenderer->GetOrCreateShader("Data/Shaders/WorldOpaque.hlsl");
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	m_spawnSound = g_theAudio->CreateOrGetSound( "Data/Audio/Teleporter.wav" );
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
		g_isDebugDraw = !g_isDebugDraw;
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
void Game::PlaySpawnSound()
{
	float volume	= 0.75f;
	float balance	= 0.f;
	float speed		= 1.f;
	g_theAudio->PlaySound( m_spawnSound, false, volume, balance, speed );
}


//---------------------------------------------------------------------------------------------------------
void Game::set_current_map( EventArgs* args )
{
	std::string mapNameToLoad = args->GetValue( "map", "MISSING" );
	if( mapNameToLoad != "MISSING" )
	{
		m_world->SetCurrentMapByName( mapNameToLoad );
	}
	else
	{
		m_world->PrintLoadedMapsToDevConsole();
	}
}
