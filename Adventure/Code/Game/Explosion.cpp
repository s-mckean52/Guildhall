#include "Game/Explosion.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


Texture* Explosion::s_explosionTexture = nullptr;
SpriteSheet* Explosion::s_explosionSpriteSheet = nullptr;

//---------------------------------------------------------------------------------------------------------
Explosion::~Explosion()
{
}


//---------------------------------------------------------------------------------------------------------
Explosion::Explosion( Map* theMap, const Vec2& startPosition, float duration, float radius )
	: Entity( theMap, startPosition, FACTION_GOOD )
	, m_duration( duration )
{
	m_orientationDegrees = g_RNG->RollRandomFloatInRange( 0.f, 360.f );

	m_explosionAnim = new SpriteAnimDefinition( *s_explosionSpriteSheet, 0, 24, m_duration, SpriteAnimPlayBackType::ONCE );

	m_spriteBox = AABB2( Vec2( -radius, -radius ), Vec2( radius, radius ) );
}


//---------------------------------------------------------------------------------------------------------
void Explosion::Update( float seconds )
{
	if( m_timeElapsed > m_duration )
	{
		Die();
	}
	m_timeElapsed += seconds;
}


//---------------------------------------------------------------------------------------------------------
void Explosion::Render() const
{
	Vec2 uvsAtMax;
	Vec2 uvsAtMin;
	SpriteDefinition currentSprite = m_explosionAnim->GetSpriteDefAtTime( m_timeElapsed );

	currentSprite.GetUVs( uvsAtMin, uvsAtMax );

	std::vector<Vertex_PCU> explosionVerts;

	AppendVertsForAABB2D( explosionVerts, m_spriteBox, m_color, uvsAtMin, uvsAtMax );

	TransformVertexArray( explosionVerts, 1.f, m_orientationDegrees, m_position );

	g_theRenderer->BindTexture( &currentSprite.GetTexture() );
	g_theRenderer->DrawVertexArray( explosionVerts );
}


//---------------------------------------------------------------------------------------------------------
void Explosion::CreateExplosionSpriteSheet()
{
	s_explosionTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	s_explosionSpriteSheet = new SpriteSheet( *s_explosionTexture, IntVec2( 5, 5 ) );
}
