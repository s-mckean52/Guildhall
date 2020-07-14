#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/Ability.hpp"
#include "Game/Item.hpp"
#include "Game/World.hpp"
#include "Game/Cursor.hpp"
#include "Game/UIButton.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/App.hpp"
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

	g_RNG = new RandomNumberGenerator();

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

	RenderBasedOnState();

	g_theRenderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );


	//Render UI
	g_theRenderer->BeginCamera( *m_UICamera );
	RenderUI();
	g_theRenderer->EndCamera( *m_UICamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderBasedOnState() const
{
	switch( m_gameState )
	{
	case GAME_STATE_LOADING:
		RenderLoading();
		break;
	case GAME_STATE_MENU:
		RenderMenu();
		break;
	case GAME_STATE_PLAYING:
		m_world->Render();
		break;
	case GAME_STATE_PAUSED:
		RenderPause();
		break;
	case GAME_STATE_DEAD:
		RenderDead();
		break;
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( m_gameState == GAME_STATE_LOADING )
		return;

	if( m_gameState == GAME_STATE_PLAYING )
	{
		Vec3 uiBottomLeft = m_UICamera->GetOrthoBottomLeft();
		Vec3 uiTopRight = m_UICamera->GetOrthoTopRight();
		AABB2 uiCameraRect = AABB2( uiBottomLeft.x, uiBottomLeft.y, uiTopRight.x, uiTopRight.y );

		float distanceBetweenAbilities = uiCameraRect.GetDimensions().x * 0.05f;
		Vec2 abilityUIStartUV = Vec2( 0.3f, 0.f );
		Vec2 abilityUIStartPos = uiCameraRect.GetPointAtUV( abilityUIStartUV );
		m_player->RenderAbilities( abilityUIStartPos, distanceBetweenAbilities );

		m_world->RenderProgressBar();
	}

	m_cursor->Render();

	if( g_isDebugDraw )
	{
		const float textHeight = 0.15f;
		const float paddingFromLeft = 0.015f;
		const float paddingFromTop = 0.05f;

		std::vector<ColorString> strings;
		std::vector<Vertex_PCU> textVerts;
		
		int		attackDamage			= m_player->GetAttackDamage();
		int		critDamage				= m_player->GetCritDamage();
		float	critChancePercent		= m_player->GetCritChanceFraction() * 100.f;
		float	critMultiplierPercent	= m_player->GetCritMultiplier() * 100.f;
		float	movementSpeed			= m_player->GetMoveSpeed();
		float	attackSpeed				= m_player->GetAttackSpeed();

		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "Player Stats:"													) ) );			
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Attack Damage:   %i",					attackDamage			) ) );
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Crit Damage:     %i",					critDamage				) ) );
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Crit Chance:     %.f%%",				critChancePercent		) ) );
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Crit Multiplier: %.f%%",				critMultiplierPercent	) ) );
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Movement Speed:  %.2f units/sec",		movementSpeed			) ) );
		strings.push_back( ColorString( Rgba8::WHITE,	Stringf( "  Attack Speed:    %.2f attacks/sec",		attackSpeed				) ) );

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
Vec2 Game::GetCursorPosition() const
{
	return m_cursor->GetWorldPosition();
}


//---------------------------------------------------------------------------------------------------------
Map* Game::GetCurrentMap() const
{
	return m_world->GetCurrentMap();
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
	if( m_gameState == GAME_STATE_LOADING )
	{
		LoadGame();
	}
	else if( !g_theConsole->IsOpen() )
	{
		UpdateFromInput();
	}
	UpdateStateBasedOnGameState();

}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateFromInput()
{
	m_cursor->Update();

	if( m_gameState == GAME_STATE_MENU )
	{
		UpdateMenuFromInput();
	}
	else if( m_gameState == GAME_STATE_PLAYING )
	{
		if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
		{
			m_gameState = GAME_STATE_PAUSED;
			m_gameClock->Pause();
		}

		UpdateCursor();
		MoveWorldCamera();
	}
	else if( m_gameState == GAME_STATE_PAUSED )
	{
		UpdatePauseFromInput();
	}
	else if( m_gameState == GAME_STATE_DEAD )
	{
		UpdateDeadFromInput();
	}

	//if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	//{
	//	m_isQuitting = true;
	//}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCamera()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	float rightMoveAmount = 0.f;
	float upMoveAmount = 0.f;
	float moveSpeed = 5.f * deltaSeconds;

	if( g_theInput->IsKeyPressed( KEY_CODE_UP_ARROW ) )
	{
		upMoveAmount += moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( KEY_CODE_DOWN_ARROW ) )
	{
		upMoveAmount -= moveSpeed;
	}	

	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_ARROW ) )
	{
		rightMoveAmount -= moveSpeed;
	}	
	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		rightMoveAmount += moveSpeed;
	}


	Vec3 cameraTranslation = Vec3( rightMoveAmount, upMoveAmount, 0.f );
	m_worldCamera->Translate( cameraTranslation );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCursor()
{
	//m_cursor->Update();

	if( m_gameState != GAME_STATE_PLAYING )
		return;

	if( g_theInput->WasKeyJustPressed( 'A' ) )
	{
		m_cursor->SetState( CURSOR_STATE_ATTACK_MOVE );
	}

	m_hoveredEnemy = m_world->GetDiscOverlapEnemyOnCurrentMap( GetCursorPosition(), m_cursor->GetRadius() );
	if( m_hoveredEnemy != nullptr && m_cursor->GetState() != CURSOR_STATE_ATTACK_MOVE )
	{
		m_cursor->SetState( CURSOR_STATE_ENEMY_HOVERED );
	}

	if( m_hoveredEnemy != nullptr && g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_RIGHT ) )
	{
		m_player->AttackEnemy( m_hoveredEnemy );
		m_cursor->SetState( CURSOR_STATE_DEFAULT );
	}
	else if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_RIGHT ) )
	{
		m_player->SetIsMoving( true );
		m_player->SetMovePosition( GetCursorPosition() );
		m_cursor->SetState( CURSOR_STATE_DEFAULT );
	}
	else if( m_cursor->GetState() == CURSOR_STATE_ATTACK_MOVE && g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
	{
		m_player->SetIsMoving( true );
		m_player->SetAttackMovePosition( GetCursorPosition() );
		m_cursor->SetState( CURSOR_STATE_DEFAULT );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	Vec3 currentCameraPosition = m_worldCamera->GetPosition();
	Vec3 newCameraPosition = Vec3( m_player->GetCurrentPosition(), currentCameraPosition.z );
	m_worldCamera->SetPosition( newCameraPosition );
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


//---------------------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	m_testShader		= g_theRenderer->GetOrCreateShader( "Data/Shaders/WorldOpaque.hlsl" );
	m_testSound			= g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );

	m_cursor = new Cursor();

	TileDefinition::InitializeTileDefinitions();
	Ability::CreateAbilitiesFromXML( "Data/Definitions/AbilityDefinitions.xml" );
	Item::CreateItemsFromXML( "Data/Definitions/ItemDefinitions.xml" );

	m_player = new Player( this );
	m_world = new World( this );

	CreateMenuButtons();

	m_world->AddPlayerToCurrentMap( m_player );
	m_shownAbilities = Ability::GetAbilityList( m_firstAbilityToDisplay, 6 );
}


//---------------------------------------------------------------------------------------------------------
void Game::CreateMenuButtons()
{
	UIButton* newGameButton = new UIButton( "New Game", Vec2( 4.f, 1.f ) );
	newGameButton->SetTextSize( 0.3f );
	newGameButton->SetOnClick( this, &Game::NewGameOnClick );

	UIButton* optionsButton = new UIButton( "Options", Vec2( 4.f, 1.f ), Vec2( 0.5f, 0.3f ) );
	optionsButton->SetTextSize( 0.3f );
	optionsButton->SetOnClick( this, &Game::QuitOnClick );

	UIButton* quitButton = new UIButton( "Quit", Vec2( 4.f, 1.f ), Vec2( 0.5f, 0.1f ) );
	quitButton->SetTextSize( 0.3f );
	quitButton->SetOnClick( this, &Game::QuitOnClick );

	m_mainMenuButtons.push_back( newGameButton );
	//m_mainMenuButtons.push_back( optionsButton );
	m_mainMenuButtons.push_back( quitButton );


	// Ability Menu Buttons
	//---------------------------------------------------------------------------------------------------------
	UIButton* startButton = new UIButton("Start", Vec2(1.25f, 0.5f), Vec2(0.98f, 0.05f));
	startButton->SetTextSize(0.2f);
	startButton->SetOnClick(this, &Game::StartGameOnClick );

	UIButton* backButton = new UIButton("Back", Vec2( 1.25f, 0.5f ), Vec2(0.02f, 0.05f));
	backButton->SetTextSize(0.2f);
	backButton->SetOnClick(this, &Game::BackOnClick);

	m_abilityMenuButtons.push_back(startButton);
	m_abilityMenuButtons.push_back(backButton);

	//Pause Menu buttons
	//---------------------------------------------------------------------------------------------------------
	UIButton* resumeButton = new UIButton( "Resume", Vec2( 4.f, 1.f ), Vec2( 0.5f, 0.3f ) );
	resumeButton->SetTextSize( 0.3f );
	resumeButton->SetOnClick( this, &Game::ResumeGameOnClick );

	UIButton* menuButton = new UIButton( "Main Menu", Vec2( 4.f, 1.f ), Vec2( 0.5f, 0.1f ) );
	menuButton->SetTextSize( 0.3f );
	menuButton->SetOnClick( this, &Game::ReturnToMenuOnClick );

	m_pauseMenuButtons.push_back( resumeButton );
	m_pauseMenuButtons.push_back( menuButton );

	//Dead Menu buttons
	//---------------------------------------------------------------------------------------------------------
	m_deadMenuButtons.push_back( menuButton );

}

//---------------------------------------------------------------------------------------------------------
void Game::RenderLoading() const
{
	Vec3 orthoBottomLeft = m_worldCamera->GetOrthoBottomLeft();
	Vec3 orthoTopRight = m_worldCamera->GetOrthoTopRight();
	AABB2 screenRect = AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );

	std::vector<Vertex_PCU> loadingVerts;
	g_devConsoleFont->AddVertsForTextInBox2D( loadingVerts, screenRect, 1.f, "Loading...", Rgba8::WHITE );
	
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( loadingVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMenuFromInput()
{
	switch( m_menuState )
	{
	case MENU_STATE_MAIN:
		UpdateMainMenu();
		break;
	case MENU_STATE_ABILITY_SELECT:
		UpdateAbilitySelect();
		break;
	case MENU_STATE_OPTIONS:
		break;
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMainMenu()
{
	for( int buttonIndex = 0; buttonIndex < m_mainMenuButtons.size(); ++buttonIndex )
	{
		m_mainMenuButtons[buttonIndex]->Update();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateAbilitySelect()
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_menuState = MENU_STATE_MAIN;
	}

	if( g_theInput->WasKeyJustPressed( 'E' ) )
	{
		++m_selectedAbilitySlot;
	}
	if( g_theInput->WasKeyJustPressed( 'Q' ) )
	{
		--m_selectedAbilitySlot;
	}
	Clamp( m_selectedAbilitySlot, 0, 3 );

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ENTER ) )
	{
		m_selectedAbilities[ m_selectedAbilitySlot ] = m_shownAbilities[ m_selectedAbilityButton ];
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_DOWN_ARROW ) )
	{
		m_selectedAbilityButton += 2;
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_UP_ARROW ) )
	{
		m_selectedAbilityButton -= 2;

	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		m_selectedAbilityButton += 1;
	}
	if( g_theInput->WasKeyJustPressed( KEY_CODE_LEFT_ARROW ) )
	{
		m_selectedAbilityButton -= 1;
	}

	if( m_selectedAbilityButton >= static_cast<int>( m_shownAbilities.size() ) )
	{
		m_firstAbilityToDisplay += 2;
		Clamp( m_firstAbilityToDisplay, 0, Ability::GetNumAbilities() - 2 );
		m_shownAbilities = Ability::GetAbilityList( m_firstAbilityToDisplay, 6 );
		m_selectedAbilityButton -= 2;
	}
	else if( m_selectedAbilityButton < 0 )
	{
		m_firstAbilityToDisplay -= 2;
		Clamp( m_firstAbilityToDisplay, 0, Ability::GetNumAbilities() - 2 );
		m_shownAbilities = Ability::GetAbilityList( m_firstAbilityToDisplay, 6 );
		m_selectedAbilityButton += 2;
	}
	Clamp( m_selectedAbilityButton, 0, 5 );

	for( int buttonIndex = 0; buttonIndex < m_abilityMenuButtons.size(); ++buttonIndex )
	{
		m_abilityMenuButtons[ buttonIndex ]->Update();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderMenu() const
{
	const float abilitySize = HALF_SCREEN_X * 0.33f;
	const float abilityGap = abilitySize * 0.25f;
	const float distanceBetweenAbilities = abilitySize + abilityGap;

	if( m_menuState == MENU_STATE_MAIN )
	{
		Vec3 orthoBottomLeft = m_worldCamera->GetOrthoBottomLeft();
		Vec3 orthoTopRight = m_worldCamera->GetOrthoTopRight();
		AABB2 screenRect = AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );

		std::vector<Vertex_PCU> titleVerts;
		g_devConsoleFont->AddVertsForTextInBox2D( titleVerts, screenRect, 1.5f, "The Trial", Rgba8::WHITE, 1.f, Vec2( 0.5f, 0.8f ) );
		
		g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
		g_theRenderer->BindShader( (Shader*)nullptr );
		g_theRenderer->DrawVertexArray( titleVerts );


		for( int buttonIndex = 0; buttonIndex < m_mainMenuButtons.size(); ++buttonIndex )
		{
			m_mainMenuButtons[ buttonIndex ]->Render();
		}
	}
	else if( m_menuState == MENU_STATE_ABILITY_SELECT )
	{
		for( int buttonIndex = 0; buttonIndex < m_abilityMenuButtons.size(); ++buttonIndex )
		{
			m_abilityMenuButtons[ buttonIndex ]->Render();
		}

		Vec2 scrollButtonSize = Vec2( 0.5f, 1.f );
		Vec2 abilityDimensions = Vec2( abilitySize, abilitySize );
		Vec2 buttonDimensions = Vec2( 4.f, 1.f );

		Vec3 orthoBottomLeft = m_worldCamera->GetOrthoBottomLeft();
		Vec3 orthoTopRight = m_worldCamera->GetOrthoTopRight();
		AABB2 screenRect = AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );

		std::vector<Vec2> abilitySlotPositions;
		Vec2 slotsStartPosition = screenRect.GetPointAtUV( Vec2( 0.5f, 0.75f ) );
		slotsStartPosition -= Vec2( distanceBetweenAbilities * 1.5f, 0.f );
		for( int slotIndex = 0; slotIndex < MAX_ABILITY_COUNT; ++slotIndex )
		{
			abilitySlotPositions.push_back( slotsStartPosition + Vec2( distanceBetweenAbilities, 0.f ) * static_cast<float>( slotIndex ) );
		}

		std::vector<Vec2> abilityButtonPositions;
		Vec2 buttonStartPosition = screenRect.GetPointAtUV( Vec2( 0.25f, 0.4f ) );
		float buttonRowOffset = -1.25f;
		float buttonColumnOffset = HALF_SCREEN_X;
		for( int buttonIndex = 0; buttonIndex < Ability::GetNumAbilities(); ++buttonIndex )
		{
			int buttonRow = buttonIndex / 2;
			int buttonColumn = buttonIndex - ( buttonRow * 2 );
			abilityButtonPositions.push_back( buttonStartPosition + Vec2( buttonColumnOffset * buttonColumn, buttonRowOffset * buttonRow ) );
		}

		AABB2 abilitySlotSelected;
		abilitySlotSelected.SetDimensions( abilityDimensions + Vec2( 0.2f, 0.2f ) );
		abilitySlotSelected.SetCenter( abilitySlotPositions[ m_selectedAbilitySlot ] );

		AABB2 abilityButtonSelected;
		abilityButtonSelected.SetDimensions( buttonDimensions + Vec2( 0.2f, 0.2f ) );
		abilityButtonSelected.SetCenter( abilityButtonPositions[ m_selectedAbilityButton ] );

		AABB2 abilityScrollButtonLeft = screenRect.GetBoxWithin( scrollButtonSize, Vec2( 0.05f, 0.75f ) );
		abilityScrollButtonLeft.SetCenter( Vec2( abilityScrollButtonLeft.GetCenter().x, slotsStartPosition.y ) );
		AABB2 abilityScrollButtonRight = screenRect.GetBoxWithin( scrollButtonSize, Vec2( 0.95f, 0.75f ) );
		abilityScrollButtonRight.SetCenter( Vec2( abilityScrollButtonRight.GetCenter().x, slotsStartPosition.y ) );

		std::vector<Vertex_PCU> abilityMenuVerts;
		std::vector<Vertex_PCU> abilityMenuTextVerts;

		AppendVertsForAABB2D( abilityMenuVerts, abilitySlotSelected, Rgba8::CYAN );
		AppendVertsForAABB2D( abilityMenuVerts, abilityButtonSelected, Rgba8::GREEN );
		AppendVertsForAABB2D( abilityMenuVerts, abilityScrollButtonLeft, Rgba8::WHITE );
		AppendVertsForAABB2D( abilityMenuVerts, abilityScrollButtonRight, Rgba8::WHITE );
		g_devConsoleFont->AddVertsForTextInBox2D( abilityMenuTextVerts, abilityScrollButtonLeft, 0.2f, "<Q", Rgba8::BLUE );
		g_devConsoleFont->AddVertsForTextInBox2D( abilityMenuTextVerts, abilityScrollButtonRight, 0.2f, "E>", Rgba8::BLUE );

		for( int slotIndex = 0; slotIndex < abilitySlotPositions.size(); ++slotIndex )
		{
			AABB2 slotBox;
			slotBox.SetDimensions( abilityDimensions );
			slotBox.SetCenter( abilitySlotPositions[ slotIndex ] );
			AppendVertsForAABB2D( abilityMenuVerts, slotBox, Rgba8::WHITE );
			g_devConsoleFont->AddVertsForTextInBox2D( abilityMenuTextVerts, slotBox, 0.2f, m_selectedAbilities[slotIndex], Rgba8::BLUE );
		}
		
		for( int buttonIndex = 0; buttonIndex < abilityButtonPositions.size(); ++buttonIndex )
		{
			AABB2 buttonBox;
			buttonBox.SetDimensions( buttonDimensions );
			buttonBox.SetCenter( abilityButtonPositions[ buttonIndex ] );
			AppendVertsForAABB2D( abilityMenuVerts, buttonBox, Rgba8::WHITE );

			if( buttonIndex < m_shownAbilities.size() )
			{
				g_devConsoleFont->AddVertsForTextInBox2D( abilityMenuTextVerts, buttonBox, 0.2f, m_shownAbilities[buttonIndex], Rgba8::BLUE );
			}
		}

		g_theRenderer->BindShader( (Shader*)nullptr );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( abilityMenuVerts );

		g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
		g_theRenderer->DrawVertexArray( abilityMenuTextVerts );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::NewGameOnClick()
{
	m_menuState = MENU_STATE_ABILITY_SELECT;
}


//---------------------------------------------------------------------------------------------------------
void Game::BackOnClick()
{
	if( m_gameState != GAME_STATE_MENU )
	{
		m_gameState = GAME_STATE_MENU;
	}

	if( m_menuState != MENU_STATE_MAIN )
	{
		m_menuState = MENU_STATE_MAIN;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ReturnToMenuOnClick()
{
	m_worldCamera->SetPosition( Vec3::ZERO );
	m_UICamera->SetPosition( Vec3::ZERO );
	m_gameState = GAME_STATE_MENU;
	m_menuState = MENU_STATE_MAIN;

	m_gameClock->Resume();
}


//---------------------------------------------------------------------------------------------------------
void Game::StartGameOnClick()
{
	delete m_player;
	m_player = new Player( this );
	m_world->AddPlayerToCurrentMap( m_player );

	for( int selectedAbilityIndex = 0; selectedAbilityIndex < m_selectedAbilities.size(); ++selectedAbilityIndex )
	{
		std::string selectedAbilityName = m_selectedAbilities[ selectedAbilityIndex ];
		m_player->AssignAbilityToSlot( selectedAbilityName, selectedAbilityIndex );
	}
	m_world->SetDifficultyLevel( 0 );
	m_world->LoadNextMap();
	m_gameState = GAME_STATE_PLAYING;
}

//---------------------------------------------------------------------------------------------------------
void Game::ResumeGameOnClick()
{
	m_gameState = GAME_STATE_PLAYING;
	m_gameClock->Resume();
}


//---------------------------------------------------------------------------------------------------------
void Game::QuitOnClick()
{
	m_isQuitting = true;
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdatePauseFromInput()
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		ResumeGameOnClick();
	}

	for( int buttonIndex = 0; buttonIndex < m_pauseMenuButtons.size(); ++buttonIndex )
	{
		m_pauseMenuButtons[buttonIndex]->Update();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderPause() const
{
	m_world->Render();

	Vec3 orthoBottomLeft = m_worldCamera->GetOrthoBottomLeft();
	Vec3 orthoTopRight = m_worldCamera->GetOrthoTopRight();
	AABB2 screenRect = AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );

	std::vector<Vertex_PCU> rectVerts;
	AppendVertsForAABB2D( rectVerts, screenRect, Rgba8( 0, 0, 0, 100 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( rectVerts );
	
	std::vector<Vertex_PCU> titleVerts;
	g_devConsoleFont->AddVertsForTextInBox2D( titleVerts, screenRect, 1.5f, "Paused", Rgba8::WHITE, 1.f, Vec2( 0.5f, 0.8f ) );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( titleVerts );

	for( int buttonIndex = 0; buttonIndex < m_pauseMenuButtons.size(); ++buttonIndex )
	{
		m_pauseMenuButtons[buttonIndex]->Render();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateDeadFromInput()
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		ReturnToMenuOnClick();
	}

	for( int buttonIndex = 0; buttonIndex < m_deadMenuButtons.size(); ++buttonIndex )
	{
		m_deadMenuButtons[buttonIndex]->Update();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderDead() const
{
	m_world->Render();

	Vec3 orthoBottomLeft = m_worldCamera->GetOrthoBottomLeft();
	Vec3 orthoTopRight = m_worldCamera->GetOrthoTopRight();
	AABB2 screenRect = AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );

	std::vector<Vertex_PCU> rectVerts;
	AppendVertsForAABB2D( rectVerts, screenRect, Rgba8( 255, 0, 0, 100 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( rectVerts );

	std::vector<Vertex_PCU> titleVerts;
	g_devConsoleFont->AddVertsForTextInBox2D( titleVerts, screenRect, 0.5f, Stringf( "You Reached Level %i", m_world->GetDifficultyLevel() ), Rgba8::WHITE, 1.f, Vec2( 0.5f, 0.8f ) );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( titleVerts );

	for( int buttonIndex = 0; buttonIndex < m_deadMenuButtons.size(); ++buttonIndex )
	{
		m_deadMenuButtons[buttonIndex]->Render();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadGame()
{
	static int frameCount = 0;

	if( frameCount == 0 )
	{
		++frameCount;
		return;
	}

	LoadAssets();
	m_gameState = GAME_STATE_MENU;
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateStateBasedOnGameState()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );

	switch( m_gameState )
	{
	case GAME_STATE_LOADING:
		break;
	case GAME_STATE_MENU:
		m_cursor->SetState( CURSOR_STATE_MENU );
		break;
	case GAME_STATE_PLAYING:
		m_world->Update( deltaSeconds );
		UpdateCameras();

		if( m_player->IsDead() )
		{
			m_gameClock->Pause();
			m_gameState = GAME_STATE_DEAD;
		}
		break;
	case GAME_STATE_PAUSED:
		m_cursor->SetState( CURSOR_STATE_MENU );
		break;
	case GAME_STATE_DEAD:
		m_cursor->SetState( CURSOR_STATE_MENU );
		break;
	default:
		break;
	}
}
