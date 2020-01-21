#include "Game/NpcTurret.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/Map.hpp"
#include "Game/RaycastResult.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//---------------------------------------------------------------------------------------------------------
NpcTurret::NpcTurret( Map* theMap, const Vec2& position, EntityFaction faction )
	: Entity( theMap, position, faction )
{
	m_entityType			= ENTITY_TYPE_NPC_TURRET;
	m_pushedByOtherEntities = false;

	m_maxHealth				= NPC_TURRET_HEALTH;
	m_health				= NPC_TURRET_HEALTH;
	m_angularVelocity		= NPC_TURRET_ANGULAR_VELOCITY;
	m_viewRange				= NPC_TURRET_VIEW_RANGE;
	m_physicsRadius			= NPC_TURRET_PHYSICS_RADIUS;
	m_cosmeticRadius		= NPC_TURRET_COSMETIC_RADIUS;
	m_shootCooldown			= NPC_TURRET_SHOOT_INTERVAL;
	m_shootAperatureDegrees	= NPC_TURRET_SHOOT_APERATURE_DEGREES;

	m_spriteBox	= AABB2( Vec2( -0.45f, -0.45f ), Vec2( 0.45f, 0.45f ) );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::TakeDamage( int damageDealt )
{
	SoundID enemyDamagedSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->PlaySound( enemyDamagedSound );

	Entity::TakeDamage( damageDealt );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::Die()
{
	SoundID enemyDeathSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->PlaySound( enemyDeathSound );

	m_map->SpawnExplosion( m_position, m_cosmeticRadius, NPC_TURRET_EXPLOSION_DURATION_SECONDS );

	Entity::Die();
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::Update( float deltaSeconds )
{
	HealthRegen( deltaSeconds );

	Entity* player = m_map->GetEntity( ENTITY_TYPE_PLAYER, 0 );

	TurnTowardsEntityIfWithinRange( player, deltaSeconds );

	m_fwdDir = Vec2::MakeFromPolarDegrees( m_orientationDegrees );
	RaycastResult raycastToDraw = m_map->Raycast( m_position, m_fwdDir, m_viewRange );
	m_bulletTrajectoryEndPoint = raycastToDraw.m_impactPos;

	ShootAtEntity( player, deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::Render() const
{
	Vec2 turretTip = m_position + ( m_fwdDir * m_cosmeticRadius );
	DrawLineBetweenPoints( turretTip, m_bulletTrajectoryEndPoint, Rgba8::RED, NPC_TURRET_LASER_THICKNESS );
	
	RenderBase();
	RenderTurret();
	RenderHealthBar( Rgba8::RED );

	if( !g_isDebugDraw ) return;

	DrawCircleAtPoint( m_position, m_viewRange, Rgba8::RED, DEBUG_THICKNESS );
	DrawCircleAtPoint( m_bulletTrajectoryEndPoint, DEBUG_THICKNESS, Rgba8::GREEN, DEBUG_THICKNESS );
	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::RenderBase() const
{
	std::vector<Vertex_PCU> turretBaseVerts;

	AppendVertsForAABB2D( turretBaseVerts, m_spriteBox, m_color, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>( turretBaseVerts.size() ), &turretBaseVerts[ 0 ], 1.f, 0.f, m_position );

	Texture* turretBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	g_theRenderer->BindTexture( turretBaseTexture );
	g_theRenderer->DrawVertexArray( turretBaseVerts );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::RenderTurret() const
{
	std::vector<Vertex_PCU> turretVerts;

	AppendVertsForAABB2D( turretVerts, m_spriteBox, Rgba8::WHITE, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>( turretVerts.size() ), &turretVerts[ 0 ], 1.f, m_orientationDegrees, m_position );

	Texture* turretTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretTop.png" );
	g_theRenderer->BindTexture( turretTexture );
	g_theRenderer->DrawVertexArray( turretVerts );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::Shoot()
{
	SoundID enemyShootSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->PlaySound( enemyShootSound );

	Vec2 bulletFirePosition = m_position + ( m_fwdDir * m_cosmeticRadius );
	Entity *firedBullet = m_map->SpawnEntity( ENTITY_TYPE_BAD_BULLET, bulletFirePosition );
	firedBullet->SetOrientationDegrees( m_orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::TurnTowardsEntityIfWithinRange( Entity* entityToTurnTowards, float deltaSeconds )
{
	Vec2 displacementToEntity = entityToTurnTowards->GetPosition() - m_position;

	if( IsEntityVisible( entityToTurnTowards ) )
	{
		m_orientationDegrees = GetTurnedToward( m_orientationDegrees, displacementToEntity.GetAngleDegrees(), m_angularVelocity * deltaSeconds );
	}
	else
	{
		if( m_enemyLastKnownLocation != Vec2( -1.f, -1.f ) )
		{
			Vec2 displacementToLastKnownLocation = m_enemyLastKnownLocation - m_position;
			float angleToTurnTowards = displacementToLastKnownLocation.GetAngleDegrees() + ( m_swivelDirection * ( NPC_TURRET_SWIVEL_APERATURE_DEGREES * 0.5f ) );
			m_orientationDegrees = GetTurnedToward( m_orientationDegrees, angleToTurnTowards, m_angularVelocity * deltaSeconds );

			if( m_orientationDegrees == angleToTurnTowards )
			{
				m_swivelDirection *= -1;
			}
		}
		else
		{
			m_orientationDegrees -= m_angularVelocity * deltaSeconds;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool NpcTurret::IsEntityVisible( Entity* entity )
{
	Vec2 displacementToEntity = entity->GetPosition() - m_position;
	if( entity->IsDead() || displacementToEntity.GetLength() > m_viewRange || !m_map->HasLineOfSight( m_position, entity->GetPosition() ) )
	{
		return false;
	}
	m_enemyLastKnownLocation = entity->GetPosition();
	return true;
}


//---------------------------------------------------------------------------------------------------------
void NpcTurret::ShootAtEntity( Entity* entityToShootAt, float deltaSeconds )
{
	m_shootCooldown -= deltaSeconds;
	if( IsEntityVisible( entityToShootAt ) && IsEntityInShootingAperature( entityToShootAt ) )
	{
		if( m_shootCooldown <= 0.f )
		{
			Shoot();
			m_shootCooldown = NPC_TURRET_SHOOT_INTERVAL;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool NpcTurret::IsEntityInShootingAperature( Entity* entityToShootAt )
{
	return IsPointInForwardSector2D( entityToShootAt->GetPosition(), m_position, m_viewRange, m_fwdDir, m_shootAperatureDegrees );
}

