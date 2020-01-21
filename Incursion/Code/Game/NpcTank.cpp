#include "Game/NpcTank.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/RaycastResult.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
NpcTank::NpcTank( Map* theMap, const Vec2& position, EntityFaction faction )
	: Entity( theMap, position, faction )
{
	m_entityType = ENTITY_TYPE_NPC_TANK;
	
	m_maxHealth				= NPC_TANK_HEALTH;
	m_health				= NPC_TANK_HEALTH;
	m_angularVelocity		= NPC_TANK_ANGULAR_VELOCITY;
	m_viewRange				= NPC_TANK_VIEW_RANGE;
	m_driveAperatureDegrees	= NPC_TANK_DRIVE_APERATURE_DEGREES;
	m_shootCooldown			= NPC_TANK_SHOOT_INTERVAL;
	m_shootAperatureDegrees	= NPC_TANK_SHOOT_APERATURE_DEGREES;
	m_physicsRadius			= NPC_TANK_PHYSICS_RADIUS;
	m_cosmeticRadius		= NPC_TANK_COSMETIC_RADIUS;
	m_velocity				= Vec2( NPC_TANK_SPEED, 0.f );

	m_spriteBox	= AABB2( Vec2( -0.45f, -0.45f ), Vec2( 0.45f, 0.45f ) );
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::TakeDamage( int damageDealt )
{
	SoundID enemyDamagedSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_theAudio->PlaySound( enemyDamagedSound );

	Entity::TakeDamage( damageDealt );
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::Die()
{
	SoundID enemyDeathSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_theAudio->PlaySound( enemyDeathSound );

	m_map->SpawnExplosion( m_position, m_cosmeticRadius, NPC_TANK_EXPLOSION_DURATION_SECONDS );

	Entity::Die();
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::Update( float deltaSeconds )
{
	HealthRegen( deltaSeconds );

	m_fwdDir = Vec2::MakeFromPolarDegrees( m_orientationDegrees );

	Entity* player = m_map->GetEntity( ENTITY_TYPE_PLAYER, 0 );

	TurnTowardsEntityIfWithinRange( player, deltaSeconds );
	SetThrustFraction( player );
	AvoidWalls();

	m_velocity.SetLength( NPC_TANK_SPEED * GetSlowFractionOnMud() );
	m_velocity.SetAngleDegrees( m_orientationDegrees );

	m_position += m_velocity * deltaSeconds * m_thrustFraction;

	ShootAtEntity( player, deltaSeconds );

	if( DoDiscsOverlap( m_position, m_physicsRadius, m_enemyLastKnownLocation, 0.f ) )
	{
		m_enemyLastKnownLocation = Vec2( -1.f, -1.f );
	}
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::AvoidWalls()
{
	Vec2 goalFwdVector = Vec2::MakeFromPolarDegrees( m_goalOrientationDegrees );
	m_goalCenterRaycastStart	= m_position + ( goalFwdVector * m_physicsRadius );
	m_goalLeftRaycastStart		= m_position + ( goalFwdVector.GetRotated90Degrees() * m_physicsRadius );
	m_goalRightRaycastStart		= m_position + ( goalFwdVector.GetRotatedMinus90Degrees() * m_physicsRadius );

	m_goalCenterRaycast		= m_map->Raycast( m_goalCenterRaycastStart, goalFwdVector, 1.f );
	m_goalLeftRaycast		= m_map->Raycast( m_goalLeftRaycastStart, goalFwdVector, 1.f );
	m_goalRightRaycast		= m_map->Raycast( m_goalRightRaycastStart, goalFwdVector, 1.f );

	if( m_goalCenterRaycast.m_didImpact )
	{
		m_goalOrientationDegrees += 10.f;
		m_newOrientationCooldown = NPC_TANK_NEW_ORIENTATION_INTERVAL;
	}

	if( m_goalRightRaycast.m_didImpact )
	{
		m_goalOrientationDegrees += 5.f;
		m_newOrientationCooldown = NPC_TANK_NEW_ORIENTATION_INTERVAL;
	}

	if( m_goalLeftRaycast.m_didImpact )
	{
		m_goalOrientationDegrees -= 5.f;
		m_newOrientationCooldown = NPC_TANK_NEW_ORIENTATION_INTERVAL;
	}


}


//---------------------------------------------------------------------------------------------------------
void NpcTank::Render() const
{
	std::vector<Vertex_PCU> npcTankVerts;

	AppendVertsForAABB2D( npcTankVerts, m_spriteBox, m_color, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>( npcTankVerts.size() ), &npcTankVerts[ 0 ], 1.f, m_orientationDegrees, m_position );
	
	Texture* npcTankTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
	g_theRenderer->BindTexture( npcTankTexture );
	g_theRenderer->DrawVertexArray( npcTankVerts );

	RenderHealthBar( Rgba8::RED );

	if( !g_isDebugDraw ) return;

	DrawLineBetweenPoints( m_goalCenterRaycastStart, m_goalCenterRaycast.m_impactPos, Rgba8::CYAN, DEBUG_THICKNESS );

	DrawLineBetweenPoints( m_goalLeftRaycastStart, m_goalLeftRaycast.m_impactPos, Rgba8::GREEN, DEBUG_THICKNESS );

	DrawLineBetweenPoints( m_goalRightRaycastStart, m_goalRightRaycast.m_impactPos, Rgba8::CYAN, DEBUG_THICKNESS );

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::Shoot()
{
	SoundID enemyShootSound = g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_theAudio->PlaySound( enemyShootSound );

	Vec2 bulletFirePosition = m_position + ( m_fwdDir * m_cosmeticRadius );
	Entity* firedBullet = m_map->SpawnEntity( ENTITY_TYPE_BAD_BULLET, bulletFirePosition );
	firedBullet->SetOrientationDegrees( m_orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::TurnTowardsEntityIfWithinRange( Entity* entityToTurnTowards, float deltaSeconds )
{
	m_newOrientationCooldown -= deltaSeconds;
	if( IsEntityVisible( entityToTurnTowards ) )
	{
		Vec2 displacementToEntity = entityToTurnTowards->GetPosition() - m_position;
		m_goalOrientationDegrees = displacementToEntity.GetAngleDegrees();
	}
	else if( m_enemyLastKnownLocation != Vec2( -1.f, -1.f ) )
	{
		Vec2 displacementToEnemyLastKnownLocation = m_enemyLastKnownLocation - m_position;
		m_goalOrientationDegrees = displacementToEnemyLastKnownLocation.GetAngleDegrees();
	}
	else if( m_newOrientationCooldown <= 0.f  )
	{
		m_goalOrientationDegrees = g_RNG->RollRandomFloatInRange( -180.f, 180.f );
		m_newOrientationCooldown = NPC_TANK_NEW_ORIENTATION_INTERVAL;
	}
	m_orientationDegrees = GetTurnedToward( m_orientationDegrees, m_goalOrientationDegrees, m_angularVelocity * deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
bool NpcTank::IsEntityVisible( Entity* entity )
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
void NpcTank::ShootAtEntity( Entity* entityToShootAt, float deltaSeconds )
{
	m_shootCooldown -= deltaSeconds;
	if( IsEntityVisible( entityToShootAt ) && IsEntityInShootingAperature( entityToShootAt ) )
	{
		if( m_shootCooldown <= 0.f )
		{
			Shoot();
			m_shootCooldown = NPC_TANK_SHOOT_INTERVAL;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void NpcTank::SetThrustFraction( Entity* entityToMoveTowards )
{
	if( IsEntityVisible( entityToMoveTowards ) && !IsEntityInDriveAperature( entityToMoveTowards ) )
	{
		m_thrustFraction = 0.f;
	}
	else 
	{
		m_thrustFraction = 1.f;
	}
}


//---------------------------------------------------------------------------------------------------------
bool NpcTank::IsEntityInShootingAperature( Entity* entityToShootAt )
{
	return IsPointInForwardSector2D( entityToShootAt->GetPosition(), m_position, m_viewRange, m_fwdDir, m_shootAperatureDegrees );
}


//---------------------------------------------------------------------------------------------------------
bool NpcTank::IsEntityInDriveAperature( Entity* entityToShootAt )
{
	return IsPointInForwardSector2D( entityToShootAt->GetPosition(), m_position, m_viewRange, m_fwdDir, m_driveAperatureDegrees );
}
