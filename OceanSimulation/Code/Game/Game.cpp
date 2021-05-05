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
	float nearPlane = -0.09f;
	float farPlane = -100.f;
	m_worldCamera->SetProjectionPerspective( 60.f, nearPlane, farPlane );
	m_worldCamera->SetDepthStencilTarget( g_theRenderer->m_defaultDepthStencil );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, Rgba8::WHITE, 1.0f, 0 );
	m_worldCamera->SetPosition( Vec3( -8.f, -8.f, 1.f ) );
	m_worldCamera->SetPitchYawRollRotationDegrees( 10.f, 90.f, 0.f );
	UpdateCameraProjection( m_worldCamera );
	g_theRenderer->DisableFog();

	m_UICamera = new Camera( g_theRenderer );
	m_UICamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	m_theSun.direction = Vec3( -1.f, -1.f, -1.f ).GetNormalize();
	m_theSun.position = Vec3( 10.f, 10.f, 10.f );
	m_theSun.color = Rgba8::WHITE.GetValuesAsFractionsVec3();
	m_theSun.intensity = 1.f;

	g_theRenderer->SetAmbientLight( m_ambientColor, m_ambientIntensity );

	m_crate = new WaterObject( Vec3( 1.f, 1.f, 1.f ), Vec3( 0.f, 0.f, 0.f ) );

	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	//Texture* skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/skybox_flipped.png" );
	Texture* skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/miramar_large_flipped.png" );
	m_skyBox = new TextureCube( g_theRenderer );
	m_skyBox->MakeFromImage( *skyboxTexture );

	Texture* noSkyboxTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Grey.png" );
	m_noSkyBox = new TextureCube( g_theRenderer );
	m_noSkyBox->MakeFromImage( *noSkyboxTexture );

	m_skyBoxTexture = m_isSkyBox ? m_skyBox : m_noSkyBox;


	std::vector<uint> skyBoxIndex;
	std::vector<Vertex_PCUTBN> skyBoxVerts;
	float skyboxHalfSize = 0.5f;
	AddTestCubeToIndexVertexArray( skyBoxVerts, skyBoxIndex, AABB3( Vec3( -skyboxHalfSize ), Vec3( skyboxHalfSize ) ), Rgba8::WHITE );
	m_skyCube = new GPUMesh( g_theRenderer, skyBoxVerts, skyBoxIndex );
	
	LoadMaterials();
	LoadSimulationSettings();
	LoadTerrains();
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

	delete m_skyBox;
	m_skyBox = nullptr;

	delete m_skyCube;
	m_skyCube = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::Render()
{
	UpdateCameraView( m_worldCamera );

	Texture* backBuffer = g_theRenderer->GetBackBuffer();
	Texture* refractionMask = g_theRenderer->AcquireRenderTargetMatching( backBuffer );
	Texture* colorOutput = g_theRenderer->AcquireRenderTargetMatching( backBuffer );

	m_worldCamera->SetColorTarget( 0, colorOutput );
	m_worldCamera->SetColorTarget( 1, refractionMask );

	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );
	Mat44 projection = m_worldCamera->GetProjectionMatrix();
	MatrixInvert( projection );
	m_waterInfo.WORLD_INVERSE_PROJECTION = projection; 

	//RenderSkyBox
	g_theRenderer->BindSampler( g_theRenderer->m_samplerLinear );
	g_theRenderer->SetCullMode( CULL_MODE_FRONT );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_ALWAYS, false );
	
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->BindTexture( m_skyBoxTexture );
	g_theRenderer->BindShaderByPath( "Data/Shaders/Skybox.hlsl" );
	g_theRenderer->DrawMesh( m_skyCube );
	
	g_theRenderer->BindSampler( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_NONE );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LESS, true );


	g_theRenderer->DisableAllLights();
	g_theRenderer->SetAmbientLight( m_ambientColor, m_ambientIntensity );
	EnableLightsForRendering();
	
	//---------------------------------------------------------------------------------------------------------
	//Refraction map
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShaderByPath( "Data/Shaders/RefractionObject.hlsl" );
	g_theRenderer->DrawMesh( m_selectedTerrain );
	if( m_updateCrate )
	{
		m_crate->Render();
	}
	g_theRenderer->BindShaderByPath( "Data/Shaders/RefractionStencil.hlsl" );
	m_FFTWaveSimulation->DrawMesh();



	//---------------------------------------------------------------------------------------------------------
	// Real Render
	g_theRenderer->SetDepthTest( COMPARE_FUNC_GEQUAL, true );
	g_theRenderer->ClearDepth( m_worldCamera->GetDepthStencilTarget(), m_worldCamera->GetClearDepth() );
	RenderWorld();

	// Get depths
	Texture* depthStencil = m_worldCamera->GetDepthStencilTarget();
	Texture* depthStencilCopy = g_theRenderer->AcquireRenderTargetMatching( depthStencil );
	g_theRenderer->CopyTexture( depthStencilCopy, depthStencil );

	//---------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );

	g_theRenderer->ReleaseRenderTarget( refractionMask );


	//---------------------------------------------------------------------------------------------------------
	if( m_isUnderWater && m_showUnderwaterEffect )
	{
		Material* underwaterMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Underwater.material" );
		underwaterMaterial->SetData( m_waterInfo );
		underwaterMaterial->SetDiffuseTexture( colorOutput );
		underwaterMaterial->AddMaterialTexture( 0, depthStencil );
		g_theRenderer->ApplyFullscreenEffect( colorOutput, backBuffer, underwaterMaterial );
	}
	else
	{
		g_theRenderer->CopyTexture( backBuffer, colorOutput );
	}

	g_theRenderer->ReleaseRenderTarget( colorOutput );
	g_theRenderer->ReleaseRenderTarget( depthStencilCopy );


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
		UpdateFromInput( deltaSeconds );

		if( m_updateCrate )
		{
			m_crate->Update( m_worldCamera );
			if( m_FFTWaveSimulation->IsIWaveEnabled() )
			{
				m_FFTWaveSimulation->m_iWave->AddWaterObject( m_crate );
			}
		}

		m_FFTWaveSimulation->Simulate();

		if( m_updateCrate )
		{
			m_FFTWaveSimulation->TransformByAverageWater( m_crate );
		}

		if( m_showUnderwaterEffect )
		{
			m_isUnderWater = m_FFTWaveSimulation->IsPointUnderWater( m_worldCamera->GetPosition() );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
float Game::GetDeltaSeconds()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );
	return deltaSeconds;
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
	g_theRenderer->DrawMesh( m_selectedTerrain );
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawWater() const
{
	Texture* depthStencil = m_worldCamera->GetDepthStencilTarget();
	Texture* colorTarget = m_worldCamera->GetColorTarget( 0 );
	Texture* refractionStencil = m_worldCamera->GetColorTarget( 1 );
	
	Texture* backBufferCopy = g_theRenderer->AcquireRenderTargetMatching( colorTarget );
	Texture* refractionStencilCopy = g_theRenderer->AcquireRenderTargetMatching( refractionStencil );
	Texture* depthStencilCopy = g_theRenderer->AcquireRenderTargetMatching( depthStencil );

	g_theRenderer->CopyTexture( backBufferCopy, colorTarget );
	g_theRenderer->CopyTexture( refractionStencilCopy, refractionStencil );
	g_theRenderer->CopyTexture( depthStencilCopy, depthStencil );

	g_theRenderer->BindMaterialTexture( 4, refractionStencilCopy );
	g_theRenderer->BindMaterialTexture( 5, m_skyBoxTexture );
	g_theRenderer->BindMaterialTexture( 6, backBufferCopy );
	g_theRenderer->BindMaterialTexture( 7, depthStencilCopy );
	
	//m_DFTWaveSimulation->Render();
	Material* waterMaterial = m_selectedMaterial;
	waterMaterial->SetData( m_waterInfo );
	m_FFTWaveSimulation->Render( waterMaterial );

	g_theRenderer->ReleaseRenderTarget( refractionStencilCopy );
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
	g_theRenderer->BindMaterialByPath( "Data/Shaders/Lit.material" );
	g_theRenderer->BindTextureByPath( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	g_theRenderer->BindNormalTexture( nullptr );

	if( m_updateCrate )
	{
		m_crate->Render();
	}

	DrawWater();	
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( !m_isDebugText )
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

	runtimeStrings.push_back( ColorString( Rgba8::YELLOW,	Stringf( "FPS: %.2f", fps ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Wave Simulation: FFT" ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[F3]  - Sky Box: %s", ( m_isSkyBox ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[Z,X] - Terrain: %s", m_terrainNames[m_selectedTerrainIndex] ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[R,F] - Water Material: %s", m_waterMaterialNames[m_selectedMaterialIndex] ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[1,2] - Setting File: %s", m_simulationSettingNames[m_selectedSettingsIndex] ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[3]   - Floating Crate: %s", ( m_updateCrate ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[4]   - Create iWave Ripples (Power: %.2f)", m_iWaveSourcePower ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[5]   - Scrolling Normals: %s", ( m_scrollingNormals ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[I]   - iWave: %s", ( m_FFTWaveSimulation->IsIWaveEnabled() ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[U]   - Underwater: %s", ( m_showUnderwaterEffect ? "Enabled" : "Disabled" ) ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[N]   - Number of Tiles: %i", m_FFTWaveSimulation->GetNumTiles() * m_FFTWaveSimulation->GetNumTiles() ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[C]   - Choppiness: %.2f", m_FFTWaveSimulation->GetChoppyWaterValue() ) ) );
	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[T]   - Time Factor: %.0f", m_FFTWaveSimulation->GetTimeFactor() ) ) );


// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Data In: %.4f(ms)", m_FFTWaveSimulation->m_simulateTimer.GetAvgTimeMilliseconds() ) ) );
// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "FFT computation: %.4f(ms)", m_FFTWaveSimulation->m_pointCalculationTimer.GetAvgTimeMilliseconds() ) ) );
// 	runtimeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Data Out: %.4f(ms)", m_FFTWaveSimulation->m_fftTimer.GetAvgTimeMilliseconds() ) ) );

	Vec2 const& simDimensions = m_FFTWaveSimulation->GetGridDimensions();
	Vec2 const& windDir = m_FFTWaveSimulation->GetWindDirection();

	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[F1] - Reload Current XML File: %s", m_currentXML.c_str() ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[F2] - Recalculate Values <current>(new)" ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[Y]  - Samples: %i(%i)", m_FFTWaveSimulation->GetNumSamples(), m_tempSamples ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[G]  - Dimensions: %.2f(%.2f), %.2f(%.2f)", simDimensions.x, m_tempDimensions.x, simDimensions.y, m_tempDimensions.y ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[H]  - Global Wave Amplitude: %.4f(%.4f)", m_FFTWaveSimulation->GetAConstant(), m_tempGloabalWaveAmp ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[J]  - Wave Suppression Height: %.4f(%.4f)", m_FFTWaveSimulation->GetWaveSuppression(), m_tempWaveSuppression ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[K]  - Wind Speed: %.2f(%.2f)", m_FFTWaveSimulation->GetWindSpeed(), m_tempWindSpeed ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[L]  - Wind Direction: %.2f(%.2f), %.2f(%.2f)", windDir.x, m_tempWindDir.x, windDir.y, m_tempWindDir.y ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[V]  - Foam Depth: %.1f", m_waterInfo.FOAM_THICKNESS ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[B]  - Foaminess: %.1f", m_waterInfo.FOAMINESS ) ) );
	precomputeStrings.push_back( ColorString( Rgba8::WHITE,	Stringf( "[M]  - Max Depth: %.1f", m_waterInfo.MAX_DEPTH ) ) );


	Vec3 cameraTopRight = m_UICamera->GetOrthoTopRight();
	Vec2 cameraTopRightXY = Vec2( cameraTopRight.x, cameraTopRight.y );
	Vec2 cameraTopLeft = Vec2( m_UICamera->GetOrthoBottomLeft().x, cameraTopRight.y );
	Vec2 leftTextStartPos = cameraTopLeft + Vec2( paddingFromLeft, -paddingFromTop - textHeight );

	Vec2 longestStingDim = GetDimensionsForLongestColorSting( g_devConsoleFont, textHeight, precomputeStrings );
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
Vec2 Game::GetDimensionsForLongestColorSting( BitmapFont* font, float textHeight, std::vector<ColorString> const& strings ) const
{
	std::string longestString = GetLongestColorSting( strings );
	Vec2 textDim = font->GetDimensionsForText2D( textHeight, longestString );
	return textDim;
}


//---------------------------------------------------------------------------------------------------------
std::string Game::GetLongestColorSting( std::vector<ColorString> const& strings ) const
{
	std::string longestString = "";
	int longestStringLength = 0;

	for( int stringIndex = 0; stringIndex < strings.size(); ++stringIndex )
	{
		std::string currentString = strings[stringIndex].m_text;
		int currentStringLength = static_cast<int>( currentString.length() );
		if( currentStringLength > longestStringLength )
		{
			longestString = currentString;
			longestStringLength = currentStringLength;
		}
	}

	return longestString;
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

	WaveSimulation* createdWaveSimulation = WaveSimulation::CreateWaveSimulation( simulationFolderPath + filepath );
	if( createdWaveSimulation != nullptr )
	{
		m_FFTWaveSimulation = dynamic_cast<FFTWaveSimulation*>( createdWaveSimulation );
		m_currentXML = filepath;
	}
	else
	{
		g_theConsole->ErrorString( "Failed to load new WaveSimulation" );
	}

	SetTempValues();
}


//---------------------------------------------------------------------------------------------------------
void Game::ReloadCurrentXML()
{
	LoadSimulationFromXML( m_currentXML.c_str() );
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
void Game::AddIWaveSources()
{
	float power = m_iWaveSourcePower * -0.1f;
	FFTWaveSimulation* fft = dynamic_cast<FFTWaveSimulation*>( m_FFTWaveSimulation );
	int numSamples = fft->GetNumSamples();
	int halfSamples = numSamples / 2;
	int quaterSamples = halfSamples / 2;
	//Center
	fft->m_iWave->AddSource( numSamples * halfSamples + halfSamples,			power );
	fft->m_iWave->AddSource( numSamples * halfSamples + halfSamples - 1,		power );
	fft->m_iWave->AddSource( numSamples * halfSamples + halfSamples + 1,		power );
	fft->m_iWave->AddSource( numSamples * ( halfSamples + 1 ) + halfSamples,	power );
	fft->m_iWave->AddSource( numSamples * ( halfSamples - 1 ) + halfSamples,	power );

	//bottom Left
	fft->m_iWave->AddSource( 0, power );
	//fft->m_iWave->AddSource( 1, power );
	//fft->m_iWave->AddSource( numSamples, power );

	//qudrant
	fft->m_iWave->AddSource( quaterSamples + ( numSamples * quaterSamples ), power );
	fft->m_iWave->AddSource( numSamples - quaterSamples + ( numSamples * quaterSamples ), power );
	fft->m_iWave->AddSource( quaterSamples + numSamples * ( numSamples - quaterSamples ), power );
	fft->m_iWave->AddSource( numSamples - quaterSamples + numSamples * ( numSamples - quaterSamples ), power );
}


//---------------------------------------------------------------------------------------------------------
GPUMesh* Game::CreateTerrainFromImage( char const* filepath, Vec2 const& meshDimensions, float minHeight, float maxHeight )
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
	for( int yStep = 0; yStep < terrainDimensions.y + 1; ++yStep )
	{
		float currentX = xMin;
		for( int xStep = 0; xStep < terrainDimensions.x + 1; ++xStep )
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
	GPUMesh* landMesh = new GPUMesh( g_theRenderer, terrainVerts, indices );
	return landMesh;
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
	for( int yStep = 0; yStep < vertDimensions.y + 1; ++yStep )
	{
		float currentX = xMin;
		for( int xStep = 0; xStep < vertDimensions.x + 1; ++xStep )
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
void Game::LoadTerrains()
{
	const Vec2 terrainSize = Vec2( 51.6f, 51.6f );
	GPUMesh* bowlTerrain = CreateTerrainFromImage( "Data/Images/Terrain.png", terrainSize, -10.f, 1.f );
	GPUMesh* wallTerrain = CreateTerrainFromImage( "Data/Images/Terrain2.png", terrainSize, -10.f, 2.f );
	GPUMesh* islandTerrain = CreateTerrainFromImage( "Data/Images/Terrain3.png", terrainSize, -10.f, 1.f );
	GPUMesh* gradientTerrain = CreateTerrainFromImage( "Data/Images/Terrain4.png", terrainSize, -10.f, 1.f );

	AddTerrainByName( "Gradient",		gradientTerrain );
	AddTerrainByName( "Gradient Wall",	wallTerrain );
	AddTerrainByName( "Island",			islandTerrain );
	AddTerrainByName( "Bowl",			bowlTerrain );

	m_selectedTerrain = m_terrains[m_selectedTerrainIndex];
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadSimulationSettings()
{	
	AddSimulationSettingByName( "Still",	"Still.xml" );
	AddSimulationSettingByName( "Calm",		"Calm.xml" );
	AddSimulationSettingByName( "Stormy",	"Stormy.xml" );
	AddSimulationSettingByName( "Zeros",	"Zero.xml" );

	m_selectedSettings = m_simulationSettings[m_selectedSettingsIndex];
	LoadSimulationFromXML( m_selectedSettings );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadMaterials()
{
	Material* completeMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Water.material" );
	Material* wireframeMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Water_Wireframe.material" );
	Material* reflectionMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Water_Reflection.material" );
	Material* transmissionMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Water_Transmission.material" );
	Material* fresnelMaterial = g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Water_Fresnel.material" );

	AddWaterMaterialByName( "Complete", completeMaterial );
	AddWaterMaterialByName( "Wireframe", wireframeMaterial );
	AddWaterMaterialByName( "Reflection", reflectionMaterial );
	AddWaterMaterialByName( "Transmission and Refraction", transmissionMaterial );
	AddWaterMaterialByName( "Fresnel", fresnelMaterial );

	m_selectedMaterial = m_waterMaterials[m_selectedMaterialIndex];
}


//---------------------------------------------------------------------------------------------------------
void Game::AddTerrainByName( char const* name, GPUMesh* terrainMesh )
{
	m_terrains.push_back( terrainMesh );
	m_terrainNames.push_back( name );
}


//---------------------------------------------------------------------------------------------------------
void Game::AddSimulationSettingByName( char const* name, char const* simulationFileName )
{
	m_simulationSettings.push_back( simulationFileName );
	m_simulationSettingNames.push_back( name );
}


//---------------------------------------------------------------------------------------------------------
void Game::AddWaterMaterialByName( char const* name, Material* material )
{
	m_waterMaterials.push_back( material );
	m_waterMaterialNames.push_back( name );
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
	m_FFTWaveSimulation->CreateQuadTree();
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
	CycleSelectedSimulationSettings();
	CycleSelectedTerrain();
	CycleSelectedMaterial();

	if( g_theInput->IsKeyPressed( SELECT_NUM_TILES ) )
	{
		UpdateTilingSize();
	}
	if( g_theInput->IsKeyPressed( SELECT_CHOPPINES ) )
	{
		UpdateChoppiness();
	}
	if( g_theInput->IsKeyPressed( SELECT_TIME_FACTOR ) )
	{
		UpdateTimeFactor();
	}
	if( g_theInput->IsKeyPressed( SELECT_SAMPLES ) )
	{
		UpdateSamples();
	}
	if( g_theInput->IsKeyPressed( SELECT_DIMENSIONS ) )
	{
		UpdateDimensions();
	}
	if( g_theInput->IsKeyPressed( SELECT_GLOBAL_AMPLITUDE ) )
	{
		UpdateGlobalWaveAmplitude();
	}
	if( g_theInput->IsKeyPressed( SELECT_WAVE_SUPPRESS ) )
	{
		UpdateWaveSuppression();
	}
	if( g_theInput->IsKeyPressed( SELECT_WIND_SPEED ) )
	{
		UpdateWindSpeed();
	}
	if( g_theInput->IsKeyPressed( SELECT_WIND_DIRECTION ) )
	{
		UpdateWindDirection();
	}

	if( g_theInput->WasKeyJustPressed( 'I' ) )
	{
		m_FFTWaveSimulation->SetIWaveEnabled( !m_FFTWaveSimulation->IsIWaveEnabled() );
	}

	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		//m_DFTWaveSimulation->ToggleSimulationClockPause();
		m_FFTWaveSimulation->ToggleSimulationClockPause();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateWaterInfoFromInput()
{
	if( g_theInput->IsKeyPressed( SELECT_FOAMINESS ) )
	{
		UpdateFoaminess();
	}
	if( g_theInput->IsKeyPressed( SELECT_FOAM_DEPTH ) )
	{
		UpdateFoamDepth();
	}
	if( g_theInput->IsKeyPressed( SELECT_MAX_DEPTH ) )
	{
		UpdateMaxDepth();
	}
	if( g_theInput->WasKeyJustPressed( TOGGLE_SCROLLING_NORMALS_KEY ) )
	{
		m_scrollingNormals = !m_scrollingNormals;
		if( m_scrollingNormals )
		{
			m_waterInfo.NORMALS_SCROLL_SPEED = Vec2( 0.01f, 0.01f );
		}
		else
		{
			m_waterInfo.NORMALS_SCROLL_SPEED = Vec2( 0.0f, 0.0f );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleSelectedSimulationSettings()
{
	if( g_theInput->WasKeyJustPressed( CYCLE_SETTINGS_FORWARD ) )
	{
		m_selectedSettingsIndex++;
		if( m_selectedSettingsIndex >=  m_simulationSettings.size() )
		{
			m_selectedSettingsIndex = 0;
		}
		m_selectedSettings = m_simulationSettings[m_selectedSettingsIndex];
		LoadSimulationFromXML( m_selectedSettings );
	}

	if( g_theInput->WasKeyJustPressed( CYCLE_SETTINGS_BACKWARD ) )
	{
		m_selectedSettingsIndex--;
		if( m_selectedSettingsIndex < 0 )
		{
			m_selectedSettingsIndex = static_cast<int>( m_simulationSettings.size() ) - 1;
		}
		m_selectedSettings = m_simulationSettings[m_selectedSettingsIndex];
		LoadSimulationFromXML( m_selectedSettings );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleSelectedTerrain()
{
	if( g_theInput->WasKeyJustPressed( CYCLE_TERRAIN_FORWARD ) )
	{
		m_selectedTerrainIndex++;
		if( m_selectedTerrainIndex >=  m_terrains.size() )
		{
			m_selectedTerrainIndex = 0;
		}
		m_selectedTerrain = m_terrains[m_selectedTerrainIndex];
	}

	if( g_theInput->WasKeyJustPressed( CYCLE_TERRAIN_BACKWARD ) )
	{
		m_selectedTerrainIndex--;
		if( m_selectedTerrainIndex < 0 )
		{
			m_selectedTerrainIndex = static_cast<int>( m_terrains.size() ) - 1;
		}
		m_selectedTerrain = m_terrains[m_selectedTerrainIndex];
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleSelectedMaterial()
{
	if( g_theInput->WasKeyJustPressed( CYCLE_WATER_SHADERS_FORWARD ) )
	{
		m_selectedMaterialIndex++;
		if( m_selectedMaterialIndex >= m_waterMaterials.size() )
		{
			m_selectedMaterialIndex = 0;
		}
		m_selectedMaterial = m_waterMaterials[m_selectedMaterialIndex];
	}

	if( g_theInput->WasKeyJustPressed( CYCLE_WATER_SHADERS_BACKWARD ) )
	{
		m_selectedMaterialIndex--;
		if( m_selectedMaterialIndex < 0 )
		{
			m_selectedMaterialIndex = static_cast<int>( m_waterMaterials.size() ) - 1;
		}
		m_selectedMaterial = m_waterMaterials[m_selectedMaterialIndex];
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateTilingSize()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	if( scrollAmount != 0.f )
	{
		int direction = static_cast<int>( Signf( scrollAmount ) );
		int currentTilingDim = m_FFTWaveSimulation->GetNumTiles();

		currentTilingDim += direction * NUM_TILES_CHANGE;
		Clamp( currentTilingDim, 1, 5 );

		m_FFTWaveSimulation->SetTilingDimensions( currentTilingDim );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateChoppiness()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * CHOPINESS_CHANGE_PER_SCROLL;
	float currentChoppiness = m_FFTWaveSimulation->GetChoppyWaterValue();

	currentChoppiness += amountToChange;
	Clamp( currentChoppiness, -1.f, 1.f );

	m_FFTWaveSimulation->SetChoppyWaterValue( currentChoppiness );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateTimeFactor()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	if( scrollAmount != 0.f )
	{
		int direction = static_cast<int>( Signf( scrollAmount ) );
		int currentTimeFactor = RoundDownToInt( m_FFTWaveSimulation->GetTimeFactor() );

		currentTimeFactor += direction * TIME_FACTOR_CHANGE;
		Clamp( currentTimeFactor, 0, 25 );

		m_FFTWaveSimulation->SetTimeFactor( static_cast<float>( currentTimeFactor ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateSamples()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	if( scrollAmount != 0.f )
	{
		int direction = static_cast<int>( Signf( scrollAmount ) );
		if( direction == 1 )
		{
			IncreaseSamples();
		}
		else
		{
			DecreaseSamples();
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateDimensions()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * DIMENSIONS_PER_SCROLL;
	AddUniformDimensions( amountToChange );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGlobalWaveAmplitude()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * GLOBAL_AMPLITUDE_PER_SCROLL;
	AddGlobalWaveAmp( amountToChange );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateWaveSuppression()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * WAVE_SUPPRESS_PER_SCROLL;
	AddWaveSuppression( amountToChange );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateWindSpeed()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * WIND_SPEED_PER_SCROLL;
	AddWindSpeed( amountToChange );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateWindDirection()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * WIND_ROTATION_PER_SCROLL;
	RotateWindDirByDegrees( amountToChange );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateIWaveSourcePower()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * IWAVE_SOURCE_POWER_PER_SCROLL;
	m_iWaveSourcePower += amountToChange;
	Clamp( m_iWaveSourcePower, 1.f, 10.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFoamDepth()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * FOAM_DEPTH_PER_SCROLL;
	m_waterInfo.FOAM_THICKNESS += amountToChange;
	Clamp( m_waterInfo.FOAM_THICKNESS, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFoaminess()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * FOAMINESS_PER_SCROLL;
	m_waterInfo.FOAMINESS += amountToChange;
	Clamp( m_waterInfo.FOAMINESS, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMaxDepth()
{
	float scrollAmount = g_theInput->GetScrollAmount();
	float amountToChange = scrollAmount * MAX_DEPTH_PER_SCROLL;
	m_waterInfo.MAX_DEPTH += amountToChange;
	Clamp( m_waterInfo.MAX_DEPTH, 0.f, 100.f );
	m_waterInfo.INVERSE_MAX_DEPTH = 1.f / m_waterInfo.MAX_DEPTH;
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

	m_tempDimensions.x = m_tempDimensions.x < 1.f ? 1.f : m_tempDimensions.x;
	m_tempDimensions.y = m_tempDimensions.y < 1.f ? 1.f : m_tempDimensions.y;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddGlobalWaveAmp( float ampToAdd )
{
	m_tempGloabalWaveAmp += ampToAdd;
	m_tempGloabalWaveAmp = m_tempGloabalWaveAmp < 0.f ? 0.f : m_tempGloabalWaveAmp;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddWaveSuppression( float suppressionToAdd )
{
	m_tempWaveSuppression += suppressionToAdd;
	m_tempWaveSuppression = m_tempWaveSuppression < 0.f ? 0.f : m_tempWaveSuppression;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddWindSpeed( float windSpeedToAdd )
{
	m_tempWindSpeed += windSpeedToAdd;
	m_tempWindSpeed = m_tempWindSpeed < 0.f ? 0.f : m_tempWindSpeed;
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
	UpdateWaterInfoFromInput();

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}

	if( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_worldCamera->SetPosition( Vec3( 0.0f ) );
		m_worldCamera->SetPitchYawRollRotationDegrees( 0.f, 0.f, 0.0f );
	}

	if( g_theInput->IsKeyPressed( APPLY_IWAVE_SOURCES_KEY ) )
	{
		UpdateIWaveSourcePower();
	}
	else if( g_theInput->WasKeyJustReleased( APPLY_IWAVE_SOURCES_KEY ) )
	{
		AddIWaveSources();
	}

	if( g_theInput->WasKeyJustPressed( TOGGLE_CRATE_KEY ) )
	{
		m_updateCrate = !m_updateCrate;
	}
	if( g_theInput->WasKeyJustPressed( TOGGLE_SKYBOX_KEY ) )
	{
		m_isSkyBox = !m_isSkyBox;
		m_skyBoxTexture = m_isSkyBox ? m_skyBox : m_noSkyBox;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F1 ) )
	{
		ReloadCurrentXML();
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F2 ) )
	{
		LoadCurrentTempValues();
	}
	if( g_theInput->WasKeyJustPressed( TOGGLE_UNDERWATER_EFFECT_KEY ) )
	{
		m_showUnderwaterEffect = !m_showUnderwaterEffect;
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F11 ) )
	{
		if( m_isDebugText == g_isDebugDraw )
		{
			m_isDebugText = !m_isDebugText;
		}
		else
		{
			g_isDebugDraw = !g_isDebugDraw;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCamera( float deltaSeconds )
{
	float forwardMoveAmount = 0.f;
	float leftMoveAmount = 0.f;
	float upMoveAmount = 0.f;
	float moveSpeed = CAMERA_MOVE_SPEED * deltaSeconds;
	if( g_theInput->IsKeyPressed( CAMERA_FAST_KEY ) )
	{
		moveSpeed *= CAMERA_FAST_FACTOR;
	}

	if( g_theInput->IsKeyPressed( CAMERA_FORWARD_KEY ) )
	{
		forwardMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( CAMERA_BACKWARD_KEY ) )
	{
		forwardMoveAmount -= moveSpeed;
	}	

	if( g_theInput->IsKeyPressed( CAMERA_LEFT_KEY ) )
	{
		leftMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( CAMERA_RIGHT_KEY ) )
	{
		leftMoveAmount -= moveSpeed;
	}

	if( g_theInput->IsKeyPressed( CAMERA_UP_KEY ) )
	{
		upMoveAmount += moveSpeed;
	}
	if( g_theInput->IsKeyPressed( CAMERA_DOWN_KEY ) )
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
	std::string filepath = args->GetValue( "file", "default" );
	if( filepath == "default" )
	{
		g_theConsole->ErrorString( "Expected a parameter for <file>" );
		g_theConsole->ErrorString( "  Loading \"Test.xml\"" );
		filepath = "Test.xml";
	}

	LoadSimulationFromXML( filepath.c_str() );
}