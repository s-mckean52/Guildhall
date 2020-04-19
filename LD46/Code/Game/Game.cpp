#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameObject.hpp"
#include "Game/PlayerObject.hpp"
#include "Game/EnemyObject.hpp"
#include "Game/Lamp.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Renderer/Sampler.hpp"
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
	g_theEventSystem->SubscribeEventCallbackFunction( "light_set_ambient_color", light_set_ambient_color );

	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionPerspective( 60.f, -0.1f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, m_clearColor, 1.0f, 0 );
	g_theRenderer->EnableFog( ( 100.f - 0.1f ) * 0.5f, 100.f, Rgba8::YELLOW, m_clearColor );

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );
	m_UICamera->SetPosition( Vec3( HALF_SCREEN_X, HALF_SCREEN_Y, 0.f ) );

	std::vector<Vertex_PCUTBN> litCubeVerts;
	std::vector<unsigned int> litCubeIndicies;
	AddBoxToIndexedVertexArray( litCubeVerts, litCubeIndicies, AABB3( -1.f, -1.f, 1.f, 1.f, 1.f, -1.f ), Rgba8::WHITE );
	m_meshCube = new GPUMesh( g_theRenderer, litCubeVerts, litCubeIndicies );

	std::vector< Vertex_PCUTBN > uvSphereVerticies;
	std::vector< unsigned int > uvSphereIndicies;
	AddUVSphereToIndexedVertexArray( uvSphereVerticies, uvSphereIndicies, Vec3::ZERO, 1.f, 32, 64, Rgba8::WHITE );
	m_uvSphere = new GPUMesh( g_theRenderer, uvSphereVerticies, uvSphereIndicies );

	Vec3 p0 = Vec3(-0.5f, 0.f, 0.5f);
	Vec3 p1 = Vec3(0.5f, 0.f, 0.5f);
	Vec3 p2 = Vec3(0.5f, 0.f, -0.5f);
	Vec3 p3 = Vec3(-0.5f, 0.f, -0.5f);
	std::vector<Vertex_PCUTBN> litQuadVerts;
	AppendVertsForQuad3D( litQuadVerts, p0, p1, p2, p3, Rgba8::WHITE );
	m_quad = new GPUMesh( g_theRenderer );
	m_quad->UpdateVerticies( static_cast<unsigned int>( litQuadVerts.size() ), &litQuadVerts[0] );

	LoadFonts();
	LoadTextures();
	LoadShaders();

	CreateWorldObjects();
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadFonts()
{
	g_devConsoleFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadTextures()
{
	m_dissolveImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/noise.png");
	m_couchDiffuse = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/example_color.png");
	m_couchNormal = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/example_normal.png");
	m_barkDiffuse = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/bark_diffuse.png");
	m_barkNormal = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/bark_normal.png");
	m_brickDiffuse = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Brick/brick_diffuse.png");
	m_brickNormal = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Brick/brick_normal.png");
	m_brickHeight = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Brick/brick_height.png");
	m_test = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");
	m_projectionImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");
	m_sunriseImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/sunrise.png");
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadShaders()
{
	m_defaultShader = nullptr;
	m_invertColorShader = g_theRenderer->GetOrCreateShader("Data/Shaders/invertColor.hlsl");
	m_litShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Lit.hlsl");
	m_normalsShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Normals.hlsl");
	m_tangentsShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Tangents.hlsl");
	m_bitangentsShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Bitangents.hlsl");
	m_surfaceNormalsShader = g_theRenderer->GetOrCreateShader("Data/Shaders/SurfaceNormals.hlsl");
	m_fresnelShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Fresnel.hlsl");
	m_dissolveShader = g_theRenderer->GetOrCreateShader("Data/Shaders/LitDissolve.hlsl");
	m_triplanarShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Triplanar.hlsl");
	m_fogShader = g_theRenderer->GetOrCreateShader("Data/Shaders/LitFog.hlsl");
	m_projectionShader = g_theRenderer->GetOrCreateShader("Data/Shaders/Projection.hlsl");
	m_parallaxShader = g_theRenderer->GetOrCreateShader("Data/Shaders/LitParallax.hlsl");
}


//---------------------------------------------------------------------------------------------------------
void Game::CreateWorldObjects()
{
	m_player = new PlayerObject( Vec3( 0.f, 0.5f, 0.f ), 0.5f );
	m_player->SetCamera( m_worldCamera );

	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -3.f, 0.f, 2.f ),		Vec3( 18.f, 10.f, 2.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -3.f, 0.f, -1.5f ),	Vec3( 4.f, 10.f, 5.f ),		0.f,	Rgba8::CYAN );
 	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 3.5f, 0.f, -1.5f ),	Vec3( 5.f, 10.f, 5.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -13.f, 0.f, -9.f ),	Vec3( 2.f, 10.f, 22.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -8.5f, 0.f, -22.f ),	Vec3( 7.f, 10.f, 6.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -13.f, 0.f, -29.5f ),	Vec3( 6.f, 10.f, 9.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -17.f, 0.f, -41.f ),	Vec3( 2.f, 10.f, 14.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -12.f, 0.f, -49.f ),	Vec3( 8.f, 10.f, 2.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -7.f, 0.f, -45.f ),	Vec3( 2.f, 10.f, 6.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -4.f, 0.f, -48.f ),	Vec3( 4.f, 10.f, 2.f ),		0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 10.f, 0.f, -48.f ),	Vec3( 20.f, 10.f, 2.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 21.f, 0.f, -36.f ),	Vec3( 2.f, 10.f, 22.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 13.f, 0.f, -23.f ),	Vec3( 14.f, 10.f, 4.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 16.f, 0.f, -14.5f ),	Vec3( 2.f, 10.f, 13.f ),	0.f,	Rgba8::CYAN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 10.5f, 0.f, -6.f ),	Vec3( 9.f, 10.f, 4.f ),		0.f,	Rgba8::CYAN );
	//First Inner
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -8.5f, 0.f, -3.5f ),	Vec3( 3.f, 10.f, 5.f ),		0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -6.5f, 0.f, -11.5f ),	Vec3( 7.f, 10.f, 11.f ),	0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 0.5f, 0.f, -12.5f ),	Vec3( 7.f, 10.f, 13.f ),	0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 0.5f, 0.f, -22.f ),	Vec3( 7.f, 10.f, 6.f ),		0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 8.5f, 0.f, -14.5f ),	Vec3( 9.f, 10.f, 9.f ),		0.f,	Rgba8::RED );
	//Second Inner
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -4.5f, 0.f, -30.5f ),	Vec3( 7.f, 10.f, 7.f ),		0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 3.f, 0.f, -28.5f ),	Vec3( 8.f, 10.f, 3.f ),		0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 12.5f, 0.f, -30.5f ),	Vec3( 11.f, 10.f, 7.f ),	0.f,	Rgba8::RED );
	//Third Inner
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -12.f, 0.f, -41.f ),	Vec3( 4.f, 10.f, 10.f ),	0.f,	Rgba8::RED );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( -7.f, 0.f, -38.f ),	Vec3( 6.f, 10.f, 4.f ),		0.f,	Rgba8::GREEN );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 7.f, 0.f, -40.5f ),	Vec3( 22.f, 10.f, 9.f ),	0.f,	Rgba8::BLUE );
	SpawnEnvironmentObject( ENVIROMENT_OBJECT_AABB,		Vec3( 3.f, 0.f, -34.f ),	Vec3( 4.f, 10.f, 4.f ),		0.f,	Rgba8::ORANGE );

	m_lamps[0] = new Lamp( Vec3( -6.f, 0.f, -5.f ) );
	m_lamps[1] = new Lamp( Vec3( -4.f, 0.f, -18.f ) );
	m_lamps[2] = new Lamp( Vec3( 3.f, 0.f, -31.f ) );
	m_lamps[3] = new Lamp( Vec3( 19.f, 0.f, -41.f ) );
	m_lamps[4] = new Lamp( Vec3( 14.f, 0.f, -9.f ) );

	m_endZone = AABB2( -2.f, -47.f, 0.f, -49.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnEnvironmentObject( EnvironmentObjectType objectType, Vec3 const& objectPosition, Vec3 const& objectDimensions, float objectOrientationAroundYDegrees, Rgba8 const& tint )
{
	Vec3 adjustedObjectPosition = objectPosition;
	adjustedObjectPosition.y += objectDimensions.y * 0.5f;
	m_enviromentObjects.push_back( new EnvironmentObject( objectType, adjustedObjectPosition, objectDimensions, objectOrientationAroundYDegrees, tint ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteEnvironmentObjects()
{
	for( int environmentObjectIndex = 0; environmentObjectIndex < m_enviromentObjects.size(); ++environmentObjectIndex )
	{
		delete m_enviromentObjects[environmentObjectIndex];
		m_enviromentObjects[environmentObjectIndex] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	DeleteEnvironmentObjects();

	delete g_RNG;
	g_RNG = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_UICamera;
	m_UICamera = nullptr;

	delete m_meshCube;
	m_meshCube = nullptr;

	delete m_quad;
	m_quad = nullptr;

	delete m_uvSphere;
	m_uvSphere = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_player;
	m_player = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{
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

	//Render UI
	g_theRenderer->BeginCamera( *m_UICamera );
	RenderUI();
	g_theRenderer->EndCamera( *m_UICamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderWorld() const
{ 
	m_player->Render();

	RenderTiledFloor();

	for( int enviromentObjectIndex = 0; enviromentObjectIndex < m_enviromentObjects.size(); ++enviromentObjectIndex )
	{
		EnvironmentObject* currentEnviromentObject = m_enviromentObjects[enviromentObjectIndex];
		currentEnviromentObject->Render();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	AABB2 screenAABB2 = AABB2( 0.f, 0.f, CAMERA_SIZE_X, CAMERA_SIZE_Y );
	
	g_theRenderer->BindSampler(nullptr);
	g_theRenderer->SetCullMode(CULL_MODE_BACK);
	g_theRenderer->SetDepthTest(COMPARE_FUNC_LEQUAL, true);
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->BindNormalTexture( nullptr );

	switch (m_gameState)
	{
	case GAME_STATE_PLAY:
		break;
	case GAME_STATE_LOSE:
	{
		float textSize = 0.5f;
		std::vector<Vertex_PCU> loseText;
		std::string loseString = "THE DARKNESS ENVELOPS YOU";
		g_devConsoleFont->AddVertsForTextInBox2D( loseText, screenAABB2, textSize, loseString, Rgba8::RED );
		g_theRenderer->BindTexture(g_devConsoleFont->GetTexture());
		g_theRenderer->DrawVertexArray( loseText );
		break;
	}
	case GAME_STATE_WIN:
	{
		g_theRenderer->BindTexture( m_sunriseImage );
		std::vector<Vertex_PCU> verts;
		AppendVertsForAABB2D( verts, screenAABB2, Rgba8::WHITE );
		g_theRenderer->DrawVertexArray(verts);
		break;
	}
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderTiledFloor() const
{
	Transform startTrans = Transform( Vec3( -20.f, -0.02f, 3.f ), Vec3::ZERO, Vec3( 10.f, 1.f, 10.f ) );
	g_theRenderer->BindTexture(m_brickDiffuse);
	g_theRenderer->BindNormalTexture(m_brickNormal);
	g_theRenderer->BindSampler(g_theRenderer->m_samplerLinear);
	g_theRenderer->SetCullMode(CULL_MODE_BACK);
	g_theRenderer->SetDepthTest(COMPARE_FUNC_LEQUAL, true);
	g_theRenderer->BindShader(m_litShader);

	for (int x = 0; x < 5; ++x)
	{
		for (int y = 0; y < 6; ++y)
		{
			Transform trans = startTrans;
			trans.Translate( Vec3( 10.f * x, 0.f, -10.f * y ) );
			g_theRenderer->SetModelUBO(trans.ToMatrix(), Rgba8::WHITE, 0.2f, 32.f);
			g_theRenderer->DrawMesh(m_quad);
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::EnableLightsForRendering() const
{
	//Enable Player Light
	m_player->EnableLight();

	//Enable Lamp Lights
	for( int lampIndex = 0; lampIndex < 5; ++lampIndex )
	{
		m_lamps[lampIndex]->EnableLight( lampIndex + 1 );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdatePlayerLightIntensity( float deltaSeconds )
{
	for( int lampIndex = 0; lampIndex < 5; ++lampIndex )
	{
		if( m_lamps[lampIndex]->DiscIsInLight( m_player->GetPosition2D(), m_player->GetRadius() ) )
		{
			m_player->IncreseLightIntensity( 0.1f, deltaSeconds );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::PushPlayerOutOfObjects()
{
	for( int environmentObjectIndex = 0; environmentObjectIndex < m_enviromentObjects.size(); ++environmentObjectIndex )
	{
		EnvironmentObject* currentEnvironmentObject = m_enviromentObjects[environmentObjectIndex];
		Vec3 newPlayerPosition = currentEnvironmentObject->PushDiscOut( m_player->GetPosition2D(), m_player->GetRadius() );
		m_player->SetPosition( newPlayerPosition );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::TranslateCamera( Camera& camera, const Vec3& directionToMove )
{
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
		m_player->Update( deltaSeconds );
		UpdateFromInput( deltaSeconds );
	}
	PushPlayerOutOfObjects();
	UpdatePlayerLightIntensity( deltaSeconds );
	UpdateGameState();
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFromInput( float deltaSeconds )
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}

	if( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_worldCamera->SetPosition( Vec3( 0.0f ) );
		m_worldCamera->SetPitchYawRollRotationDegrees( 0.f, 0.f, 0.0f );
	}
	UpdateInputLights( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateInputLights( float deltaSeconds )
{
	const float ambientIntensityUpdateAmount = 0.5f;
	const float pointLightIntensityUpdateAmount = 0.5f;
	const float specularFactorUpdateAmount = 0.5f;
	const float specularPowerUpdateAmount = 1.5f;
	const float gammaUpdateAmount = 0.75;

	// Update Ambinet Intensity
	if( g_theInput->IsKeyPressed( '9' ) )
	{
		AddAmbientLightIntensity( -ambientIntensityUpdateAmount * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( '0' ) )
	{
		AddAmbientLightIntensity( ambientIntensityUpdateAmount * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameState()
{
	if( m_gameState != GAME_STATE_PLAY ) return;

	if (m_player->IsDead())
	{
		m_gameState = GAME_STATE_LOSE;
	}
	else if( DoDiscAndAABB2Overlap( m_endZone, m_player->GetPosition2D(), m_player->GetRadius() ) )
	{
		m_gameState = GAME_STATE_WIN;
	}

}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::AddAmbientLightIntensity( float intensityToAdd )
{
	float newIntensity = m_ambientIntensity;
	newIntensity += intensityToAdd;
	ClampZeroToOne( newIntensity );

	m_ambientIntensity = newIntensity;
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


//---------------------------------------------------------------------------------------------------------
STATIC void Game::light_set_ambient_color( NamedStrings* args )
{
	Vec3 defaultAmbientColor = Vec3::UNIT;
	Vec3 ambientColor = args->GetValue( "color", defaultAmbientColor );

	Rgba8 ambientRGBA8 = Rgba8::MakeFromFloats( ambientColor.x, ambientColor.y, ambientColor.z, 0.f );
	g_theGame->m_ambientColor = ambientRGBA8;
}