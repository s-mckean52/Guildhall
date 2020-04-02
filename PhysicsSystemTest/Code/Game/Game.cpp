#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/GameObject.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include <string>


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
	EnableDebugRendering();
	m_gameClock = new Clock();
	g_theRenderer->SetGameClock( m_gameClock );
	m_physics2D = new Physics2D( m_gameClock );

	m_worldCamera = new Camera( g_theRenderer );
	m_uiCamera = new Camera( g_theRenderer );

	g_theConsole->PrintString( Rgba8::RED, "Game Start Up" );
	g_RNG = new RandomNumberGenerator();

	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	LoadAssets();

	m_worldCamera->SetPosition( m_focalPoint );
	m_worldCamera->SetProjectionOrthographic( m_cameraHeight, -20.f, 20.f );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	
	m_uiCamera->SetPosition( m_focalPoint );
	m_uiCamera->SetProjectionOrthographic( m_cameraHeight );

	g_theEventSystem->SubscribeEventCallbackFunction( "set_physics_update", SetPhysicsUpdate );
	DebugAddWorldPoint(Vec3(0.f, 0.f, 0.f), 10.f, Rgba8::RED, 10.f, DEBUG_RENDER_ALWAYS);
	//DebugAddWorldLine(Vec3(0.f,0.f, 0.f), Vec3(1.f, 1.f, 0.f), Rgba8::RED, 10.f, DEBUG_RENDER_ALWAYS);
}


//---------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete g_RNG;
	g_RNG = nullptr;

	delete m_physics2D;
	m_physics2D = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void Game::EndFrame()
{
	DestroyGameObjects();
	m_physics2D->EndFrame();
}


//---------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Render worldCamera
	g_theRenderer->BeginCamera( *m_worldCamera );
	DrawNewPolygonPoints();
	DrawGameObjects();
	g_theRenderer->EndCamera( *m_worldCamera );


	//UI Camera
	g_theRenderer->BeginCamera( *m_uiCamera );
	RenderUI();
	g_theRenderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//---------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	// Draw Gravity Text
	std::vector<Vertex_PCU> gravityVerts;
	std::string gravityAsString = Stringf( "Gravity: %f", m_physics2D->GetGravityAmount() );

	Vec3 positionToDraw = m_uiCamera->ClientToWorldPosition( Vec2( 0.01f, 0.98f ) );

	g_testFont->AddVertsForText2D( gravityVerts, Vec2( positionToDraw.x, positionToDraw.y ) , 0.1f, gravityAsString );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( gravityVerts );


	// Draw Physics State
	std::vector<Vertex_PCU> physicsClockStateVerts;
	std::string physicsStateAsString = Stringf( "Physics - IsPause: %s  TimeScale: %f", m_physics2D->m_clock->IsPaused() ? "true" : "false", m_physics2D->m_clock->GetScale() );
	
	Vec3 positionToDrawPhysicsState = m_uiCamera->ClientToWorldPosition( Vec2( 0.01f, 0.95f ) );

	g_testFont->AddVertsForText2D( physicsClockStateVerts, Vec2( positionToDrawPhysicsState.x, positionToDrawPhysicsState.y ), 0.1f, physicsStateAsString );
	g_theRenderer->BindTexture( g_testFont->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( physicsClockStateVerts );
}


//---------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );
	if( !g_theConsole->IsOpen() )
	{
		UpdateGameStatesFromInput( deltaSeconds );
	}
	
	m_physics2D->Update();

// 	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
// 	{
// 		GameObject* currentGO = m_gameObjects[ goIndex ];
// 		currentGO->m_isOverlapping = false;
// 		for( int indexToCheck = 0; indexToCheck < m_gameObjects.size(); ++indexToCheck )
// 		{
// 			if( currentGO == m_gameObjects[ indexToCheck ] ) continue;
// 			if( currentGO->GetCollider()->Intersects( m_gameObjects[ indexToCheck ]->GetCollider() ) )
// 			{
// 				currentGO->m_isOverlapping = true;
// 				m_gameObjects[ indexToCheck ]->m_isOverlapping = true;
// 			}
// 		}
// 	}

	UpdateGameObjects( deltaSeconds );
	UpdateCameras( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameStatesFromInput( float deltaSeconds )
{
	UpdateMousePos( *m_worldCamera );
	MoveWorldCameraPosition( deltaSeconds );
	ResetCameraPosition( deltaSeconds );

	if( m_isCreatingPolygon )
	{
		CreatePolygonFromInput();
		return;
	}

	if( m_draggedObject == nullptr )
	{
		CreateNewGameObject( deltaSeconds );
		ModifyPhysicsGravity( deltaSeconds );
	}
	else
	{
		ChangeDraggedObjectSimulationMode( deltaSeconds );
		ModifyDraggedObjectBounciness( deltaSeconds );
		ModifyMassOfDraggedObject( deltaSeconds );
		ModifyDragOfDraggedObject( deltaSeconds );
		ModifyFrictionOfDraggedObject( deltaSeconds );
		ModifyDraggedObjectRotation( deltaSeconds );
		ModifyDraggedObjectRotationalVelocity( deltaSeconds );
		DeleteDraggedObject( deltaSeconds );
	}

	UpdatePhysicsInput();

	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
	{
		SetDraggedObject();
	}

	if( g_theInput->WasMouseButtonJustReleased( MOUSE_BUTTON_LEFT ) )
	{
		DropDraggedObject( deltaSeconds );
	}

	float scrollAmount = g_theInput->GetScrollAmount();
	if( scrollAmount != 0.f )
	{
		m_cameraHeight += 1.f * -scrollAmount;
		Clamp( m_cameraHeight, m_cameraMinHeight, m_cameraMaxHeight );
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_isQuitting = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	//Update m_worldCamera
	m_worldCamera->SetPosition( m_focalPoint );
	m_worldCamera->SetProjectionOrthographic( m_cameraHeight );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	LoadTextures();
	LoadAudio();
	g_theConsole->PrintString( Rgba8::GREEN, "Assets Loaded" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadTextures()
{
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGl.png" );
	g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
}


//---------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateMousePos( const Camera& camera )
{
 	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	Vec3 newMousePos = camera.ClientToWorldPosition( mouseNormalizedPos );
	m_mousePos = Vec2( newMousePos.x, newMousePos.y );

	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGO = m_gameObjects[ goIndex ];
		if( currentGO )
		{
			Rigidbody2D* currentGORigidbody = currentGO->m_rigidbody;
			currentGO->m_isHovered = currentGORigidbody->m_collider->Contains( m_mousePos );
		}
	}

	if( m_draggedObject )
	{
		m_draggedObject->SetPosition( m_mousePos + m_draggedObjectOffset );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveWorldCameraPosition( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( g_theInput->IsKeyPressed( 'W' ) )
	{
		m_focalPoint.y += m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'A' ) )
	{
		m_focalPoint.x -= m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'S' ) )
	{
		m_focalPoint.y -= m_cameraMoveAmountPerFrame;
	}
	if( g_theInput->IsKeyPressed( 'D' ) )
	{
		m_focalPoint.x += m_cameraMoveAmountPerFrame;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ResetCameraPosition( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( g_theInput->IsKeyPressed( 'O' ) )
	{
		m_focalPoint = Vec3();
		m_cameraHeight = CAMERA_SIZE_Y;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CreateNewGameObject( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( g_theInput->WasKeyJustPressed( '1' ) && !m_draggedObject )
	{
		GameObject* newGameObject = CreateDisc();
		AddGameObject( newGameObject );
	}

	if( g_theInput->WasKeyJustPressed( '2' ) && !m_draggedObject )
	{
		AddPointToNewPolygon();
		m_isCreatingPolygon = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ChangeDraggedObjectSimulationMode( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( m_draggedObject == nullptr ) return;

	if( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	}

	if( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_KINEMATIC );
	}

	if( g_theInput->WasKeyJustPressed( '3' ) )
	{
		m_draggedObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DropDraggedObject( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	Rigidbody2D* draggedRb = m_draggedObject->m_rigidbody;
	if( deltaSeconds > 0.f )
	{
		Vec2 velocity = (draggedRb->m_worldPosition - draggedRb->m_positionLastFrame) / deltaSeconds;
		m_draggedObject->m_rigidbody->SetVelocity( velocity );
	}

	m_draggedObject->m_isHeld = false;
	m_draggedObject = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyPhysicsGravity( float deltaSeconds )
{
	if( g_theInput->IsKeyPressed( KEY_CODE_MINUS ) )
	{
		m_physics2D->AddGravityInDownDirection( 1.0f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_PLUS ) )
	{
		m_physics2D->AddGravityInDownDirection( -1.0f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyDraggedObjectBounciness( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	if( g_theInput->IsKeyPressed( KEY_CODE_MINUS ) )
	{
		m_draggedObject->AddBounciness( -0.1f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_PLUS ) )
	{
		m_draggedObject->AddBounciness( 0.1f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DeleteDraggedObject( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if( m_draggedObject == nullptr ) return;

	if( g_theInput->WasKeyJustPressed( KEY_CODE_DELETE ) || g_theInput->WasKeyJustPressed( KEY_CODE_BACKSPACE ) )
	{
		m_draggedObject->m_isDestroyed = true;
		m_draggedObject = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyMassOfDraggedObject( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	if( g_theInput->IsKeyPressed( KEY_CODE_LEFT_BRACKET ) )
	{
		m_draggedObject->AddMass( -10.f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_RIGHT_BRACKET ) )
	{
		m_draggedObject->AddMass( 10.f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyDragOfDraggedObject( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	if( g_theInput->IsKeyPressed( KEY_CODE_SEMICOLON ) )
	{
		m_draggedObject->AddDrag( 5.0f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_BACK_SLASH ) )
	{
		m_draggedObject->AddDrag( -5.0f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyFrictionOfDraggedObject( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	if( g_theInput->IsKeyPressed( KEY_CODE_COMMA ) )
	{
		m_draggedObject->AddFriction( -0.5f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( KEY_CODE_PERIOD ) )
	{
		m_draggedObject->AddFriction( 0.5f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdatePhysicsInput()
{
	if( g_theInput->WasKeyJustPressed( 'P' ) )
	{
		m_physics2D->ToggleClockPause();
	}

	if( g_theInput->WasKeyJustPressed( '8' ) )
	{
		m_physics2D->SetClockScale( 0.5 );
	}

	if( g_theInput->WasKeyJustPressed( '9' ) )
	{
		m_physics2D->SetClockScale( 2.0 );
	}

	if( g_theInput->WasKeyJustPressed( '0' ) )
	{
		m_physics2D->SetClockScale( 1.0 );
		m_physics2D->m_clock->Resume();
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyDraggedObjectRotation( float deltaSeconds )
{
	if( m_draggedObject == nullptr ) return;

	if( g_theInput->IsKeyPressed( 'R' ) )
	{
		m_draggedObject->AddRotationDegrees( 45.f * deltaSeconds );
	}

	if ( g_theInput->IsKeyPressed( 'F' ) )
	{
		m_draggedObject->AddRotationDegrees( -45.f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::ModifyDraggedObjectRotationalVelocity( float deltaSeconds )
{
	if ( m_draggedObject == nullptr ) return;

	if( g_theInput->WasKeyJustPressed( 'V' ) )
	{
		m_draggedObject->m_rigidbody->SetAngularVelocity( 0.0f );
	}

	if( g_theInput->IsKeyPressed( 'T' ) )
	{
		m_draggedObject->m_rigidbody->AddAngularVelocity( 5.f * deltaSeconds );
	}

	if( g_theInput->IsKeyPressed( 'G' ) )
	{
		m_draggedObject->m_rigidbody->AddAngularVelocity( -5.f * deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
GameObject* Game::CreateDisc()
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody2D();
	gameObject->m_rigidbody->SetPosition( m_mousePos );
	gameObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );

	float radius = g_RNG->RollRandomFloatInRange( 0.5f, 2.f );
	DiscCollider2D* collider = m_physics2D->CreateDiscCollider2D( Vec2( 0.f, 0.f ), radius );
	gameObject->m_rigidbody->TakeCollider( collider );
	gameObject->m_rigidbody->SetMass( 1.f );

	return gameObject;
}


//---------------------------------------------------------------------------------------------------------
GameObject* Game::CreatePolygon( std::vector<Vec2> polygonPoints )
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody2D();
	gameObject->m_rigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	
	Vec2 position = AveragePositions( polygonPoints );
	gameObject->SetPosition( position );

	for( int pointIndex = 0; pointIndex < polygonPoints.size(); ++pointIndex )
	{
		polygonPoints[ pointIndex ] -= position;
	}

	PolygonCollider2D* collider = m_physics2D->CreatePolygonCollider2D( polygonPoints, Vec2( 0.f, 0.f ) );
	gameObject->m_rigidbody->TakeCollider( collider );
	gameObject->m_rigidbody->SetMass( 1.f );

	return gameObject;
}


//---------------------------------------------------------------------------------------------------------
void Game::AddPointToNewPolygon()
{
	m_newPolygonToDraw.push_back( m_mousePos );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Game::AveragePositions( std::vector<Vec2> points )
{
	Vec2 averagePosition;
	for( int pointIndex = 0; pointIndex < points.size(); ++pointIndex )
	{
		averagePosition += points[ pointIndex ];
	}
	averagePosition /= static_cast<float>( points.size() );
	return averagePosition;
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawGameObjects() const
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[ goIndex ] )
		{
			m_gameObjects[ goIndex ]->Draw( m_mousePos );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::UpdateGameObjects( float deltaSeconds )
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject )
		{
			currentGameObject->Update( deltaSeconds );
			HandleGameObjectsOutOfBounds( currentGameObject );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::HandleGameObjectsOutOfBounds( GameObject* gameObject )
{
	MoveGameObjectToOppositeSideOfScreen( gameObject );
	BounceGameObjectOffBottomOfScreen( gameObject );
}


//---------------------------------------------------------------------------------------------------------
void Game::MoveGameObjectToOppositeSideOfScreen( GameObject* gameObject )
{
	Collider2D* gameObjectCollider = gameObject->GetCollider();
	if( gameObjectCollider == nullptr ) return;

	Vec3 cameraBoundsMin = m_worldCamera->GetOrthoBottomLeft();
	Vec3 cameraBoundsMax = m_worldCamera->GetOrthoTopRight();

	AABB2 cameraBounds( cameraBoundsMin.x, cameraBoundsMin.y, cameraBoundsMax.x, cameraBoundsMax.y );
	AABB2 gameObjectWolrdBounds = gameObjectCollider->GetWorldBounds();
	Vec2 gameObjectDimensionsOfBounds = gameObjectWolrdBounds.GetDimensions();

	if( !DoAABB2sOverlap( gameObjectWolrdBounds, cameraBounds ) )
	{
		float cameraHalfHeight = m_cameraHeight * 0.5f;
		float cameraHalfWidth = cameraHalfHeight * m_worldCamera->GetAspectRatio();

		Vec2 gameObjectWorldPosition = gameObject->m_rigidbody->m_worldPosition;
		Vec2 diplacmentToCenter = gameObjectWorldPosition - cameraBounds.GetCenter();
		float lengthToRight = GetProjectedLength2D( diplacmentToCenter, Vec2::RIGHT );

		float orientationPosition = cameraHalfWidth + ( gameObjectDimensionsOfBounds.x * 0.5f ) - 0.001f;
		if( lengthToRight > cameraHalfWidth )
		{
			gameObject->SetPosition( Vec2( -orientationPosition, gameObjectWorldPosition.y ) );
		}
		else if( lengthToRight < -cameraHalfWidth )
		{
			gameObject->SetPosition( Vec2( orientationPosition, gameObjectWorldPosition.y ) );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::BounceGameObjectOffBottomOfScreen( GameObject* gameObject )
{
	Vec3 cameraMins = m_worldCamera->GetOrthoBottomLeft();
	Vec3 cameraMaxes = m_worldCamera->GetOrthoTopRight();

	Vec2 cameraBottomLeft = Vec2( cameraMins.x, cameraMins.y );
	Vec2 cameraBottomRight = Vec2( cameraMaxes.x, cameraMins.y );

	AABB2 gameObjectWorldBounds = gameObject->GetCollider()->GetWorldBounds();
	OBB2 boundsAsOBB = OBB2( gameObjectWorldBounds );
	if( DoOBBAndLineSegmentOverlap2D( boundsAsOBB, cameraBottomLeft, cameraBottomRight ) )
	{
		Rigidbody2D* gameObjectRigidbody = gameObject->m_rigidbody;
		Vec2 reflectedVelocity = gameObjectRigidbody->GetVelocity();
		reflectedVelocity.y = -reflectedVelocity.y;
		gameObjectRigidbody->SetVelocity( reflectedVelocity );
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::CheckGameObjectOverlap( GameObject* gameObject )
{
	bool isOverlapping = false;
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject && gameObject && currentGameObject != gameObject )
		{
			if( gameObject->m_rigidbody->m_collider->Intersects( currentGameObject->m_rigidbody->m_collider ) )
			{
				isOverlapping = true;
			}
		}
	}
	gameObject->m_isOverlapping = isOverlapping;
}


//---------------------------------------------------------------------------------------------------------
void Game::SetDraggedObject()
{
	int gameObjectSize = static_cast<int>( m_gameObjects.size() ) - 1;
	for( int goIndex = gameObjectSize; goIndex >= 0; --goIndex )
	{
		GameObject* currentGameObject = m_gameObjects[ goIndex ];
		if( currentGameObject && currentGameObject->m_isHovered )
		{
			m_draggedObject = currentGameObject;
			m_draggedObjectOffset = currentGameObject->m_rigidbody->m_worldPosition - m_mousePos;

			currentGameObject->m_isHeld = true;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DestroyGameObjects()
{
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[ goIndex] && m_gameObjects[goIndex]->m_isDestroyed )
		{
			delete m_gameObjects[ goIndex ];
			m_gameObjects[ goIndex ] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Game::IsNextPointValidOnPolygon( const Vec2& point )
{
	Vec2 start;
	Vec2 end;

	for( int polygonIndex = 0; polygonIndex < m_newPolygonToDraw.size(); ++polygonIndex )
	{
		if( polygonIndex != m_newPolygonToDraw.size() - 1 )
		{
			start = m_newPolygonToDraw[ polygonIndex ];
			end = m_newPolygonToDraw[ polygonIndex + 1 ];

			Vec2 edge = end - start;
			Vec2 edgeNormal = edge.GetRotated90Degrees();
			edgeNormal.Normalize();
			Vec2 displacementStartToPoint = point - start;

			if( DotProduct2D( edgeNormal, displacementStartToPoint ) < 0.f )
			{
				return false;
			}
		}
		else if( m_newPolygonToDraw.size() > 2 )
		{
			start = m_newPolygonToDraw[ polygonIndex ];
			end = m_newPolygonToDraw[ 0 ];

			Vec2 edge = end - start;
			Vec2 edgeNormal = edge.GetRotated90Degrees();
			edgeNormal.Normalize();
			Vec2 displacementStartToPoint = point - start;

			if( DotProduct2D( edgeNormal, displacementStartToPoint ) > 0.f )
			{
				return false;
			}
		}
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
void Game::CreatePolygonFromInput()
{
	bool isValidPoint = IsNextPointValidOnPolygon( m_mousePos );

	if( isValidPoint )
	{
		m_polygonLineColor = Rgba8::BLUE;
	}
	else
	{
		m_polygonLineColor = Rgba8::RED;
	}

	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) && isValidPoint )
	{
		AddPointToNewPolygon();
	}

	if( g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_RIGHT ) )
	{
		GameObject* newGameObject = CreatePolygon( m_newPolygonToDraw );
		AddGameObject( newGameObject );
		m_newPolygonToDraw.clear();
		m_isCreatingPolygon = false;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		m_newPolygonToDraw.clear();
		m_isCreatingPolygon = false;
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::DrawNewPolygonPoints() const
{
	for( int pointIndex = 0; pointIndex < m_newPolygonToDraw.size(); ++pointIndex )
	{
		Vec2 start;
		Vec2 end;

		if( pointIndex != m_newPolygonToDraw.size() - 1 )
		{
			start = m_newPolygonToDraw[ pointIndex ];
			end = m_newPolygonToDraw[ pointIndex + 1 ];
			DrawLineBetweenPoints( start, end, Rgba8::BLUE, 0.05f );
		}
		else
		{
			start = m_newPolygonToDraw[ m_newPolygonToDraw.size() - 1 ];
			end = m_mousePos;
			DrawLineBetweenPoints( start, end, m_polygonLineColor, 0.05f );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Game::AddGameObject( GameObject* gameObject )
{
	bool wasPlaced = false;
	for( int goIndex = 0; goIndex < m_gameObjects.size(); ++goIndex )
	{
		if( m_gameObjects[goIndex] == nullptr )
		{
			m_gameObjects[goIndex] = gameObject;
			wasPlaced = true;
			break;
		}
	}
	if( !wasPlaced )
	{
		m_gameObjects.push_back( gameObject );
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC void Game::SetPhysicsUpdate( NamedStrings* args )
{
	double defaultHz = 120;
	double hz = args->GetValue( "hz", defaultHz );
	Physics2D* physicsSystem = g_theGame->GetPhysicsSystem();
	physicsSystem->SetFixedDeltaTime( 1 / hz );

	g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Physics Update set to %fhz", hz ) );
}