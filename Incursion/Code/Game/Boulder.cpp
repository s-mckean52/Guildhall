#include "Game/Boulder.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Boulder::Boulder( Map* theMap, Vec2 position, EntityFaction faction )
	: Entity( theMap, position, faction )
{
	m_entityType = ENTITY_TYPE_BOULDER;
	m_isHitByBullet = false;
	m_physicsRadius = BOULDER_RADIUS;
	m_cosmeticRadius = BOULDER_RADIUS;

	m_spriteBox	= AABB2( Vec2( -0.45f, -0.45f ), Vec2( 0.45f, 0.45f ) );

	CreateBoulderSpriteSheet();
}


//---------------------------------------------------------------------------------------------------------
void Boulder::CreateBoulderSpriteSheet()
{
	m_texture_4x4 = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	m_spriteSheet = new SpriteSheet( *m_texture_4x4, IntVec2( 4, 4 ) );

	m_spriteSheet->GetSpriteUVs( m_uvAtMins, m_uvAtMaxes, 3 );

}


//---------------------------------------------------------------------------------------------------------
void Boulder::Render() const
{
	std::vector<Vertex_PCU> boulderVerts;
	AppendVertsForAABB2D( boulderVerts, m_spriteBox, m_color, m_uvAtMins, m_uvAtMaxes );

	TransformVertexArray( static_cast<int>( boulderVerts.size() ), &boulderVerts[ 0 ], m_scale, m_orientationDegrees, m_position );

	g_theRenderer->BindTexture( m_texture_4x4 );
	g_theRenderer->DrawVertexArray( boulderVerts );

	if( !g_isDebugDraw ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Boulder::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Boulder::DebugDraw() const
{
	Entity::DebugDraw();
}
