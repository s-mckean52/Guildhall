#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
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
	XmlDocument shaderFile = new XmlDocument();
	shaderFile.LoadFile( "Data/Shaders/lit.shaderstate" );
	ShaderState testState = ShaderState( g_theRenderer, *shaderFile.RootElement() );


	mesh_import_options_t scifi_fighter_options;
	scifi_fighter_options.generateNormals = false;
	scifi_fighter_options.generateTangents = true;
	scifi_fighter_options.invertWindingOrder = false;
	scifi_fighter_options.invertV = false;
	std::vector<Vertex_PCUTBN> objVerts;
	std::vector<unsigned int> objIndicies;
	ReadAndParseObjFile( "Data/Models/scifi_fighter/mesh.obj", objVerts );
	MeshLoadToVertexArray( objVerts, scifi_fighter_options );
	m_objMesh = new GPUMesh( g_theRenderer );
	m_objMesh->UpdateVerticies( objVerts.size(), &objVerts[0] );

	EnableDebugRendering();

	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );

	g_theEventSystem->SubscribeEventCallbackFunction( "GainFocus", GainFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "LoseFocus", LoseFocus );
	g_theEventSystem->SubscribeEventCallbackFunction( "light_set_ambient_color", light_set_ambient_color );
	g_theEventSystem->SubscribeEventCallbackFunction( "light_set_color", light_set_color );

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

	m_p0 = Vec3( -1.f, -1.f, 0.f );
	m_p1 = Vec3( 1.f, -1.f, 0.f );
	m_p2 = Vec3( 1.f, 1.f, 0.f );
	m_p3 = Vec3( -1.f, 1.f, 0.f );

	std::vector<Vertex_PCUTBN> litQuadVerts;
	AppendVertsForQuad3D( litQuadVerts, m_p0, m_p1, m_p2, m_p3, Rgba8::WHITE );
	m_quad = new GPUMesh( g_theRenderer );
	m_quad->UpdateVerticies( static_cast<unsigned int>( litQuadVerts.size() ), &litQuadVerts[0] );



// 	std::vector<Vertex_PCU> planeVerts;
// 	std::vector<unsigned int> planeIndicies;
// 	AddPlaneToIndexedVertexArray(	planeVerts, planeIndicies, Vec3(), Rgba8::GREEN,
// 									Vec3::RIGHT, -1.f, 1.f,
// 									Vec3::UP, -1.f, 1.f, 3 );
// 	AddSurfaceToIndexedVertexArray( planeVerts, planeIndicies, Vec3(), Rgba8::GREEN,
// 									-1.f, 1.f, 10,
// 									-1.f, 1.f, 10, ParabolaEquation );
// 	m_plane = new GPUMesh( g_theRenderer, planeVerts, planeIndicies );


	m_cubeTransform = new Transform();
	m_sphereTransform = new Transform();
	m_ringTransform = new Transform();
	m_quadTransform = new Transform();
	m_triplanarSphereTransform = new Transform();
	m_quadTransform->SetPosition( Vec3( 0.f, 0.f, -12.f ) );
	m_cubeTransform->SetPosition( Vec3( 5.f, 0.f, -12.f ) );
	m_sphereTransform->SetPosition( Vec3( -5.f, 0.f, -12.f ) );
	m_triplanarSphereTransform->SetPosition( Vec3( -10.f, 0.f, -12.f ) );

	g_devConsoleFont	= g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	m_pokeball			= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/pokeball.png" );
	m_dissolveImage		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/noise.png" );
	m_couchDiffuse		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/example_color.png" );
	m_couchNormal		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/example_normal.png" );
	m_barkDiffuse		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/bark_diffuse.png" );
	m_barkNormal		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/bark_normal.png" );
	m_brickDiffuse		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Brick/brick_diffuse.png" );
	m_brickNormal		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Brick/brick_normal.png" );
	m_brickHeight		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Brick/brick_height.png" );
	m_test				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_projectionImage	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );

	m_defaultShader = nullptr;
	m_invertColorShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/invertColor.hlsl" );
	m_litShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" );
 	m_normalsShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Normals.hlsl" );
 	m_tangentsShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Tangents.hlsl" );
 	m_bitangentsShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Bitangents.hlsl" );
 	m_surfaceNormalsShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/SurfaceNormals.hlsl" );
	m_fresnelShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Fresnel.hlsl" );
	m_dissolveShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/LitDissolve.hlsl" );
	m_triplanarShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Triplanar.hlsl" );
	m_fogShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/LitFog.hlsl" );
	m_projectionShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Projection.hlsl" );
	m_parallaxShader = g_theRenderer->GetOrCreateShader( "Data/Shaders/LitParallax.hlsl" );

	AddShader( "Lit Fog", m_fogShader );
	AddShader( "Color Only", nullptr );
	AddShader( "Vertex Normals", m_normalsShader );
	AddShader( "Vertex Tangents", m_tangentsShader );
	AddShader( "Vertex Bitangents", m_bitangentsShader );
	AddShader( "Surface Normals", m_surfaceNormalsShader );
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

	delete m_objMesh;
	m_objMesh = nullptr;

	delete m_quad;
	m_quad = nullptr;

	delete m_quadTransform;
	m_quadTransform = nullptr;

	delete m_cubeTransform;
	m_cubeTransform = nullptr;

	delete m_sphereTransform;
	m_sphereTransform = nullptr;

	delete m_triplanarSphereTransform;
	m_triplanarSphereTransform = nullptr;

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
	g_theRenderer->BindSampler( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_BACK );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LEQUAL, true );

	g_theRenderer->DisableAllLights();
	g_theRenderer->SetAmbientLight( m_ambientColor, m_ambientIntensity );
	EnableLightsForRendering();

	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Models/scifi_fighter/diffuse.png" ) );
	//g_theRenderer->BindNormalTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/SF_Fighter/SF_Fighter-Normal.png" ) );
	//g_theRenderer->SetModelUBO( m_quadTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->BindSampler( g_theRenderer->m_samplerLinear );
	g_theRenderer->BindShader( m_litShader );
	g_theRenderer->DrawMesh( m_objMesh );
	//RenderWorld();

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
	//Render Quad
	g_theRenderer->BindTexture( m_brickDiffuse );
	g_theRenderer->BindNormalTexture( m_brickNormal );
	g_theRenderer->BindMaterialTexture( 8, m_brickHeight );
	g_theRenderer->BindSampler( g_theRenderer->m_samplerLinear );
	g_theRenderer->BindShader( m_parallaxShader );

	parallax_t parallaxData;
	parallaxData.depth = m_parallaxDepth;
	g_theRenderer->SetMaterialUBO( &parallaxData, sizeof( parallaxData ) );
	g_theRenderer->SetModelUBO( m_quadTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_quad );


	g_theRenderer->BindShader( m_shadersToUse[ m_currentShaderIndex ] );
	g_theRenderer->BindSampler( nullptr );
	g_theRenderer->BindTexture( m_couchDiffuse );
	g_theRenderer->BindNormalTexture( m_couchNormal );

	//Render Sphere
	g_theRenderer->BindShader( m_shadersToUse[ m_currentShaderIndex ] );
	g_theRenderer->SetModelUBO( m_sphereTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_uvSphere );

	//Render Cube
	dissolve_t dissolveData;
	dissolveData.amount = m_dissolveAmount;
	dissolveData.edgeRange = 0.1f;
	dissolveData.edgeEndColor = Vec3( 1.f, 0.f, 0.f );
	dissolveData.edgeStartColor = Vec3( 1.f, 1.f, 0.f );
	g_theRenderer->BindMaterialTexture( 8, m_dissolveImage );
	g_theRenderer->BindShader( m_dissolveShader );
	g_theRenderer->SetMaterialUBO( &dissolveData, sizeof( dissolveData ) );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_meshCube );


	//Render Sphere
	g_theRenderer->BindMaterialTexture( 8 + 0, m_couchDiffuse );
	g_theRenderer->BindMaterialTexture( 8 + 1, m_couchNormal );
	g_theRenderer->BindMaterialTexture( 8 + 2, m_brickDiffuse );
	g_theRenderer->BindMaterialTexture( 8 + 3, m_brickNormal );
	g_theRenderer->BindMaterialTexture( 8 + 4, m_barkDiffuse );
	g_theRenderer->BindMaterialTexture( 8 + 5, m_barkNormal );
	g_theRenderer->BindShader( m_triplanarShader );
	g_theRenderer->SetModelUBO( m_triplanarSphereTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_uvSphere );


	//Render Fresnel
	fresnel_t fresnelData;
	fresnelData.color = Vec3( 0.f, 1.f, 0.f );
	fresnelData.power = 32.f;
	fresnelData.factor = 1.f;
	g_theRenderer->SetDepthTest( COMPARE_FUNC_EQUAL, true );
	g_theRenderer->SetMaterialUBO( &fresnelData, sizeof( fresnelData ) );
	g_theRenderer->BindShader( m_fresnelShader );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_meshCube );


	//Render Projection
	projection_t projectionData;
	Light light = m_animatedLights[0].light;
	Mat44 projection = Mat44::CreatePerspectiveProjection( 90.f, 1.f, 0.1f, 100.f );
	Mat44 view = Mat44::LookAt( light.position, light.position + light.direction );
	MatrixInvert( view );
	projectionData.matrix = view.GetTransformMatrixBy( projection );
	projectionData.position = light.position;
	projectionData.intensity = m_projectionIntensity;

	g_theRenderer->BindMaterialTexture( 8, m_projectionImage );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_EQUAL, true );
	g_theRenderer->SetMaterialUBO( &projectionData, sizeof( projectionData ) );
	g_theRenderer->BindShader( m_projectionShader );

	//Redraw all objects for Projection
	g_theRenderer->SetModelUBO( m_quadTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_quad );
	g_theRenderer->SetModelUBO( m_sphereTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_uvSphere );
	g_theRenderer->SetModelUBO( m_triplanarSphereTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_uvSphere );
	g_theRenderer->SetModelUBO( m_cubeTransform->ToMatrix(), Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_theRenderer->DrawMesh( m_meshCube );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderRingOfSpheres() const
{
/*	Mat44 model = m_sphereTransform->ToMatrix();*/
/*	g_theRenderer->SetModelMatrix( model );*/
	g_theRenderer->BindTexture( m_pokeball );
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

		g_theRenderer->SetModelUBO( sphereMat );
		g_theRenderer->DrawMesh( m_uvSphere );

		currentAngleDegrees += degreeStep;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	const float textHeight = 0.15f;
	const float paddingFromLeft = 0.015f;
	const float paddingFromTop = 0.05f;

	std::vector<std::string> strings;
	std::vector<Vertex_PCU> textVerts;

	Light currentLight = m_animatedLights[m_selectedLight].light;
	std::string	lightType = m_animatedLights[m_selectedLight].GetLightTypeAsString();

	strings.push_back( Stringf( "[q,e] - Current Light [%i]"				, m_selectedLight ) );
	strings.push_back( Stringf( "[R]   - Light Type - %s"					, lightType.c_str() ) );
	strings.push_back( Stringf( "[F4]  - Position At Origin" ) );
	strings.push_back( Stringf( "[F5]  - Position At Camera" ) );
	strings.push_back( Stringf( "[F6]  - Follow Camera" ) );
	strings.push_back( Stringf( "[F7]  - Animate Position" ) );
	strings.push_back( Stringf( "[g,h] - Gamma: %.2f"						, g_theRenderer->GetGamma() ) );
	strings.push_back( Stringf( "[0,9] - Ambient Intensity: %.2f"			, m_ambientIntensity ) );
	strings.push_back( Stringf( "[-,+] - Point Light Intensity: %.2f"		, currentLight.intensity ) );
	strings.push_back( Stringf( "[T]   - Attenuation: (%.2f, %.2f, %.2f)"	, currentLight.attenuation.x, currentLight.attenuation.y, currentLight.attenuation.z ) );
	strings.push_back( Stringf( "[{,}] - Specular Factor: %.2f"				, m_specularFactor ) );
	strings.push_back( Stringf( "[;,'] - Specular Power: %.2f"				, m_specularPower ) );
	strings.push_back( Stringf( "[<,>] - Shader Mode: %s"					, m_shaderNames[ m_currentShaderIndex ].c_str() ) );
	strings.push_back( Stringf( "[N,M] - Parallax Depth: %.2f"				, m_parallaxDepth ) );

	Vec2 textStartPos = Vec2( paddingFromLeft, m_UICamera->GetCameraDimensions().y - paddingFromTop - textHeight );
	for( int stringIndex = 0; stringIndex < strings.size(); ++stringIndex )
	{
		g_devConsoleFont->AddVertsForText2D( textVerts, textStartPos, textHeight, strings[ stringIndex ], Rgba8::WHITE );
		textStartPos -= Vec2( 0.f, textHeight + paddingFromTop );
	}

	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->BindNormalTexture( nullptr );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->DrawVertexArray( textVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::EnableLightsForRendering() const
{
	for( unsigned int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
	{
		g_theRenderer->EnableLight( lightIndex, m_animatedLights[lightIndex].light );
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
	m_cubeTransform->AddRotationPitchYawRollDegrees( 45.f * deltaSeconds, 90.f * deltaSeconds, 0.f );
	m_sphereTransform->AddRotationPitchYawRollDegrees( 45.f * deltaSeconds, 90.f * deltaSeconds, 0.f );
	m_ringTransform->AddRotationPitchYawRollDegrees( 0.f, 0.f, 10.f * deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateLightPositions()
{
	for( unsigned int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
	{
		Light currentLight = m_animatedLights[ lightIndex ].light;
		if( m_animatedLights[lightIndex].moveType == LIGHT_MOVEMENT_FOLLOW )
		{
			currentLight.position = m_worldCamera->GetPosition();
			currentLight.direction = -m_worldCamera->GetTransform().ToMatrix().GetKBasis3D();
		}
		else if( m_animatedLights[lightIndex].moveType == LIGHT_MOVEMENT_ANIMATED )
		{
			const float animateRadius = 7.f;
			Vec3 animateCenter = m_quadTransform->GetPosition();
			float time = static_cast<float>(m_gameClock->GetTotalElapsedSeconds());

			Vec3 lightPosition = animateCenter;
			lightPosition += Vec3(static_cast<float>(cos(time)), 0.f, static_cast<float>(sin(time))) * animateRadius;
			currentLight.position = lightPosition;
			currentLight.direction = Mat44::LookAt( lightPosition, animateCenter ).GetKBasis3D();
		}

		m_animatedLights[ lightIndex ].light = currentLight;
		DebugDrawLight( &m_animatedLights[ lightIndex ] );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DebugDrawLight( AnimatedLight* lightToDraw )
{
	if( lightToDraw->moveType == LIGHT_MOVEMENT_FOLLOW ) return; 

	Light currentLight = lightToDraw->light;
	Vec3 pointPos = currentLight.position;
	Rgba8 pointColor = Rgba8::MakeFromFloats( currentLight.color.x, currentLight.color.y, currentLight.color.z, GetClampZeroToOne(currentLight.intensity) );

	if( lightToDraw->lightType == LIGHT_TYPE_SPOTLIGHT )
	{
		const float coneHeight = 0.5f;
		Vec3 coneEndPosition = pointPos + ( currentLight.direction * coneHeight );
		float coneEndRadius = sinf( acosf( currentLight.cosOutterHalfAngle ) ) * coneHeight;
		DebugAddWorldCone( pointPos, coneEndPosition, coneEndRadius, pointColor, 0.f, DEBUG_RENDER_XRAY );
	}
	else if( lightToDraw->lightType == LIGHT_TYPE_DIRECTIONAL )
	{
		const float arrowLength = 1.f;
		DebugAddWorldArrow( pointPos, pointPos + ( currentLight.direction * arrowLength ), pointColor, 0.f, DEBUG_RENDER_XRAY );
	}
	else
	{
		DebugAddWorldPoint( pointPos, pointColor, 0.f, DEBUG_RENDER_XRAY );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleLightToModify()
{
	int currentlySelectedLight = m_selectedLight;
	if( g_theInput->WasKeyJustPressed( 'Q' ) )
	{
		currentlySelectedLight -= 1;
	}
	if( g_theInput->WasKeyJustPressed( 'E' ) )
	{
		currentlySelectedLight += 1;
	}

	if( currentlySelectedLight > static_cast<int>( MAX_LIGHTS - 1 ) )
	{
		currentlySelectedLight = 0;
	}
	else if( currentlySelectedLight < 0 )
	{
		currentlySelectedLight = MAX_LIGHTS - 1;
	}
	m_selectedLight = currentlySelectedLight;
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleLightType( AnimatedLight* lightToModify )
{
	LightType currentLightType = lightToModify->lightType;
	LightType newLightType = ( LightType )( ( currentLightType + 1 ) % NUM_LIGHT_TYPES );
	lightToModify->SetLightType( newLightType );
}


//---------------------------------------------------------------------------------------------------------
void Game::ToggleFog()
{
	if( m_isFogEnabled )
	{
		g_theRenderer->DisableFog();
		m_isFogEnabled = false;
	}
	else
	{
		g_theRenderer->EnableFog( ( 100 - 0.1f ) * 0.5f, 100.f, Rgba8::YELLOW, m_clearColor );
		m_isFogEnabled = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::AddParallaxDepth( float depthToAdd )
{
	m_parallaxDepth += depthToAdd;
	Clamp( m_parallaxDepth, 0.01f, 0.1f );
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
	UpdateLightPositions();

	//ChangeClearColor( deltaSeconds );
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

	if( g_theInput->IsKeyPressed( KEY_CODE_UP_ARROW ) )
	{
		m_dissolveAmount += 0.5f * deltaSeconds;
		ClampZeroToOne( m_dissolveAmount );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_DOWN_ARROW ) )
	{
		m_dissolveAmount -= 0.5f * deltaSeconds;
		ClampZeroToOne( m_dissolveAmount );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		m_projectionIntensity += 0.5f * deltaSeconds;
		ClampZeroToOne( m_projectionIntensity );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_ARROW ) )
	{
		m_projectionIntensity -= 0.5f * deltaSeconds;
		ClampZeroToOne( m_projectionIntensity );
	}

	if( g_theInput->WasKeyJustPressed( 'F' ) )
	{
		ToggleFog();
	}

	if (g_theInput->IsKeyPressed( 'N' ) )
	{
		AddParallaxDepth( -0.05f * deltaSeconds );
	}
	if (g_theInput->IsKeyPressed( 'M' ) )
	{
		AddParallaxDepth( 0.05f * deltaSeconds );
	}

	UpdateDrawDebugObjects();
	UpdateInputLights( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateDrawDebugObjects()
{
	Mat44 cameraTransform = m_worldCamera->GetTransform().ToMatrix();
	//draw world line
	if( g_theInput->WasKeyJustPressed( '1' ) )
	{
		DebugAddWorldLine( cameraTransform.GetTranslation3D(), Vec3::ZERO, Rgba8::WHITE, Rgba8::BLACK, 10.f, DEBUG_RENDER_USE_DEPTH );
	}
	//draw world arrow
	if( g_theInput->WasKeyJustPressed( '2' ) )
	{
		DebugAddWorldArrow( Vec3::ZERO, cameraTransform.GetTranslation3D(), Rgba8::GREEN, 10.f, DEBUG_RENDER_XRAY );
	}
	//draw world basis
	if( g_theInput->WasKeyJustPressed( '3' ) )
	{
		DebugAddWorldBasis( cameraTransform, Rgba8::WHITE, Rgba8::BLACK, 20.f, DEBUG_RENDER_ALWAYS );
	}
	//draw world quad
	if( g_theInput->WasKeyJustPressed( '4' ) )
	{
		Vec3 cameraPosition = cameraTransform.GetTranslation3D();
		Vec3 p0 = Vec3::ZERO;
		Vec3 p1 = Vec3( cameraPosition.x, 0.f, cameraPosition.z );
		Vec3 p2 = cameraPosition;
		Vec3 p3 = Vec3( 0.f, cameraPosition.y, 0.f );
		DebugAddWorldQuad( p0, p1, p2, p3, AABB2(), Rgba8::CYAN, Rgba8::MAGENTA, 5.f, DEBUG_RENDER_ALWAYS );
	}
	//draw world text
	if( g_theInput->WasKeyJustPressed( '5' ) )
	{
		Vec3 cameraPosition = cameraTransform.GetTranslation3D();
		DebugAddWorldTextf( cameraTransform, ALIGN_CENTERED, Rgba8::ORANGE, 10.f, DEBUG_RENDER_XRAY, "x: %.2f, y: %.2f, z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z );
	}
	//draw screen line
	if( g_theInput->WasKeyJustPressed( '6' ) )
	{
		DebugAddScreenArrow( Vec2::ZERO, Vec2::UNIT * 100.f, Rgba8::WHITE, 10.f );
	}
	//draw screen textured quad
	if( g_theInput->WasKeyJustPressed( '7' ) )
	{
		AABB2 textureBounds = AABB2( 500.f, 500.f, 700.f, 700.f );
		DebugAddScreenTexturedQuad( textureBounds, m_couchDiffuse, AABB2(), Rgba8::MAGENTA, Rgba8::CYAN, 10.f );
	}
}

//---------------------------------------------------------------------------------------------------------
void Game::UpdateInputLights( float deltaSeconds )
{
	CycleLightToModify();

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
	// Cycle Attenuation Mode
	if( g_theInput->WasKeyJustPressed( 'T' ) )
	{
		CycleAttenuationMode();
	}
	// Update Point Light Intensity
	if( g_theInput->IsKeyPressed( KEY_CODE_PLUS ) )
	{
		AddPointLightIntensity( pointLightIntensityUpdateAmount * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_MINUS ) )
	{
		AddPointLightIntensity( -pointLightIntensityUpdateAmount * deltaSeconds );
	}
	// Update Specular Factor
	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_BRACKET ) )
	{
		AddSpecFactor( -specularFactorUpdateAmount * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_BRACKET ) )
	{
		AddSpecFactor( specularFactorUpdateAmount * deltaSeconds );
	}
	// Update Specular Power
	if( g_theInput->IsKeyPressed( KEY_CODE_SEMICOLON ) )
	{
		AddSpecPower( -specularPowerUpdateAmount * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( KEY_CODE_APOSTROPHE ) )
	{
		AddSpecPower( specularPowerUpdateAmount * deltaSeconds );
	}
	// Update Gamma
	if( g_theInput->IsKeyPressed( 'G' ) )
	{
		AddGamma( -gammaUpdateAmount * deltaSeconds );
	}
	if( g_theInput->IsKeyPressed( 'H' ) )
	{
		AddGamma( gammaUpdateAmount * deltaSeconds );
	}
	// Update Shader
	if( g_theInput->WasKeyJustPressed( KEY_CODE_COMMA ) )
	{
		ChangeShader( -1 );
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_PERIOD ) )
	{
		ChangeShader( 1 );
	}
	if( g_theInput->WasKeyJustPressed( 'R' ) )
	{
		CycleLightType( &m_animatedLights[m_selectedLight] );
	}



	//---------------------------------------------------------------------------------------------------------
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F4 ) )
	{
		m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_STATIONARY;
		m_animatedLights[m_selectedLight].light.position = Vec3::ZERO;
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F5 ) )
	{	
		m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_STATIONARY;
		m_animatedLights[m_selectedLight].light.position = m_worldCamera->GetPosition();
		m_animatedLights[m_selectedLight].light.direction = -m_worldCamera->GetTransform().ToMatrix().GetKBasis3D();
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F6 ) )
	{
		LightMovement currentMovement = m_animatedLights[m_selectedLight].moveType;
		if( currentMovement == LIGHT_MOVEMENT_FOLLOW )
		{ 
			m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_STATIONARY;
		}
		else
		{
			m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_FOLLOW;
		}
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_F7 ) )
	{
		LightMovement currentMovement = m_animatedLights[m_selectedLight].moveType;
		if( currentMovement == LIGHT_MOVEMENT_ANIMATED ) 
		{ 
			m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_STATIONARY;
		}
		else
		{
			m_animatedLights[m_selectedLight].moveType = LIGHT_MOVEMENT_ANIMATED;
		}
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
void Game::AddAmbientLightIntensity( float intensityToAdd )
{
	float newIntensity = m_ambientIntensity;
	newIntensity += intensityToAdd;
	ClampZeroToOne( newIntensity );

	m_ambientIntensity = newIntensity;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddPointLightIntensity( float intensityToAdd )
{
	Light currentLight = m_animatedLights[m_selectedLight].light;
	float newIntensity = currentLight.intensity;
	newIntensity += intensityToAdd;
	if( newIntensity >= 0.f )
	{
		currentLight.intensity = newIntensity;
	}
	else
	{
		currentLight.intensity = 0.f;
	}
	m_animatedLights[m_selectedLight].light = currentLight;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddGamma( float gammaToAdd )
{
	float newGamma = g_theRenderer->GetGamma();
	newGamma += gammaToAdd;
	Clamp( newGamma, 0.25f, 4.f );
	g_theRenderer->UpdateGamma( newGamma );
}


//---------------------------------------------------------------------------------------------------------
void Game::AddSpecFactor( float factorToAdd )
{
	float newSpecFactor = m_specularFactor;
	newSpecFactor += factorToAdd;
	m_specularFactor = GetClampZeroToOne( newSpecFactor );
}


//---------------------------------------------------------------------------------------------------------
void Game::AddSpecPower( float powerToAdd )
{
	float newSpecPower = m_specularPower;
	newSpecPower += powerToAdd;
	if (newSpecPower >= 1.f)
	{
		m_specularPower = newSpecPower;
		return;
	}
	m_specularPower = 1.f;
}


//---------------------------------------------------------------------------------------------------------
void Game::CycleAttenuationMode()
{
	Vec3 currentAttenuation = m_animatedLights[m_selectedLight].light.attenuation;
	if( currentAttenuation == Vec3( 0.f, 1.f, 0.f ) )
	{
		currentAttenuation = Vec3( 0.f, 0.f, 1.f );
	}
	else if( currentAttenuation == Vec3( 0.f, 0.f, 1.f ) )
	{
		currentAttenuation = Vec3( 1.f, 0.f, 0.f );
	}
	else
	{
		currentAttenuation = Vec3( 0.f, 1.f, 0.f );
	}
	m_animatedLights[m_selectedLight].light.attenuation = currentAttenuation;
	m_animatedLights[m_selectedLight].light.specAttenuation = currentAttenuation;
}


//---------------------------------------------------------------------------------------------------------
void Game::ChangeShader( int direction )
{
	int numShaders = static_cast<int>( m_shadersToUse.size() );
	m_currentShaderIndex += direction;
	if( m_currentShaderIndex < 0 )
	{
		m_currentShaderIndex = numShaders - 1;
	}
	else if( m_currentShaderIndex >= numShaders )
	{
		m_currentShaderIndex = 0;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::AddShader( std::string shaderName, Shader* shader )
{
	m_shaderNames.push_back( shaderName );
	m_shadersToUse.push_back( shader );
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


//---------------------------------------------------------------------------------------------------------
STATIC void Game::light_set_ambient_color( NamedStrings* args )
{
	Vec3 defaultAmbientColor = Vec3::UNIT;
	Vec3 ambientColor = args->GetValue( "color", defaultAmbientColor );

	Rgba8 ambientRGBA8 = Rgba8::MakeFromFloats( ambientColor.x, ambientColor.y, ambientColor.z, 0.f );
	g_theGame->m_ambientColor = ambientRGBA8;
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::light_set_color( NamedStrings* args )
{
	int defaultLightIndex = 0;
	Vec3 defaultPointLightColor = Vec3::UNIT;

	int lightIndex = args->GetValue( "index", defaultLightIndex );
	Vec3 pointLightColor = args->GetValue( "color", defaultPointLightColor );

	Clamp( lightIndex, 0, MAX_LIGHTS - 1 );

	g_theGame->m_animatedLights[lightIndex].light.color = pointLightColor;
}
