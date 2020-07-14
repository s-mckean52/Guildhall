#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Cursor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame )
{
	m_theGame = theGame;
	std::string mapFolderPath = "Data/Maps/";
	Strings mapFilePaths = GetFileNamesInFolder( mapFolderPath, "*.png" );
	for( int mapPathIndex = 0; mapPathIndex < mapFilePaths.size(); ++mapPathIndex )
	{
		std::string currentMapFilePath = mapFolderPath + mapFilePaths[ mapPathIndex ];
		Map* newMap = new Map( theGame, this, currentMapFilePath.c_str() );
		m_maps.push_back( newMap );
	}
	m_currentMap = m_maps[m_currentMapIndex];

	UpdateNewLevelProgress();
}


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	delete m_currentMap;
	m_currentMap = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void World::CleanUpDeadEntities()
{
	m_currentMap->CleanUpEntities();
}


//---------------------------------------------------------------------------------------------------------
void World::Render() const
{
	m_currentMap->Render();

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void World::RenderProgressBar() const
{
	Camera* uiCamera = m_theGame->GetUICamera();
	Vec3 uiBottomLeft = uiCamera->GetOrthoBottomLeft();
	Vec3 uiTopRight = uiCamera->GetOrthoTopRight();
	AABB2 uiCameraRect = AABB2( uiBottomLeft.x, uiBottomLeft.y, uiTopRight.x, uiTopRight.y );

	Vec2 progressBarDimensions = Vec2( uiCameraRect.GetDimensions().x * 0.8f, 0.1f );

	AABB2 progressBarBase = uiCameraRect.GetBoxWithin( progressBarDimensions, Vec2( 0.5f, 0.95f ) );
	
	AABB2 progressBarBackground = progressBarBase;
	progressBarBackground.SetDimensions( progressBarDimensions + Vec2( 0.05f, 0.05f ) );

	float progressFraction = static_cast<float>( m_currentScore ) / static_cast<float>( m_scoreToProceed );
	Clamp( progressFraction, 0.f, 1.f );
	AABB2 progressBar = progressBarBase.GetBoxAtLeft( progressFraction );

	std::vector<Vertex_PCU> progressBarVerts;
	AppendVertsForAABB2D( progressBarVerts, progressBarBackground, Rgba8( 0, 0, 0, 127 ) );
	AppendVertsForAABB2OutlineAtPoint( progressBarVerts, progressBarBackground, Rgba8::WHITE, 0.05f );
	AppendVertsForAABB2D( progressBarVerts, progressBar, Rgba8::BLUE );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( progressBarVerts );
}


//---------------------------------------------------------------------------------------------------------
void World::DebugRender() const
{
}


//---------------------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
	if( m_enemySpawnTimer.CheckAndReset() )
	{
		m_currentMap->SpawnEnemy( m_maxNumEnemies );
	}
	m_currentMap->Update( deltaSeconds );


	if( m_currentScore >= m_scoreToProceed )
	{
		UnlockCurrentMapExit();
	}

	CleanUpDeadEntities();
	if( m_currentMap->IsPlayerInExit() )
	{
		LoadNextMap();
	}
}



//---------------------------------------------------------------------------------------------------------
void World::UpdateNewLevelProgress()
{
	++m_difficultyLevel;
	m_scoreToProceed = 100 * ( m_difficultyLevel * m_difficultyLevel );
	m_currentScore = 0;
	m_maxNumEnemies = 4 + m_difficultyLevel;

	m_enemySpawnTimer.SetSeconds( m_theGame->GetGameClock(), 2.0 );
}


//---------------------------------------------------------------------------------------------------------
void World::AddValueToCurrentScore( int value )
{
	m_currentScore += value * m_difficultyLevel;
}


//---------------------------------------------------------------------------------------------------------
void World::UnlockCurrentMapExit()
{
	m_currentMap->SetExitIsEnabled( true );
}


//---------------------------------------------------------------------------------------------------------
void World::LoadNextMap()
{
	int mapIndex = m_currentMapIndex % m_maps.size();
	m_currentMap = m_maps[mapIndex];
	m_currentMap->SetExitIsEnabled( false );
	UpdateNewLevelProgress();

	m_currentMap->CleanUpEntities();
	m_currentMap->ClearEntities();
	AddPlayerToCurrentMap( m_theGame->GetPlayer() );
}


//---------------------------------------------------------------------------------------------------------
float World::GetItemDropChance( float entityBaseDropChance )
{
	float worldDropChance = m_difficultyLevel * 0.01f;
	return entityBaseDropChance + worldDropChance;
}


//---------------------------------------------------------------------------------------------------------
void World::SetDifficultyLevel( int difficultyLevel )
{
	m_difficultyLevel = difficultyLevel;
}


//---------------------------------------------------------------------------------------------------------
void World::AddPlayerToCurrentMap( Player* player )
{
	m_currentMap->AddPlayerToMap( player );
}


//---------------------------------------------------------------------------------------------------------
Enemy* World::GetDiscOverlapEnemyOnCurrentMap( Vec2 const& discCenterPosition, float discRadius )
{
	return m_currentMap->GetDiscOverlapEnemy( discCenterPosition, discRadius );
}

