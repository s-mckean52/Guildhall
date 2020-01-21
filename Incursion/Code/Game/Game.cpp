#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/World.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/World.hpp"
#include "Game/PlayerEntity.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


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
	g_RNG = new RandomNumberGenerator();

	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y) );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_world;
	m_world = nullptr;
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
	if( m_gameState != GAME_STATE_LOADING && m_gameState != GAME_STATE_ATTRACT )
	{
		g_theRenderer->BeginCamera( m_worldCamera );
		m_world->Render();
		g_theRenderer->EndCamera( m_worldCamera );
	}

	//UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_uiCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( m_gameState == GAME_STATE_LOADING && m_loadingFrame == 1 )
	{
		RenderLoadingScreen();
		return;
	}
	else if( m_gameState == GAME_STATE_ATTRACT )
	{
		RenderAttractScreen();
		return;
	}
	else if( m_gameState == GAME_STATE_PAUSED )
	{
		RenderPauseScreen();
	}
	else if( m_gameState == GAME_STATE_VICTORY )
	{
		RenderVictoryScreen();
		return;
	}
	else if( m_gameState == GAME_STATE_DEATH )
	{
		RenderDeathScreen();
		return;
	}
	
	RenderLives();
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderLoadingScreen() const
{
	std::vector<Vertex_PCU> loadingVerts;
	g_testFont->AddVertsForText2D( loadingVerts, Vec2(), 1.f, "Loading..." );
	TransformVertexArray( loadingVerts, 1.f, 0.f, Vec2( HALF_SCREEN_X - 4.5, HALF_SCREEN_Y ) );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( loadingVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderAttractScreen() const
{
	std::vector<Vertex_PCU> attractVerts;
	g_testFont->AddVertsForText2D( attractVerts, Vec2( HALF_SCREEN_X - 4.5f, HALF_SCREEN_Y ), 1.f, "Incursion" );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( attractVerts );

	attractVerts.clear();
	g_testFont->AddVertsForText2D( attractVerts, Vec2( HALF_SCREEN_X - 3.f, HALF_SCREEN_Y - 0.5f ), 0.3f, "Press 'P' or 'Start'" );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( attractVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderPauseScreen() const
{
	std::vector<Vertex_PCU> overlayVerts;
	AppendVertsForAABB2D( overlayVerts, AABB2( 0, 0, CAMERA_SIZE_X, CAMERA_SIZE_Y ), Rgba8( 0, 0, 0, 128 ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( overlayVerts );

	std::vector<Vertex_PCU> pauseTextVerts;
	g_testFont->AddVertsForText2D( pauseTextVerts, Vec2( HALF_SCREEN_X - 3.f, HALF_SCREEN_Y), 1.f, "Paused" );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( pauseTextVerts );

	pauseTextVerts.clear();
	g_testFont->AddVertsForText2D( pauseTextVerts, Vec2( HALF_SCREEN_X - 3.75f, HALF_SCREEN_Y - 0.5f ), 0.25f, "Press 'P' or 'Start' to Resume");
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( pauseTextVerts );

	pauseTextVerts.clear();
	g_testFont->AddVertsForText2D( pauseTextVerts, Vec2( HALF_SCREEN_X - 3.625f, HALF_SCREEN_Y - 0.75f ), 0.25f, "Press 'Esc' or 'Back' to Quit");
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( pauseTextVerts );

}


//---------------------------------------------------------------------------------------------------------
void Game::RenderVictoryScreen() const
{
	std::vector<Vertex_PCU> victoryOverlayVerts;
	unsigned char overlayAlpha = static_cast<unsigned char>(255.f * m_victoryOverlayAlphaFraction );
	AppendVertsForAABB2D( victoryOverlayVerts, AABB2( 0, 0, CAMERA_SIZE_X, CAMERA_SIZE_Y ), Rgba8( 210, 180, 140, overlayAlpha ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( victoryOverlayVerts );

	if( m_victoryOverlayTimer > DELAY_BEFORE_VICTORY_SECONDS )
	{
		std::vector<Vertex_PCU> victoryTextVerts;
		g_testFont->AddVertsForText2D( victoryTextVerts, Vec2( HALF_SCREEN_X - 3.5f, HALF_SCREEN_Y ), 1.f, "You Win", Rgba8::GREEN );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( victoryTextVerts );

		victoryTextVerts.clear();
		g_testFont->AddVertsForText2D( victoryTextVerts, Vec2( HALF_SCREEN_X - 4.5f, HALF_SCREEN_Y - 0.5f ), 0.25f, "Press 'Esc', 'P', 'Start', or 'Back'", Rgba8::GREEN );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( victoryTextVerts );

		victoryTextVerts.clear();
		g_testFont->AddVertsForText2D( victoryTextVerts, Vec2( HALF_SCREEN_X - 4.5f, HALF_SCREEN_Y - 0.75f ), 0.25f, "   to Return to the Title Screen", Rgba8::GREEN );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( victoryTextVerts );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderDeathScreen() const
{
	std::vector<Vertex_PCU> deathOverlayVerts;
	unsigned char overlayAlpha = static_cast<unsigned char>( 255.f * m_deathOverlayAlphaFraction );
	AppendVertsForAABB2D( deathOverlayVerts, AABB2( 0, 0, CAMERA_SIZE_X, CAMERA_SIZE_Y ), Rgba8( 0, 0, 0, overlayAlpha ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( deathOverlayVerts );

	if( m_deathStateTimeSeconds > DELAY_AFTER_DEATH_SECONDS )
	{
		std::vector<Vertex_PCU> deathTextVerts;
		g_testFont->AddVertsForText2D( deathTextVerts, Vec2( HALF_SCREEN_X - 6.5f, HALF_SCREEN_Y ), 1.f, "You Have Died", Rgba8::RED );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( deathTextVerts );

		deathTextVerts.clear();
		g_testFont->AddVertsForText2D( deathTextVerts, Vec2( HALF_SCREEN_X - 3.75f, HALF_SCREEN_Y - 0.5f ), 0.25f, "Press 'Start' or 'P' to Resume", Rgba8::RED );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( deathTextVerts );

		deathTextVerts.clear();
		g_testFont->AddVertsForText2D( deathTextVerts, Vec2( HALF_SCREEN_X - 3.625f, HALF_SCREEN_Y - 0.75f ), 0.25f, "Press 'Back' or 'Esc' to Quit", Rgba8::RED );
		g_theRenderer->BindTexture( g_testFont->GetTexture() );
		g_theRenderer->DrawVertexArray( deathTextVerts );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderLives() const
{
	float lifePositionX = 1.f;
	float lifePositionY = CAMERA_SIZE_Y - 1.f;

	PlayerEntity* player = static_cast<PlayerEntity*>(m_world->GetCurrentMapEntity( ENTITY_TYPE_PLAYER, 0 ));
	for( int playerLifeCount = 0; playerLifeCount < player->GetRemainingLives(); ++playerLifeCount )
	{
		Vec2 position( lifePositionX + (1.f * playerLifeCount), lifePositionY );
		RenderTankIcon( position, Rgba8::WHITE );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderTankIcon( const Vec2& position, const Rgba8& tint ) const
{
	std::vector<Vertex_PCU> lifeIconVerts;

	AppendVertsForAABB2D( lifeIconVerts, AABB2(), tint, Vec2( 0.f, 0.f), Vec2( 1.f, 1.f ) );
	
	TransformVertexArray( static_cast<int>( lifeIconVerts.size() ), &lifeIconVerts[ 0 ], 1.f, 90.f, position );

	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->BindTexture( tankBaseTexture );
	g_theRenderer->DrawVertexArray( lifeIconVerts );


	Texture* tankTurretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->BindTexture( tankTurretTexture );
	g_theRenderer->DrawVertexArray( lifeIconVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	UpdateGameStatesFromInput();

	if( m_isSlowMo )
	{
		deltaSeconds *= 0.1f;
	}

	if( m_isFastMo )
	{
		deltaSeconds *= 4.f;
	}


	if( m_gameState == GAME_STATE_LOADING )
	{
		UpdateLoadingState( deltaSeconds );
		return; 
	}

	if( m_gameState == GAME_STATE_ATTRACT )
	{
		UpdateAttractState( deltaSeconds );
	}

	if( m_gameState == GAME_STATE_DEATH )
	{
		UpdateDeathState( deltaSeconds );
	}

	if( m_gameState == GAME_STATE_VICTORY )
	{
		UpdateVictoryState( deltaSeconds );
	}

	if( m_gameState == GAME_STATE_DEATH || m_gameState == GAME_STATE_PLAYING )
	{
		m_world->Update( deltaSeconds );
	}

	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateLoadingState( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	m_loadingFrame += 1;

	if( m_loadingFrame == 2 )
	{
		LoadAssets();
		SetGameState( GAME_STATE_ATTRACT );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateAttractState( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if( !m_world )
	{
		m_world = new World( this );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateVictoryState( float deltaSeconds )
{
	m_victoryOverlayTimer += deltaSeconds;
	if( m_victoryOverlayTimer < DELAY_BEFORE_VICTORY_SECONDS )
	{
		m_victoryOverlayAlphaFraction += Lerp( 0.f, 1.f, deltaSeconds / DELAY_BEFORE_VICTORY_SECONDS );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateDeathState( float deltaSeconds )
{
	if( m_deathStateTimeSeconds < DELAY_AFTER_DEATH_SECONDS )
	{
		m_deathStateTimeSeconds += deltaSeconds;
		m_deathOverlayAlphaFraction += Lerp( 0.f, 0.4f, deltaSeconds / DELAY_AFTER_DEATH_SECONDS );
		m_isRespawning = false;
	}
	else if( m_isRespawning )
	{
		m_respawnDelaySeconds += deltaSeconds;
		m_deathOverlayAlphaFraction -= Lerp( 0.f, 0.4f, deltaSeconds / DELAY_BEFORE_RESPAWN_SECONDS );
	}

	if( m_respawnDelaySeconds > DELAY_BEFORE_RESPAWN_SECONDS )
	{
		PlayerEntity* player = static_cast<PlayerEntity*>(m_world->GetCurrentMapEntity( ENTITY_TYPE_PLAYER, 0 ) );
		player->Respawn();
		m_deathOverlayAlphaFraction = 0.f;
		m_deathStateTimeSeconds = 0.f;
		m_respawnDelaySeconds = 0.f;
		m_isRespawning = false;

		SetGameState( GAME_STATE_PLAYING );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStatesFromInput()
{
	const XboxController& firstPlayerController = g_theInput->GetXboxController( 0 );

	switch( m_gameState )
	{
	case GAME_STATE_LOADING:
		break;

	case GAME_STATE_ATTRACT:
		UpdateAttractFromInput( firstPlayerController );
		break;

	case GAME_STATE_PLAYING:
		UpdatePlayingFromInput( firstPlayerController );
		break;

	case GAME_STATE_DEATH:
		UpdateDeathFromInput( firstPlayerController );
		break;

	case GAME_STATE_VICTORY:
		UpdateVictoryFromInput( firstPlayerController );
		break;

	case GAME_STATE_PAUSED:
		UpdatePausedFromInput( firstPlayerController );
		break;

	default:
		ERROR_AND_DIE( Stringf( "Invalid Game State #%i", m_gameState ) );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateVictoryFromInput( const XboxController& firstPlayerController )
{
	if( m_victoryOverlayTimer < DELAY_BEFORE_VICTORY_SECONDS + 1.f ) return;

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}

	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}

	if( !firstPlayerController.IsConnected() ) return;

	const KeyButtonState& startButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_START );
	if( startButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}

	const KeyButtonState& backButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_BACK );
	if( backButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateDeathFromInput( const XboxController& firstPlayerController )
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}

	if( !firstPlayerController.IsConnected() ) return;

	const KeyButtonState& backButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_BACK );
	if( backButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_ATTRACT;
		DeleteCurrentWorld();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdatePausedFromInput( const XboxController& firstPlayerController )
{
	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		m_gameState = GAME_STATE_PLAYING;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_gameState = GAME_STATE_ATTRACT;
	}

	if( !firstPlayerController.IsConnected() ) return;

	const KeyButtonState& startButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_START );
	if( startButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_PLAYING;
	}

	const KeyButtonState& backButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_BACK );
	if( backButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_ATTRACT;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateAttractFromInput( const XboxController& firstPlayerController )
{
	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		m_gameState = GAME_STATE_PLAYING;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}

	if( !firstPlayerController.IsConnected() ) return;

	const KeyButtonState& startButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_START );
	if( startButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_PLAYING;
	}

	const KeyButtonState& backButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_BACK );
	if( backButton.WasJustPressed() )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdatePlayingFromInput( const XboxController& firstPlayerController )
{
	if( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_gameState = GAME_STATE_PAUSED;
	}

	if( g_theInput->IsKeyPressed( 'T' ) )
	{
		m_isSlowMo = true;
	}
	else
	{
		m_isSlowMo = false;
	}

	if( g_theInput->IsKeyPressed( 'Y' ) )
	{
		m_isFastMo = true;
	}
	else
	{
		m_isFastMo = false;
	}

	if( !firstPlayerController.IsConnected() ) return;

	const KeyButtonState& startButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_START );
	if( startButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_PAUSED;
	}

	const KeyButtonState& backButton = firstPlayerController.GetButtonState( XBOX_BUTTON_ID_BACK );
	if( backButton.WasJustPressed() )
	{
		m_gameState = GAME_STATE_PAUSED;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	//Update m_worldCamera
	if( g_isDebugCamera )
	{
		IntVec2 currentMapDimensions = m_world->GetCurrentMapDimensions();
		if( currentMapDimensions.x < currentMapDimensions.y )
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( static_cast<float>( currentMapDimensions.y ) * CLIENT_ASPECT, static_cast<float>( currentMapDimensions.y ) ) );
		}
		else
		{
			m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( static_cast<float>( currentMapDimensions.x ), static_cast<float>( currentMapDimensions.x ) / CLIENT_ASPECT ) );
		}
	}
	else
	{
		Vec2 mapDimensions = Vec2( static_cast<float>(m_world->GetCurrentMapDimensions().x), static_cast<float>(m_world->GetCurrentMapDimensions().y) );

		m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );

		Vec2 player1Positon = m_world->GetCurrentMapEntity( ENTITY_TYPE_PLAYER, 0 )->GetPosition();
		float worldCameraPositionX = player1Positon.x - HALF_SCREEN_X;
		float worldCameraPositionY = player1Positon.y - HALF_SCREEN_Y;

		Clamp( worldCameraPositionX, 0.f, mapDimensions.x - CAMERA_SIZE_X );
		Clamp( worldCameraPositionY, 0.f, mapDimensions.y - CAMERA_SIZE_Y );

		m_worldCamera.Translate2D( Vec2( worldCameraPositionX, worldCameraPositionY ) );
	}
	
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
}


//---------------------------------------------------------------------------------------------------------
void Game::SetIsRespawning( bool isRespawning )
{
	m_isRespawning = isRespawning;
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadTextures()
{
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Bullet.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretTop.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankTop.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/Playerhit.wav" );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
}


//---------------------------------------------------------------------------------------------------------
void Game::SetGameState( GameState newGameState )
{
	m_gameState = newGameState;
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteCurrentWorld()
{
	if( m_world )
	{
		delete m_world;
		m_world = nullptr;
	}
}
