#include "Game/Enemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/RaycastResult.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Item.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/IntVec2.hpp"


//---------------------------------------------------------------------------------------------------------
Enemy::Enemy( Game* theGame, Vec2 const& position )
	: Actor( theGame )
{
	m_currentPosition = position;
	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Enemies/Skeleton/Idle/down.png" );
	m_spriteSheet = new SpriteSheet( *spriteTexture, IntVec2( 6, 1 ) );

	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Walk/down.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Walk/left.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Walk/right.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Walk/up.png" );

	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Idle/down.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Idle/left.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Idle/right.png" );
	CreateSpriteAnimFromPath( "Data/Images/Enemies/Skeleton/Idle/up.png" );
}


//---------------------------------------------------------------------------------------------------------
Enemy::~Enemy()
{
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Update( float deltaSeconds )
{
	UpdateStatusEffects();

	Vec2 playerPosition = m_theGame->GetPlayer()->GetCurrentPosition();
	Vec2 displacementToPlayer = playerPosition - m_currentPosition;
	lineOfSight = m_theWorld->GetCurrentMap()->Raycast( m_currentPosition, displacementToPlayer.GetNormalized(), displacementToPlayer.GetLength(), this );

	if( lineOfSight.didImpact && lineOfSight.impactEntity != nullptr )
	{
		Player* hitPlayer = dynamic_cast<Player*>( lineOfSight.impactEntity );
		if( hitPlayer != nullptr )
		{
			m_actorState = ACTOR_STATE_WALK;
			SetMovePosition( playerPosition );
			MoveTowardsPosition( deltaSeconds );
		}
		else
		{
			m_actorState = ACTOR_STATE_IDLE;
		}
	}

	UpdateAnimSpriteBasedOnMovementDirection( "Enemies/Skeleton" );
}


//---------------------------------------------------------------------------------------------------------
void Enemy::Render() const
{
	if( m_isDead )
		return;

	Vec2 uvMin;
	Vec2 uvMax;
	std::vector<Vertex_PCU> verts;
	AABB2 worldSpriteBounds = m_renderBounds;
	worldSpriteBounds.SetCenter( m_currentPosition );

	Clock* gameClock = g_theGame->GetGameClock();
	float elapsedTime = static_cast<float>( gameClock->GetTotalElapsedSeconds() );
	SpriteDefinition const& spriteDef = m_anim->GetSpriteDefAtTime( elapsedTime * GetMoveSpeed() );
	spriteDef.GetUVs( uvMin, uvMax );

	AppendVertsForAABB2D( verts, worldSpriteBounds, Rgba8::WHITE, uvMin, uvMax );
	
	g_theRenderer->BindTexture( &spriteDef.GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( verts );
	
	RenderHealthBar( Rgba8::RED );

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Enemy::DebugRender() const
{
	DrawLineBetweenPoints( lineOfSight.impactPosition, lineOfSight.startPosition + ( lineOfSight.forwardNormal * lineOfSight.maxDistance ), Rgba8::GREEN, DEBUG_THICKNESS );
	DrawLineBetweenPoints( lineOfSight.startPosition, lineOfSight.impactPosition, Rgba8::RED, DEBUG_THICKNESS );
	DrawCircleAtPoint( lineOfSight.impactPosition, DEBUG_THICKNESS, Rgba8::YELLOW, DEBUG_THICKNESS );
}


//---------------------------------------------------------------------------------------------------------
void Enemy::TakeDamage( int damageToTake )
{
	Actor::TakeDamage( damageToTake );

	if( m_isDead )
	{
		DropRandomItem();
		m_theWorld->AddValueToCurrentScore( m_scoreValue );
	}
}


//---------------------------------------------------------------------------------------------------------
void Enemy::DropRandomItem()
{
	float dropChance = m_theWorld->GetItemDropChance( m_baseItemDropChance );
	if( g_RNG->RollPercentChance( dropChance ) )
	{
		Map* currentMap = m_theWorld->GetCurrentMap();
		Item* itemToDrop = Item::GetRandomItem();
		itemToDrop->SetCurrentPosition( m_currentPosition );
		currentMap->AddEntityToList( itemToDrop );
	}
}

//---------------------------------------------------------------------------------------------------------
void Enemy::AttackActor( Actor* actor )
{
	if( !m_attackTimer.HasElapsed() )
		return;

	actor->TakeDamage( GetDamageToDeal() );

	float attackCooldownSeconds = 1.f / GetAttackSpeed();
	m_attackTimer.SetSeconds( m_theGame->GetGameClock(), attackCooldownSeconds );
}
