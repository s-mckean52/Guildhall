#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Debris.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/App.hpp"

RandomNumberGenerator *g_RNG = nullptr;

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
	m_gameState = ATTRACT_STATE;
	
	g_RNG = new RandomNumberGenerator();

	m_playerShip = new PlayerShip( this, Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_playerShip;
	m_playerShip = nullptr;

	DeleteAllAsteroids();
	DeleteAllBullets();
	DeleteAllBeetles();
	DeleteAllWasps();
	DeleteAllDebris();
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnWave()
{
	if( m_wave == NUM_OF_WAVES )
	{
 		m_wave = INVALID_WAVE_NUM;
		m_gameState = RESTARTING_STATE;
	}
	else if( m_wave != INVALID_WAVE_NUM )
	{
		int numberOfAsteroidsInWave = NUM_STARTING_ASTEROIDS * m_wave;
		int numberOfBeetlesInWave	= m_wave + 1;
		int numberOfWaspsInWave		= m_wave - 1;

		for( int initialAsteroidCount = 0; initialAsteroidCount < numberOfAsteroidsInWave; ++initialAsteroidCount )
		{
			SpawnAsteroid();
		}

		for( int initialBeetleCount = 0; initialBeetleCount < numberOfBeetlesInWave; ++initialBeetleCount )
		{
			SpawnBeetle();
		}

		for( int initialWaspCount = 0; initialWaspCount < numberOfWaspsInWave; ++initialWaspCount )
		{
			SpawnWasp();
		}		
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteAllAsteroids()
{
	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		Asteroid *currentAsteroid = m_asteroids[ asteroidIndex ];
		if( currentAsteroid != nullptr )
		{
			delete currentAsteroid;
			currentAsteroid = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteAllBullets()
{
	for( int bulletIndex = 0; bulletIndex < MAX_BULLET_COUNT; ++bulletIndex )
	{
		Bullet *currentBullet = m_bullets[ bulletIndex ];
		if( currentBullet != nullptr )
		{
			delete currentBullet;
			currentBullet = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteAllBeetles()
{
	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		Beetle *currentBeetle = m_beetles[ beetleIndex ];
		if( currentBeetle != nullptr )
		{
			delete currentBeetle;
			currentBeetle = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteAllWasps()
{
	for( int waspIndex = 0; waspIndex < MAX_WASP_COUNT; ++waspIndex )
	{
		Wasp *currentWasp = m_wasps[ waspIndex ];
		if( currentWasp != nullptr )
		{
			delete currentWasp;
			currentWasp = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteAllDebris()
{
	for( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_COUNT; ++debrisIndex )
	{
		Debris *currentDebris = m_debris[ debrisIndex ];
		if( currentDebris != nullptr )
		{
			delete currentDebris;
			currentDebris = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnBullet( Vec2 location, float rotation )
{
	Bullet *newBullet = new Bullet( this, location, rotation );
	for( int bulletIndex = 0; bulletIndex < MAX_BULLET_COUNT; ++bulletIndex )
	{
		if( m_bullets[ bulletIndex ] == nullptr )
		{
			m_bullets[ bulletIndex ] = newBullet;
			return;
		}
	}
	ERROR_RECOVERABLE( Stringf( "Cannot spawn Bullet; maximum number of Bullets is %i", MAX_BULLET_COUNT ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnAsteroid( )
{
	m_enemiesRemaining++;
	Asteroid *newAsteroid = new Asteroid( this );
	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		if( m_asteroids[asteroidIndex] == nullptr )
		{
			m_asteroids[ asteroidIndex ] = newAsteroid;
			return;
		}
	}
	ERROR_RECOVERABLE( Stringf( "Cannot spawn Asteroid; maximum number of Asteroids is %i", MAX_ASTEROID_COUNT ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnBeetle()
{
	m_enemiesRemaining++;
	float beetlePositionY = g_RNG->GetRandomFloatInRange( 0.f, CAMERA_SIZE_Y );	
	
	int spawnSide = g_RNG->GetRandomIntInRange( 0, 1 );
	float beetlePositionX = CAMERA_SIZE_X * spawnSide;
	if( spawnSide == 0 )
	{
		beetlePositionX -= BEETLE_COSMETIC_RADIUS;
	}
	else
	{
		beetlePositionX += BEETLE_COSMETIC_RADIUS;
	}

	Beetle *newBeetle = new Beetle( this, Vec2( beetlePositionX, beetlePositionY ) );
	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		if( m_beetles[beetleIndex] == nullptr )
		{
			m_beetles[ beetleIndex ] = newBeetle;
			return;
		}
	}
	ERROR_RECOVERABLE( Stringf( "Cannot spawn Asteroid; maximum number of Beetles is %i", MAX_BEETLE_COUNT ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnWasp()
{
	m_enemiesRemaining++;
	float waspPositionY = g_RNG->GetRandomFloatInRange( 0.f, CAMERA_SIZE_Y );

	int spawnSide = g_RNG->GetRandomIntInRange( 0, 1 );
	float waspPositionX = CAMERA_SIZE_X * spawnSide;
	if( spawnSide == 0 )
	{
		waspPositionX -= WASP_COSMETIC_RADIUS;
	}
	else
	{
		waspPositionX += WASP_COSMETIC_RADIUS;
	}

	Wasp *newWasp = new Wasp( this, Vec2( waspPositionX, waspPositionY ) );
	for( int waspIndex = 0; waspIndex < MAX_BEETLE_COUNT; ++waspIndex )
	{
		if( m_wasps[ waspIndex ] == nullptr )
		{
			m_wasps[ waspIndex ] = newWasp;
			return;
		}
	}
	ERROR_RECOVERABLE( Stringf( "Cannot spawn Asteroid; maximum number of Wasps is %i", MAX_WASP_COUNT ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::SpawnDebris( const Entity* dyingEntity, const Entity* killingEntity )
{
	int numberOfDebrisPieces = dyingEntity->GetNumberOfDebris();
	float maxDebrisRadius = dyingEntity->GetPhysicsRadius() * DEBRIS_SIZE_PERCENTAGE;
	Vec2 spawnPosition = dyingEntity->GetPosition();
	Rgba8 debrisColor = dyingEntity->GetColor();
	Vec2 debrisSpeed = dyingEntity->GetForwardVector() + killingEntity->GetForwardVector();
	
	int debrisPiecesIndex = 0;

 	for( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_COUNT; ++debrisIndex )
	{
		while( debrisPiecesIndex < numberOfDebrisPieces && debrisIndex < MAX_DEBRIS_COUNT)
		{
			if( m_debris[ debrisIndex ] == nullptr )
			{
				Debris *newDebris = new Debris( this, spawnPosition, debrisSpeed, debrisColor, maxDebrisRadius );
				m_debris[ debrisIndex ] = newDebris;
				++debrisPiecesIndex;
			}
			++debrisIndex;
		}
		if( debrisPiecesIndex >= numberOfDebrisPieces )
		{
			return;
		}
	}
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
	if( m_gameState != ATTRACT_STATE )
	{
		g_theRenderer->BeginCamera( m_worldCamera );
		RenderEntities();
		g_theRenderer->EndCamera( m_worldCamera );
	}

	//UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );
	RenderUI();
	g_theRenderer->EndCamera( m_uiCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	//Draw Bullets
	for( int numBullets = 0; numBullets < MAX_BULLET_COUNT; ++numBullets )
	{
		Bullet *currentBullet = m_bullets[ numBullets ];
		if( currentBullet != nullptr )
		{
			currentBullet->Render();

			if( m_gameState == DEBUG_STATE )
			{
				DrawLineBetweenEntities( currentBullet, m_playerShip );
			}
		}
	}

	//Asteroid Render
	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		Asteroid* currentAsteroid = m_asteroids[ asteroidIndex ];
		if( currentAsteroid != nullptr )
		{
			currentAsteroid->Render();

			if( m_gameState == DEBUG_STATE )
			{
				DrawLineBetweenEntities( currentAsteroid, m_playerShip );
			}
		}
	}

	//Beetle Render
	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		Beetle* currentBeetle = m_beetles[ beetleIndex ];
		if( currentBeetle != nullptr )
		{
			currentBeetle->Render();

			if( m_gameState == DEBUG_STATE )
			{
				DrawLineBetweenEntities( currentBeetle, m_playerShip );
			}
		}
	}

	//Wasp Render
	for( int waspIndex = 0; waspIndex < MAX_WASP_COUNT; ++waspIndex )
	{
		Wasp* currentWasp = m_wasps[ waspIndex ];
		if( currentWasp != nullptr )
		{
 			currentWasp->Render();

			if( m_gameState == DEBUG_STATE )
			{
				DrawLineBetweenEntities( currentWasp, m_playerShip );
			}
		}
	}

	//Debris Render
	for( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_COUNT; ++debrisIndex )
	{
		Debris* currentDebris = m_debris[ debrisIndex ];
		if( currentDebris != nullptr )
		{
			currentDebris->Render();

			if( m_gameState == DEBUG_STATE )
			{
				DrawLineBetweenEntities( currentDebris, m_playerShip );
			}
		}
	}

	m_playerShip->Render();
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	if( m_gameState == ATTRACT_STATE )
	{
		RenderStarshipTitle();
	}
	else
	{
		float lifePositionX = 2.f;
		float lifePositionY = CAMERA_SIZE_Y - 2.f;

		for( int playerLifeCount = 0; playerLifeCount < m_playerShip->GetLivesRemaining(); ++playerLifeCount )
		{
			Vec2 position( lifePositionX + (3 * playerLifeCount), lifePositionY );
			RenderLifeIcon( position );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	if( g_theInput->WasKeyJustPressed( KEYCODE_ESCAPE ) )
	{
		g_theApp->HandleQuitRequested();
	}

	if( m_gameState == ATTRACT_STATE )
	{
		UpdateCameras( deltaSeconds );

		if( g_theInput->WasKeyJustPressed( 'N' ) || g_theInput->WasKeyJustPressed( KEYCODE_SPACE ) )
		{
			SetGameState( PLAY_STATE );
		}
	}
	else if( m_gameState == PAUSED_STATE ) return;
	else
	{
		UpdateEntities( deltaSeconds );

		UpdateCameras( deltaSeconds );

		if( m_enemiesRemaining <= 0 )
		{
			m_wave = static_cast<Waves>( static_cast<int>(m_wave) + 1 );
			SpawnWave();
		}
		
		if( m_playerShip->GetLivesRemaining() <= 0 )
		{
			m_restartDelaySeconds -= deltaSeconds;
			if( m_restartDelaySeconds <= 0.f )
			{
				m_gameState = RESTARTING_STATE;
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateEntities( float deltaSeconds )
{
	//Update Bullets
	for( int numBullets = 0; numBullets < MAX_BULLET_COUNT; ++numBullets )
	{
		Bullet* currentBullet = m_bullets[ numBullets ];
		if( currentBullet != nullptr )
		{
			currentBullet->Update( deltaSeconds );
		}
	}

	//Update Asteroids
	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		Asteroid* currentAsteroid = m_asteroids[ asteroidIndex ];
		if( currentAsteroid != nullptr )
		{
			currentAsteroid->Update( deltaSeconds );
		}
	}

	//Update Beetles
	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		Beetle* currentBeetle = m_beetles[ beetleIndex ];
		if( currentBeetle != nullptr )
		{
			currentBeetle->Update( deltaSeconds );
		}
	}

	//Update Wasps
	for( int waspIndex = 0; waspIndex < MAX_WASP_COUNT; ++waspIndex )
	{
		Wasp* currentWasp = m_wasps[ waspIndex ];
		if( currentWasp != nullptr )
		{
			currentWasp->Update( deltaSeconds );
		}
	}

	//Update Debris
	for( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_COUNT; ++debrisIndex )
	{
		Debris* currentDebris = m_debris[ debrisIndex ];
		if( currentDebris != nullptr )
		{
			currentDebris->Update( deltaSeconds );
		}
	}

	//Update Player.
	if( m_playerShip->GetLivesRemaining() > 0 )
	{
		m_playerShip->Update( deltaSeconds );
	}

	CheckCollisions();

	DeleteGarbageEntities();
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	//Update m_worldCamera
	m_worldCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
	
	m_screenShakeIntensity -= SCREEN_SHAKE_ABBERATION * deltaSeconds;
	m_screenShakeIntensity = GetClamp( m_screenShakeIntensity, 0.f, 1.f );
	float maxShakeIntensity = MAX_SCREEN_SHAKE_DISPLACEMENT * m_screenShakeIntensity;
	float cameraTranslateX = g_RNG->GetRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	float cameraTranslateY = g_RNG->GetRandomFloatInRange( -maxShakeIntensity, maxShakeIntensity );
	m_worldCamera.Translate2D( Vec2( cameraTranslateX, cameraTranslateY ) );
	
	//Update m_uiCamera
	m_uiCamera.SetOrthoView( Vec2( 0, 0 ), Vec2( CAMERA_SIZE_X, CAMERA_SIZE_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckCollisions()
{
	Bullet *currentBullet = nullptr;

	for( int bulletIndex = 0; bulletIndex < MAX_BULLET_COUNT; ++bulletIndex )
	{
		currentBullet = m_bullets[ bulletIndex ];
		if( currentBullet != nullptr )
		{
			CheckAsteroidCollisionsWithEntity( currentBullet );
			CheckBeetleCollisionsWithEntity( currentBullet );
			CheckWaspCollisionsWithEntity( currentBullet );
		}
	}

	CheckAsteroidCollisionsWithEntity( m_playerShip );
	CheckBeetleCollisionsWithEntity( m_playerShip );
	CheckWaspCollisionsWithEntity( m_playerShip );
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckAsteroidCollisionsWithEntity( Entity* collider )
{
	Asteroid *currentAsteroid;

	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		currentAsteroid = m_asteroids[ asteroidIndex ];

		if( currentAsteroid != nullptr && DoEntitiesOverlap( currentAsteroid, collider ) )
		{
 			collider->TakeDamage( 1 );
			if( collider->IsDead() )
			{
				SpawnDebris( collider, currentAsteroid );
			}

			currentAsteroid->TakeDamage( 1 );
			if( currentAsteroid->IsDead() )
			{
				SpawnDebris( currentAsteroid, collider );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckBeetleCollisionsWithEntity( Entity* collider )
{
	Beetle* currentBeetle;

	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		currentBeetle = m_beetles[beetleIndex];

		if( currentBeetle != nullptr && DoEntitiesOverlap( currentBeetle, collider ) )
		{
			collider->TakeDamage( 1 );
			if( collider->IsDead() )
			{
				SpawnDebris( collider, currentBeetle );
			}

			currentBeetle->TakeDamage( 1 );
			if( currentBeetle->IsDead() )
			{
				SpawnDebris( currentBeetle, collider );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckWaspCollisionsWithEntity( Entity* collider )
{
	Wasp* currentWasp;

	for( int waspIndex = 0; waspIndex < MAX_WASP_COUNT; ++waspIndex )
	{
		currentWasp = m_wasps[ waspIndex ];

		if( currentWasp != nullptr && DoEntitiesOverlap( currentWasp, collider ) )
		{
			collider->TakeDamage( 1 );
			if( collider->IsDead() )
			{
				SpawnDebris( collider, currentWasp );
			}

			currentWasp->TakeDamage( 1 );
			if( currentWasp->IsDead() )
			{
				SpawnDebris( currentWasp, collider );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Game::DoEntitiesOverlap( Entity* A, Entity* B)
{
	if( !A->IsDead() && !B->IsDead())
	{
		return DoDiscsOverlap( A->GetPosition(), A->GetPhysicsRadius(), B->GetPosition(), B->GetPhysicsRadius() );
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawLineBetweenEntities( Entity* A, Entity* B ) const
{
	DrawLineBetweenPoints( A->GetPosition(), B->GetPosition(), Rgba8( 50, 50, 50, 255 ), 0.15f );
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteGarbageEntities()
{
	//Delete Bullets
	for( int numBullets = 0; numBullets < MAX_BULLET_COUNT; ++numBullets )
	{
		Bullet* currentBullet = m_bullets[ numBullets ];
		if( currentBullet != nullptr && currentBullet->IsGarbage() )
		{
			delete currentBullet;
			m_bullets[numBullets] = nullptr;
		}
	}

	//Delete Asteroids
	for( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROID_COUNT; ++asteroidIndex )
	{
		Asteroid* currentAsteroid = m_asteroids[ asteroidIndex ];
		if( currentAsteroid != nullptr && currentAsteroid->IsGarbage() )
		{
			delete currentAsteroid;
			m_asteroids[ asteroidIndex ] = nullptr;
		}
	}

	//Delete Beetles
	for( int beetleIndex = 0; beetleIndex < MAX_BEETLE_COUNT; ++beetleIndex )
	{
		Beetle* currentBeetle = m_beetles[ beetleIndex ];
		if( currentBeetle != nullptr && currentBeetle->IsGarbage() )
		{
			delete currentBeetle;
			m_beetles[ beetleIndex ] = nullptr;
		}
	}

	//Delete Wasps
	for( int waspIndex = 0; waspIndex < MAX_WASP_COUNT; ++waspIndex )
	{
		Wasp* currentWasp = m_wasps[ waspIndex ];
		if( currentWasp != nullptr && currentWasp->IsGarbage() )
		{
			delete currentWasp;
			m_wasps[ waspIndex ] = nullptr;
		}
	}

	//Delete Debris
	for( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_COUNT; ++debrisIndex )
	{
		Debris* currentDebris = m_debris[ debrisIndex ];
		if( currentDebris != nullptr && currentDebris->IsGarbage() )
		{
			delete currentDebris;
			m_debris[ debrisIndex ] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DecrementEnemyCount()
{
	m_enemiesRemaining--;
}


//---------------------------------------------------------------------------------------------------------
void Game::SetGameState( GameState newGameState )
{
	m_gameState = newGameState;
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderLifeIcon( const Vec2& position ) const
{
	Rgba8 lifeColor = RGBA8_BALI_HAI;
	lifeColor.a = 175;

	Vertex_PCU playerShipLife[] =
	{
		// Nose
		Vertex_PCU( Vec2( 0.f, -1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 1.f, 0.f ),	lifeColor ),

		// Body
		Vertex_PCU( Vec2( -2.f, -1.f ), lifeColor ),
		Vertex_PCU( Vec2( 0.f, -1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	lifeColor ),

		Vertex_PCU( Vec2( -2.f, -1.f ), lifeColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	lifeColor ),
		Vertex_PCU( Vec2( -2.f, 1.f ),	lifeColor ),

		// Left Wing
		Vertex_PCU( Vec2( -2.f, 1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 2.f, 1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 0.f, 2.f ),	lifeColor ),

		//Right Wing
		Vertex_PCU( Vec2( -2.f, -1.f ),	lifeColor ),
		Vertex_PCU( Vec2( 0.f, -2.f ),	lifeColor ),
		Vertex_PCU( Vec2( 2.f, -1.f ),	lifeColor )
	};

	constexpr int SHIP_VERT_SIZE = sizeof( playerShipLife ) / sizeof( playerShipLife[0] );

	TransformVertexArray( SHIP_VERT_SIZE, playerShipLife, 0.5f, 0.f, position );

	g_theRenderer->DrawVertexArray( SHIP_VERT_SIZE, playerShipLife );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderStarshipTitle() const
{
	Rgba8 titleColor = RGBA8_CYAN;
	float titleThickness = 0.7f;
	Vec2 startPosition( STARSHIP_TITLE_START_X, STARSHIP_TITLE_START_Y );

	//S
	DrawLineBetweenPoints( Vec2( 10.f, 10.f ) + startPosition,  Vec2( 0.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 0.f, 10.f ) + startPosition,  Vec2( 0.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 0.f, 0.f ) + startPosition,  Vec2( 10.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 10.f, 0.f ) + startPosition,  Vec2( 10.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 10.f, -10.f ) + startPosition,  Vec2( 0.f, -10.f ) + startPosition, titleColor, titleThickness );

	//T
	DrawLineBetweenPoints( Vec2( 15.f, 10.f ) + startPosition,  Vec2( 25.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 20.f, 10.f ) + startPosition,  Vec2( 20.f, -10.f ) + startPosition, titleColor, titleThickness );
	
	//A
	DrawLineBetweenPoints( Vec2( 30.f, -10.f ) + startPosition,  Vec2( 30.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 30.f, 10.f ) + startPosition,  Vec2( 40.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 40.f, 10.f ) + startPosition,  Vec2( 40.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 30.f, 0.f ) + startPosition,  Vec2( 40.f, 0.f ) + startPosition, titleColor, titleThickness );
	
	//R
	DrawLineBetweenPoints( Vec2( 45.f, -10.f ) + startPosition,  Vec2( 45.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 45.f, 10.f ) + startPosition,  Vec2( 55.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 55.f, 10.f ) + startPosition,  Vec2( 55.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 55.f, 0.f ) + startPosition,  Vec2( 45.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 45.f, 0.f ) + startPosition,  Vec2( 55.f, -10.f ) + startPosition, titleColor, titleThickness );
	
	//S
	DrawLineBetweenPoints( Vec2( 70.f, 10.f ) + startPosition, Vec2( 60.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 60.f, 10.f ) + startPosition, Vec2( 60.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 60.f, 0.f ) + startPosition, Vec2( 70.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 70.f, 0.f ) + startPosition, Vec2( 70.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 70.f, -10.f ) + startPosition, Vec2( 60.f, -10.f ) + startPosition, titleColor, titleThickness );

	//H
	DrawLineBetweenPoints( Vec2( 75.f, 10.f ) + startPosition,  Vec2( 75.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 85.f, 10.f ) + startPosition,  Vec2( 85.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 75.f, 0.f ) + startPosition,  Vec2( 85.f, 0.f ) + startPosition, titleColor, titleThickness );

	//I
	DrawLineBetweenPoints( Vec2( 90.f, 10.f ) + startPosition,  Vec2( 100.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 90.f, -10.f ) + startPosition,  Vec2( 100.f, -10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 95.f, 10.f ) + startPosition,  Vec2( 95.f, -10.f ) + startPosition, titleColor, titleThickness );

	//P
	DrawLineBetweenPoints( Vec2( 105.f, -10.f ) + startPosition,  Vec2( 105.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 105.f, 10.f ) + startPosition,  Vec2( 115.f, 10.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 115.f, 10.f ) + startPosition,  Vec2( 115.f, 0.f ) + startPosition, titleColor, titleThickness );
	DrawLineBetweenPoints( Vec2( 115.f, 0.f ) + startPosition,  Vec2( 105.f, 0.f ) + startPosition, titleColor, titleThickness );
}