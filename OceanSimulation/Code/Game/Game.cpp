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
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/GerstnerWaveSimulation.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/IWave.hpp"
#include "Game/WaveSimulation.hpp"
#include "Game/WaterObject.hpp"
#include "Game/TextureCube.hpp"
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
	g_RNG = new RandomNumberGenerator();
	g_RNG->Reset(100);

	g_theJobSystem->CreateWorkerThreads(6);

	EnableDebugRendering();

	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );
	g_theEventSystem->SubscribeEventCallbackMethod( "new_fft_sim", this, &Game::create_new_fft_simulation );
	g_theEventSystem->SubscribeEventCallbackMethod( "fft_xml", this, &Game::fft_from_xml );

	g_theInput->SetCursorMode( MOUSE_MODE_RELATIVE );

	m_worldCamera = new Camera( g_theRenderer );
	m_worldCamera->SetProjectionPerspective( 60.f, -0.09f, -100.f );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, Rgba8::MakeFromFloats( 0.1f, 0.1f, 0.1f ), 1.0f, 0 );
	m_worldCamera->SetPosition( Vec3( -8.f, -8.f, 1.f ) );
	m_worldCamera->SetPitchYawRollRotationDegrees( 10.f, 90.f, 0.f );
	UpdateCameraProjection( m_worldCamera );
	g_theRenderer->DisableFog();

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	m_theSun.direction = Vec3( -1.f, -1.f, -1.f ).GetNormalize();
	m_theSun.position = Vec3( 10.f, 10.f, 10.f );
	m_theSun.color = Rgba8::WHITE.GetValuesAsFractionsVec3();//Rgba8(255, 184, 19).GetValuesAsFractionsVec3();
	m_theSun.intensity = 1.f;

	g_theRenderer->SetAmbientColor( Rgba8::BLACK );
	g_theRenderer->SetAmbientIntensity( 0.f );

	uint samples = 256;
	Vec2 dimensions = Vec2( 64.f, 64.f );
	float wind = 37.f;
	//m_DFTWaveSimulation = new DFTWaveSimulation( dimensions, samples, wind );
	LoadSimulationFromXML( "Test.xml" );
	//CreateNewFFTSimulation( samples, dimensions, wind );
	//m_FFTWaveSimulation->SetPosition( Vec3( dimensions.x, 0.f, 0.f ) );
// 	for( int i = 0; i < -1; ++i )
// 	{
// 		Vec2 randomDirection = g_RNG->RollRandomDirection2D();
// 		float randomWaveLength = g_RNG->RollRandomFloatInRange( 0.5f, 7.f ) * 0.1f;
// 		float randomAmplitude = g_RNG->RollRandomFloatInRange( 0.1f, 1.f ) * 0.1f;
// 		float randomPhase = g_RNG->RollRandomFloatInRange( 0.0f, 5.f );
// 
// 		m_waveSimulation->AddWave( new Wave( randomDirection, randomWaveLength, randomAmplitude, randomPhase ) );
// 	}

	m_testCube = new WaterObject( Vec3( 1.f, 1.f, 1.f ), Vec3( 0.f, 0.f, 0.f ) );

	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	m_test				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Grid.png" );

	m_testShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/WorldOpaque.hlsl" );

	m_testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );

	//Texture* skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/skybox_flipped.png" );
	Texture* skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/miramar_large_flipped.png" );
	m_skyBox = new TextureCube( g_theRenderer );
	m_skyBox->MakeFromImage( *skyboxTexture );

	std::vector<uint> skyBoxIndex;
	std::vector<Vertex_PCUTBN> skyBoxVerts;
	float skyboxHalfSize = 0.5f;
	AddTestCubeToIndexVertexArray( skyBoxVerts, skyBoxIndex, AABB3( Vec3( -skyboxHalfSize ), Vec3( skyboxHalfSize ) ), Rgba8::WHITE );
	m_skyCube = new GPUMesh( g_theRenderer, skyBoxVerts, skyBoxIndex );
	
	CreateTerrainFromImage( "Data/Images/Terrain3.png", Vec2( 50.f, 50.f ), -10.f, 1.f );
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

	delete m_FFTWaveSimulation;
	m_FFTWaveSimulation = nullptr;
	
	delete m_DFTWaveSimulation;
	m_DFTWaveSimulation = nullptr;

	delete m_skyBox;
	m_skyBox = nullptr;

	delete m_skyCube;
	m_skyCube = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render()
{
	UpdateCameraView( m_worldCamera );

	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );

	//RenderSkyBox
	g_theRenderer->BindSampler( g_theRenderer->m_samplerLinear );
	g_theRenderer->SetCullMode( CULL_MODE_FRONT );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_ALWAYS, false );
	
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );//Mat44::CreateTranslationXYZ( m_worldCamera->GetPosition() ) );
	g_theRenderer->BindTexture( m_skyBox );
	g_theRenderer->BindShaderByPath( "Data/Shaders/Skybox.hlsl" );
	g_theRenderer->DrawMesh( m_skyCube );
	
	g_theRenderer->BindSampler( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_NONE );
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
void Game::Update()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	if( !g_theConsole->IsOpen() )
	{
		m_testCube->Update();
		UpdateFromInput( deltaSeconds );
		//m_DFTWaveSimulation->Simulate();
		//m_FFTWaveSimulation->m_iWave->AddWaterObject( m_testCube );
		m_FFTWaveSimulation->Simulate();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawWorldBasis() const
{
	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 cameraTransformMatrix = Mat44::CreateTranslationXYZ( m_worldCamera->GetPosition() );
	cameraTransformMatrix.RotateZDegrees( m_worldCamera->GetYawDegrees() );
	cameraTransformMatrix.RotateYDegrees( m_worldCamera->GetPitchDegrees() );
	cameraTransformMatrix.RotateXDegrees( m_worldCamera->GetRollDegrees() );
	cameraTransformMatrix.ScaleNonUniform3D( m_worldCamera->GetScale() );

	Vec3 compasStartPosition = m_worldCamera->GetPosition();
	compasStartPosition += cameraTransformMatrix.TransformVector3D( Vec3( 0.1f, 0.085f, -0.045f ) );

	Mat44 cameraBasisMatrix = Mat44::CreateUniformScaleXYZ( 0.01f );
	cameraBasisMatrix.SetTranslation3D( compasStartPosition );
	DebugAddWorldBasis( cameraBasisMatrix, 0.f, DEBUG_RENDER_ALWAYS );
	
	DebugAddWorldArrow( m_theSun.position, m_theSun.position + m_theSun.direction, Rgba8::WHITE, 0.f, DEBUG_RENDER_ALWAYS );
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawTerrain() const
{
	g_theRenderer->BindMaterialByPath( "Data/Shaders/Terrain.material" );
	g_theRenderer->SetModelUBO( Mat44::CreateTranslationXYZ( Vec3( 0.f, 0.f, 0.f ) ) );
	g_theRenderer->DrawMesh( m_landMesh );
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawWater() const
{
	Texture* depthStencil = m_worldCamera->GetDepthStencilTarget();
	Texture* backBuffer = g_theRenderer->GetBackBuffer();
	
	Texture* backBufferCopy = g_theRenderer->AcquireRenderTargetMatching( backBuffer );
	Texture* depthStencilCopy = g_theRenderer->AcquireRenderTargetMatching( depthStencil );

	g_theRenderer->CopyTexture( backBufferCopy, backBuffer );
	g_theRenderer->CopyTexture( depthStencilCopy, depthStencil );

	g_theRenderer->BindMaterialTexture( 5, m_skyBox );
	g_theRenderer->BindMaterialTexture( 6, backBufferCopy );
	g_theRenderer->BindMaterialTexture( 7, depthStencilCopy );
	
	//m_DFTWaveSimulation->Render();
	m_FFTWaveSimulation->Render();
	
	g_theRenderer->ReleaseRenderTarget( backBufferCopy );
	g_theRenderer->ReleaseRenderTarget( depthStencilCopy );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderWorld() const
{
	if( g_isDebugDraw )
	{
		DrawWorldBasis();
	}

	DrawTerrain();
	DrawWater();
	
	//m_testCube->Render();
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( !g_isDebugDraw )
		return;

	const float textHeight = 0.15f;
	const float paddingFromLeft = 0.015f;
	const float paddingFromTop = 0.05f;

	std::vector<ColorString> runtimeStrings;
	std::vector<ColorString> precomputeStrings;
	std::vector<Vertex_PCU> textVerts;

	float fps = 1.f / static_cast<float>( m_gameClock->GetLastDeltaSeconds() * m_gameClock->GetScale() );

	Transform cameraTransform = m_worldCamera->GetTransform();
	Vec3 cameraPosition = cameraTransform.GetPosition();
	Vec3 cameraRotationPitchYawRollDegrees = cameraTransform.GetRotationPitchYawRollDegrees();
	Mat44 cameraView = m_worldCamera->GetViewMatrix();
	MatrixInvertOrthoNormal( cameraView );

	Wave* selectedWave = m_FFTWaveSimulation->GetWaveAtIndex( m_selectedWaveIndex );

	runtimeStrings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "FPS: %.2f", fps ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Wave Simulation: FFT" ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Number of Tiles: %i", m_FFTWaveSimulation->GetNumTiles() * m_FFTWaveSimulation->GetNumTiles() ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[I] - iWave: %s", ( m_FFTWaveSimulation->IsIWaveEnabled() ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[F] - Wire Frame: %s", ( m_FFTWaveSimulation->IsWireFrameEnabled() ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[C,V] - Choppiness: %.2f", m_FFTWaveSimulation->GetChoppyWaterValue() ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[Z,X] - Time Factor: %.0f", m_FFTWaveSimulation->GetTimeFactor() ) ) );


// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Data In: %.4f(ms)", m_FFTWaveSimulation->m_simulateTimer.GetAvgTimeMilliseconds() ) ) );
// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "FFT computation: %.4f(ms)", m_FFTWaveSimulation->m_pointCalculationTimer.GetAvgTimeMilliseconds() ) ) );
// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Data Out: %.4f(ms)", m_FFTWaveSimulation->m_fftTimer.GetAvgTimeMilliseconds() ) ) );

	Vec2 const& simDimensions = m_FFTWaveSimulation->GetGridDimensions();
	Vec2 const& windDir = m_FFTWaveSimulation->GetWindDirection();

	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[F2] - Recalculate Values <current>(new)" ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[Y,U] - Samples: %i(%i)", m_FFTWaveSimulation->GetNumSamples(), m_tempSamples ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[-,+] - Dimensions: %.2f(%.2f), %.2f(%.2f)", simDimensions.x, m_tempDimensions.x, simDimensions.y, m_tempDimensions.y ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[H,J] - Global Wave Amplitude: %.4f(%.4f)", m_FFTWaveSimulation->GetAConstant(), m_tempGloabalWaveAmp ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[K,L] - Wave Suppression Height: %.4f(%.4f)", m_FFTWaveSimulation->GetWaveSuppression(), m_tempWaveSuppression ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[N,M] - Wind Speed: %.2f(%.2f)", m_FFTWaveSimulation->GetWindSpeed(), m_tempWindSpeed ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[<,>] - Wind Direction: %.2f(%.2f), %.2f(%.2f)", windDir.x, m_tempWindDir.x, windDir.y, m_tempWindDir.y ) ) );


	if( selectedWave != nullptr )
	{
		runtimeStrings.push_back( ColorString( Rgba8::BLUE,	Stringf( " " ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[LEFT, RIGHT] - Wave: %i", m_selectedWaveIndex ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[UP, DOWN]    - Direction: ( %.2f, %.2f )", selectedWave->m_directionNormal.x, selectedWave->m_directionNormal.y ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[{, }]        - Size:      %.2f", selectedWave->m_directionNormal.GetLength() ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[-, +]        - Amplitude: %.2f", selectedWave->m_amplitude ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[;, ']        - Phase:     %.2f", selectedWave->m_phase ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "              - Frequency: %.2f", selectedWave->m_frequency ) ) );
		runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "              - Magnitude: %.2f", selectedWave->m_magnitude ) ) );
	}

	Vec3 cameraTopRight = m_UICamera->GetOrthoTopRight();
	Vec2 cameraTopRightXY = Vec2( cameraTopRight.x, cameraTopRight.y );
	Vec2 cameraTopLeft = Vec2( m_UICamera->GetOrthoBottomLeft().x, cameraTopRight.y );
	Vec2 leftTextStartPos = cameraTopLeft + Vec2( paddingFromLeft, -paddingFromTop - textHeight );

	Vec2 longestStingDim = g_devConsoleFont->GetDimensionsForText2D( textHeight, precomputeStrings[4].m_text );
	Vec2 rightTextStartPos = cameraTopRightXY + Vec2( -longestStingDim.x - paddingFromLeft, -paddingFromTop - textHeight );

	for( int stringIndex = 0; stringIndex < runtimeStrings.size(); ++stringIndex )
	{
		ColorString& coloredString = runtimeStrings[ stringIndex ];
		g_devConsoleFont->AddVertsForText2D( textVerts, leftTextStartPos, textHeight, coloredString.m_text, coloredString.m_color );
		leftTextStartPos -= Vec2( 0.f, textHeight + paddingFromTop );
	}

	for( int precomputeStringIndex = 0; precomputeStringIndex < precomputeStrings.size(); ++precomputeStringIndex )
	{
		ColorString& coloredString = precomputeStrings[ precomputeStringIndex ];
		g_devConsoleFont->AddVertsForText2D( textVerts, rightTextStartPos, textHeight, coloredString.m_text, coloredString.m_color );
		rightTextStartPos -= Vec2( 0.f, textHeight + paddingFromTop );
	}

	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->DrawVertexArray( textVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::EnableLightsForRendering() const
{
// 	for( unsigned int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
// 	{
// 	}
	g_theRenderer->EnableLight( 0, m_theSun );
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
void Game::CreateNewFFTSimulation( int samples, Vec2 const& dimensions, float windSpeed )
{
	if( m_FFTWaveSimulation != nullptr )
	{
		delete m_FFTWaveSimulation;
		m_FFTWaveSimulation = nullptr;
	}
	m_FFTWaveSimulation = new FFTWaveSimulation( dimensions, samples, windSpeed );
	m_FFTWaveSimulation->SetTilingDimensions( 1 );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadSimulationFromXML( char const* filepath )
{
	std::string simulationFolderPath = "Data/SimulationSettings/";
	if( m_FFTWaveSimulation != nullptr )
	{
		delete m_FFTWaveSimulation;
		m_FFTWaveSimulation = nullptr;
	}

	m_FFTWaveSimulation = dynamic_cast<FFTWaveSimulation*>( WaveSimulation::CreateWaveSimulation( simulationFolderPath + filepath ) );
	m_currentXML = filepath;

	SetTempValues();
}


//---------------------------------------------------------------------------------------------------------
void Game::ReloadCurrentXML()
{
	LoadSimulationFromXML( m_currentXML );
}


//---------------------------------------------------------------------------------------------------------
void Game::SetTempValues()
{
	m_tempSamples = m_FFTWaveSimulation->GetNumSamples();
	m_tempDimensions = m_FFTWaveSimulation->GetGridDimensions();
	m_tempWindDir = m_FFTWaveSimulation->GetWindDirection();
	m_tempWindSpeed = m_FFTWaveSimulation->GetWindSpeed();
	m_tempWaveSuppression = m_FFTWaveSimulation->GetWaveSuppression();
	m_tempGloabalWaveAmp = m_FFTWaveSimulation->GetAConstant();
}


//---------------------------------------------------------------------------------------------------------
void Game::CreateTerrainFromImage( char const* filepath, Vec2 const& meshDimensions, float minHeight, float maxHeight )
{
	Image terrainImage = Image( filepath );
	IntVec2 terrainDimensions = terrainImage.GetDimensions() - IntVec2( 1, 1 );

	std::vector<Vertex_PCUTBN> terrainVerts;

	Vec2 halfDimensions = meshDimensions * 0.5f;
	float xMin = -halfDimensions.x;
	float yMin = -halfDimensions.y;

	float xStepAmount = meshDimensions.x / static_cast<float>( terrainDimensions.x );
	float yStepAmount = meshDimensions.y / static_cast<float>( terrainDimensions.y );

	Vec3 xStepDisplacement = Vec3( xStepAmount, 0.f, 0.f );
	Vec3 yStepDisplacement = Vec3( 0.f, yStepAmount, 0.f );

	float currentY = yMin;
	for( unsigned int yStep = 0; yStep < terrainDimensions.y + 1; ++yStep )
	{
		float currentX = xMin;
		for( unsigned int xStep = 0; xStep < terrainDimensions.x + 1; ++xStep )
		{
			Vec3 currentPosition = Vec3::ZERO;
			currentPosition.x += currentX;
			currentPosition.y += currentY;

			Rgba8 texelColorAtVert = terrainImage.GetTexelColor( xStep, terrainDimensions.y - yStep );
			Vec4 colorPercents = texelColorAtVert.GetValuesAsFractions();
			currentPosition.z = Lerp( minHeight, maxHeight, colorPercents.w );

			float u = RangeMapFloat( xMin, halfDimensions.x, 0.f, 1.f, currentX );
			float v = RangeMapFloat( yMin, halfDimensions.y, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v  );

			terrainVerts.push_back( Vertex_PCUTBN( currentPosition, Rgba8::WHITE, Vec3::UNIT_POSITIVE_X, Vec3::UNIT_POSITIVE_Y, Vec3::UNIT_POSITIVE_Z, uv ) );
			currentX += xStepAmount;
		}
		currentY += yStepAmount;
	}

	std::vector<uint> indices;
	uint xSteps = terrainDimensions.x;
	uint ySteps = terrainDimensions.y;

	for( unsigned int yIndex = 0; yIndex < ySteps; ++yIndex )
	{
		for( unsigned int xIndex = 0; xIndex < xSteps; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( ( xSteps + 1 ) * yIndex);

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + xSteps + 1;
			unsigned int topRight = currentVertIndex + xSteps + 2;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}

	MeshGenerateNormals( terrainVerts, indices );
	m_landMesh = new GPUMesh( g_theRenderer, terrainVerts, indices ); 
}


//---------------------------------------------------------------------------------------------------------
void Game::GenerateTerrainVerts( GPUMesh* meshToModify, IntVec2 const& vertDimensions, Vec2 const& dimensions, float minHeight, float maxHeight )
{
	std::vector<Vertex_PCUTBN> terrainVerts;
	
	Vec2 halfDimensions = dimensions * 0.5f;

	float xMin = -halfDimensions.x;
	float yMin = -halfDimensions.y;

	float xStepAmount = dimensions.x / static_cast<float>( vertDimensions.x );
	float yStepAmount = dimensions.y / static_cast<float>( vertDimensions.y );

	Vec3 xStepDisplacement = Vec3( xStepAmount, 0.f, 0.f );
	Vec3 yStepDisplacement = Vec3( 0.f, yStepAmount, 0.f );

	float currentY = yMin;
	for( unsigned int yStep = 0; yStep < vertDimensions.y + 1; ++yStep )
	{
		float currentX = xMin;
		for( unsigned int xStep = 0; xStep < vertDimensions.x + 1; ++xStep )
		{
			Vec3 currentPosition = Vec3::ZERO;
			currentPosition.x += currentX;// * xStepDisplacement;
			currentPosition.y += currentY;// * yStepDisplacement;

			float percent = currentPosition.GetLength() / halfDimensions.GetLength();
			SmoothStep3( percent );
			currentPosition.z = Lerp( minHeight, maxHeight, percent );

			float u = RangeMapFloat( xMin, halfDimensions.x, 0.f, 1.f, currentX );
			float v = RangeMapFloat( yMin, halfDimensions.y, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v  );

			terrainVerts.push_back( Vertex_PCUTBN( currentPosition, Rgba8::WHITE, Vec3::UNIT_POSITIVE_X, Vec3::UNIT_POSITIVE_Y, Vec3::UNIT_POSITIVE_Z, uv ) );
			currentX += xStepAmount;
		}
		currentY += yStepAmount;
	}

	std::vector<uint> indices;
	uint xSteps = vertDimensions.x;
	uint ySteps = vertDimensions.y;

	for( unsigned int yIndex = 0; yIndex < ySteps; ++yIndex )
	{
		for( unsigned int xIndex = 0; xIndex < xSteps; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( ( xSteps + 1 ) * yIndex);

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + xSteps + 1;
			unsigned int topRight = currentVertIndex + xSteps + 2;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}

	meshToModify->UpdateVerticies( static_cast<uint>( terrainVerts.size() ), &terrainVerts[0] );
	meshToModify->UpdateIndicies( static_cast<uint>( indices.size() ), &indices[0] );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadCurrentTempValues()
{
	bool tempWireFrame = m_FFTWaveSimulation->IsWireFrameEnabled();
	bool tempIWave = m_FFTWaveSimulation->IsIWaveEnabled();
	float tempChoppiness = m_FFTWaveSimulation->GetChoppyWaterValue();
	uint tilingSize = m_FFTWaveSimulation->GetNumTiles();

	delete m_FFTWaveSimulation;
	m_FFTWaveSimulation = nullptr;

	m_FFTWaveSimulation = new FFTWaveSimulation( m_tempDimensions, m_tempSamples, m_tempWindSpeed, m_tempWindDir, m_tempGloabalWaveAmp, m_tempWaveSuppression );
	m_FFTWaveSimulation->SetIsWireFrame( tempWireFrame );
	m_FFTWaveSimulation->SetIWaveEnabled( tempIWave );
	m_FFTWaveSimulation->SetChoppyWaterValue( tempChoppiness );
	m_FFTWaveSimulation->SetTilingDimensions( tilingSize );
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
void Game::UpdateSimulationFromInput()
{
	const float updateSpeed = 1.f;
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	if( g_theInput->WasKeyJustPressed( 'I' ) )
	{
		m_FFTWaveSimulation->SetIWaveEnabled( !m_FFTWaveSimulation->IsIWaveEnabled() );
	}

	if( g_theInput->IsKeyPressed( 'C' ) )
	{
		m_FFTWaveSimulation->AddChoppyWaterValue( -updateSpeed * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( 'V' ) )
	{
		m_FFTWaveSimulation->AddChoppyWaterValue( updateSpeed * deltaSeconds );
	}

	if( g_theInput->WasKeyJustPressed( 'M' ) )
	{
		FFTWaveSimulation* fft = dynamic_cast<FFTWaveSimulation*>( m_FFTWaveSimulation );
		fft->m_iWave->AddSource( 32 * 16 + 16, -0.5f );
		fft->m_iWave->AddSource( 32 * 16 + 15, -0.5f );
		fft->m_iWave->AddSource( 32 * 16 + 17, -0.5f );
		fft->m_iWave->AddSource( 32 * 17 + 16, -0.5f );
		fft->m_iWave->AddSource( 32 * 15 + 16, -0.5f );
	}

	if( g_theInput->WasKeyJustPressed( 'F' ) )
	{
		//m_DFTWaveSimulation->ToggleWireFrameMode();
		m_FFTWaveSimulation->ToggleWireFrameMode();
	}

	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		//m_DFTWaveSimulation->ToggleSimulationClockPause();
		m_FFTWaveSimulation->ToggleSimulationClockPause();
	}

	if( g_theInput->WasKeyJustPressed( 'U' ) )
	{
		IncreaseSamples();
	}
	if( g_theInput->WasKeyJustPressed( 'Y' ) )
	{
		DecreaseSamples();
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_PLUS ) )
	{
		AddUniformDimensions( updateSpeed * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_MINUS ) )
	{
		AddUniformDimensions( -updateSpeed * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( 'H' ) )
	{
		AddGlobalWaveAmp( -0.01f * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( 'J' ) )
	{
		AddGlobalWaveAmp( 0.01f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( 'K' ) )
	{
		AddWaveSuppression( -0.1f * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( 'L' ) )
	{
		AddWaveSuppression( 0.1f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( 'N' ) )
	{
		AddWindSpeed( -updateSpeed * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( 'M' ) )
	{
		AddWindSpeed( updateSpeed * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_COMMA ) )
	{
		RotateWindDirByDegrees( 30.f * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_PERIOD ) )
	{
		RotateWindDirByDegrees( 30.f * deltaSeconds );
	}

	if( g_theInput->WasKeyJustPressed( 'Z' ) )
	{
		m_FFTWaveSimulation->AddTimeFactor( -1.f );
	}
	if( g_theInput->WasKeyJustPressed( 'X' ) )
	{
		m_FFTWaveSimulation->AddTimeFactor( 1.f );
	}

	int numWaves = m_FFTWaveSimulation->GetNumWaves();
	if( numWaves <= 0 )
		return;


	//Change Wave
	if( g_theInput->WasKeyJustPressed( KEY_CODE_LEFT_ARROW ) )
	{
		--m_selectedWaveIndex;
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		++m_selectedWaveIndex;
	}
	Clamp( m_selectedWaveIndex, 0, numWaves - 1 );
	Wave* waveToModify = m_DFTWaveSimulation->GetWaveAtIndex( m_selectedWaveIndex );

	//Change Direction
	if( g_theInput->IsKeyPressed( KEY_CODE_UP_ARROW ) )
	{
		waveToModify->RotateDirectionDegrees( updateSpeed * 3.f * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_DOWN_ARROW ) )
	{
		waveToModify->RotateDirectionDegrees( -updateSpeed * 3.f * deltaSeconds );
	}

	//Change Amplitude
	if( g_theInput->IsKeyPressed( KEY_CODE_PLUS ) )
	{
		waveToModify->AddAmplitude( updateSpeed * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_MINUS ) )
	{
		waveToModify->AddAmplitude( -updateSpeed * deltaSeconds );
	}

	//Change Frequency
	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_BRACKET ) )
	{
		waveToModify->AddMagnitude( updateSpeed * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_BRACKET ) )
	{
		waveToModify->AddMagnitude( -updateSpeed * deltaSeconds );
	}

	//Change Phase
	if( g_theInput->IsKeyPressed( KEY_CODE_APOSTROPHE ) )
	{
		waveToModify->AddPhase( updateSpeed * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_SEMICOLON ) )
	{
		waveToModify->AddPhase( -updateSpeed * deltaSeconds );
	}

	//Change Wave Size
	if( g_theInput->IsKeyPressed( KEY_CODE_COMMA ) )
	{
		//Increase Amplitude
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_PERIOD ) )
	{
		//Decrease Amplitude
	}

}


//---------------------------------------------------------------------------------------------------------
void Game::IncreaseSamples()
{
	m_tempSamples <<= 1;
	if( m_tempSamples > 2048 )
	{
		m_tempSamples = 2048;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DecreaseSamples()
{
	m_tempSamples >>= 1;
	if( m_tempSamples < 4 )
	{
		m_tempSamples = 4;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::AddUniformDimensions( float dimensionsToAdd )
{
	m_tempDimensions.x += dimensionsToAdd;
	m_tempDimensions.y += dimensionsToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddGlobalWaveAmp( float ampToAdd )
{
	m_tempGloabalWaveAmp += ampToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddWaveSuppression( float suppressionToAdd )
{
	m_tempWaveSuppression += suppressionToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddWindSpeed( float windSpeedToAdd )
{
	m_tempWindSpeed += windSpeedToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Game::RotateWindDirByDegrees( float degreesToRotateBy )
{
	m_tempWindDir.RotateDegrees( degreesToRotateBy );
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
void Game::UpdateFromInput( float deltaSeconds )
{
	UpdateBasedOnMouseMovement();

	MoveWorldCamera( deltaSeconds );

	UpdateSimulationFromInput();

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
		ReloadCurrentXML();
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F2 ) )
	{
		LoadCurrentTempValues();
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F11 ) )
	{
		g_isDebugDraw = !g_isDebugDraw;
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

// 	Vec3 cameraForwardXY0 = Vec3( CosDegrees( m_worldCamera->GetYawDegrees() ), SinDegrees( m_worldCamera->GetYawDegrees() ), 0.f );
// 	Vec3 cameraLeftXY0 = Vec3( -SinDegrees( m_worldCamera->GetYawDegrees() ), CosDegrees( m_worldCamera->GetYawDegrees() ), 0.f );
	Mat44 cameraView = m_worldCamera->GetViewMatrix();
	MatrixInvertOrthoNormal( cameraView );
	Vec3 cameraForward = cameraView.TransformVector3D( Vec3::UNIT_POSITIVE_X );
	Vec3 cameraLeft = cameraView.TransformVector3D( Vec3::UNIT_POSITIVE_Y );
	Vec3 cameraTranslation = ( cameraForward * forwardMoveAmount ) + ( cameraLeft * leftMoveAmount );
	cameraTranslation.z += upMoveAmount;
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
void Game::create_new_fft_simulation( EventArgs* args )
{
	int		defaultSamples		= 32;
	float	defaultWindSpeed	= 37.f;
	Vec2	defaultDim			= Vec2( 32.f, 32.f );

	Vec2	dimensions	= args->GetValue( "dimensions", defaultDim );
	int		samples		= args->GetValue( "samples", defaultSamples );
	float	windSpeed	= args->GetValue( "windSpeed", defaultWindSpeed );

	g_theConsole->PrintString( Rgba8::YELLOW, "Creating New FFT Simulation: with %i samples and dimensions %.2fx%.2f", samples, dimensions.x, dimensions.y );
	g_theConsole->PrintString( Rgba8::YELLOW, "   Samples:    %i", samples );
	g_theConsole->PrintString( Rgba8::YELLOW, "   Dimensions: %.2f x %.2f", dimensions.x, dimensions.y );
	g_theConsole->PrintString( Rgba8::YELLOW, "   Wind Speed: %.2f", windSpeed );

	CreateNewFFTSimulation( samples, dimensions, windSpeed );
}

//---------------------------------------------------------------------------------------------------------
void Game::fft_from_xml( EventArgs* args )
{
	std::string filepath = args->GetValue( "file", "Test.xml" );

	LoadSimulationFromXML( filepath.c_str() );
}

//---------------------------------------------------------------------------------------------------------
void Game::PlayTestSound()
{
	float volume	= g_RNG->RollRandomFloatInRange(  0.5f,  1.0f );
	float balance	= g_RNG->RollRandomFloatInRange( -1.0f,  1.0f );
	float speed		= g_RNG->RollRandomFloatInRange(  0.5f,  2.0f );
	g_theAudio->PlaySound( m_testSound, false, volume, balance, speed );
}