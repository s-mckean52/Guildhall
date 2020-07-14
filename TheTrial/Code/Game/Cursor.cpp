#include "Game/Cursor.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"


//---------------------------------------------------------------------------------------------------------
Cursor::Cursor()
{
	Texture* cursorTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Cursors_8x8.png" );
	m_sheet = new SpriteSheet( *cursorTexture, IntVec2( 8, 8 ) );
}


//---------------------------------------------------------------------------------------------------------
Cursor::~Cursor()
{
	delete m_sheet;
	m_sheet = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Cursor::Update()
{
	if( m_state != CURSOR_STATE_ATTACK_MOVE )
	{
		SetState( CURSOR_STATE_DEFAULT );
	}

	Vec2 mouseNormalizedPos = g_theInput->GetMouseNormalizedClientPosition();
	
	Camera* playerCamera = g_theGame->GetPlayerCamera();
	Vec3 newWorldPos = playerCamera->ClientToWorldPosition( mouseNormalizedPos );

	Camera* uiCamera = g_theGame->GetUICamera();
	Vec3 newUIPos = uiCamera->ClientToWorldPosition( mouseNormalizedPos );

	m_worldPosition = Vec2( newWorldPos.x, newWorldPos.y );
	m_uiPosition = Vec2( newUIPos.x, newUIPos.y );
}


//---------------------------------------------------------------------------------------------------------
void Cursor::Render() const
{
	Vec2 minUV;
	Vec2 maxUV;
	std::vector<Vertex_PCU> verts;

	m_sheet->GetSpriteUVs( minUV, maxUV, m_state );
	AppendVertsForAABB2D( verts, m_dimensions, m_tint, minUV, maxUV );
	
	TransformVertexArray( verts, 1.f, 0.f, m_uiPosition );
	g_theRenderer->BindTexture( &m_sheet->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	if( g_isDebugDraw )
	{
		DrawCircleAtPoint( m_uiPosition, m_radius, Rgba8::CYAN, DEBUG_THICKNESS );
	}
}


//---------------------------------------------------------------------------------------------------------
void Cursor::SetState( CursorState cursorState )
{
	m_state = cursorState;
}


//---------------------------------------------------------------------------------------------------------
void Cursor::SetPosition( Vec2 const& position )
{
	m_worldPosition = position;
	m_uiPosition = position;
}


//---------------------------------------------------------------------------------------------------------
bool Cursor::IsOverlappingEntity( Entity* entity )
{
	Vec2 entityPosition = entity->GetCurrentPosition();
	float entityRadius = entity->GetPhysicsRadius();

	return DoDiscsOverlap( m_worldPosition, m_radius, entityPosition, entityRadius );
}
