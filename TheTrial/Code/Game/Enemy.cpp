#include "Game/Enemy.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

//---------------------------------------------------------------------------------------------------------
Enemy::Enemy( Game* theGame, Vec2 const& position )
	: Actor( theGame )
{
	m_currentPosition = position;
	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Enemies/Skeleton/Idle/down.png" );
	m_spriteSheet = new SpriteSheet( *spriteTexture, IntVec2( 6, 1 ) );
}


//---------------------------------------------------------------------------------------------------------
Enemy::~Enemy()
{
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Render() const
{
	if( m_isDead )
		return;

	Vec2 uvMin;
	Vec2 uvMax;
	std::vector<Vertex_PCU> verts;

	m_spriteSheet->GetSpriteUVs( uvMin, uvMax, 0 );
	AABB2 worldBounds = m_renderBounds;
	worldBounds.Translate( m_currentPosition );
	AppendVertsForAABB2D( verts, worldBounds, Rgba8::WHITE, uvMin, uvMax );
	
	g_theRenderer->BindTexture( &m_spriteSheet->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( verts );
	


	RenderHealthBar( Rgba8::RED );
}
