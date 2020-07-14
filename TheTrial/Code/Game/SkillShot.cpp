#include "Game/SkillShot.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
SkillShot::SkillShot( XmlElement const& xmlElement )
	: Ability( xmlElement )
{
	m_type = ABILITY_TYPE_SKILLSHOT;

	XmlElement const& valueElement = *xmlElement.FirstChildElement( "values" );
	m_range = ParseXmlAttribute( valueElement, "range", m_range );
	m_projectileSpeed = ParseXmlAttribute( valueElement, "speed", m_projectileSpeed );
	m_numProjectiles = ParseXmlAttribute( valueElement, "numProjectiles", m_numProjectiles );
	m_baseDamagePerProjectile = ParseXmlAttribute( valueElement, "baseDamage", m_baseDamagePerProjectile );
	m_aimAperatureDegrees = ParseXmlAttribute( valueElement, "aperture", m_aimAperatureDegrees );

	XmlElement const* nextValueChild = valueElement.FirstChildElement( "Damage" );
	std::string statToModAsString = ParseXmlAttribute( *nextValueChild, "type", "INVALID" );
	m_statToScale = Item::GetStatTypeFromString( statToModAsString );
	m_scaleFraction = ParseXmlAttribute( *nextValueChild, "fraction", m_scaleFraction );
}


//---------------------------------------------------------------------------------------------------------
SkillShot::SkillShot( SkillShot const& copyFrom )
	: Ability( copyFrom )
	, m_range( copyFrom.m_range )
	, m_projectileSpeed( copyFrom.m_projectileSpeed )
	, m_numProjectiles( copyFrom.m_numProjectiles )
	, m_baseDamagePerProjectile( copyFrom.m_baseDamagePerProjectile )
	, m_statToScale( copyFrom.m_statToScale )
	, m_scaleFraction( copyFrom.m_scaleFraction )
	, m_aimAperatureDegrees( copyFrom.m_aimAperatureDegrees )
{
}


//---------------------------------------------------------------------------------------------------------
SkillShot::~SkillShot()
{
}


//---------------------------------------------------------------------------------------------------------
void SkillShot::Use()
{
	if( IsOnCooldown() )
		return;

	Ability::Use();
	
	float statValue = 0.f;
	switch( m_statToScale )
	{
	case STAT_CRIT_MULTIPLIER:	statValue = m_owner->GetCritMultiplier();						break;
	case STAT_CRIT_CHANCE:		statValue = m_owner->GetCritChanceFraction();					break;
	case STAT_MOVEMENT_SPEED:	statValue = m_owner->GetMoveSpeed();							break;
	case STAT_ATTACK_SPEED:		statValue = m_owner->GetAttackSpeed();							break;
	case STAT_ATTACK_DAMAGE:	statValue = static_cast<float>( m_owner->GetAttackDamage() );	break;

	default:
		break;
	}
	int damagePerProjectile = static_cast<int>( statValue * m_scaleFraction );

	float angleDegreesBetweenProjectiles = m_aimAperatureDegrees / ( m_numProjectiles - 1 );
	Vec2 aimDirection = g_theGame->GetCursorPosition() - m_owner->GetCurrentPosition();
	aimDirection.Normalize();

	float startAngleOffset = static_cast<float>( m_numProjectiles / 2 ) * angleDegreesBetweenProjectiles;
	Vec2 projectileDirection = aimDirection.GetRotatedDegrees( -startAngleOffset );
	for( int projectileNum = 0; projectileNum < m_numProjectiles; ++projectileNum )
	{
		Projectile* spawnedProjectile = new Projectile( m_theGame, damagePerProjectile, m_projectileSpeed, projectileDirection, m_range );
		spawnedProjectile->SetCurrentPosition( m_owner->GetCurrentPosition() );
		projectileDirection.RotateDegrees( angleDegreesBetweenProjectiles );
		
		Map* map = m_theGame->GetCurrentMap();
		map->AddEntityToList( spawnedProjectile );
	}
}