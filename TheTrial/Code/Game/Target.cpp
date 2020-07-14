#include "Game/Target.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Enemy.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Target::Target( XmlElement const& xmlElement )
	: Ability( xmlElement )
{
	m_type = ABILITY_TYPE_TARGET;
	XmlElement const& valueElement = *xmlElement.FirstChildElement( "values" );

	m_range = ParseXmlAttribute( valueElement, "range", m_range );
	m_duration = ParseXmlAttribute( valueElement, "duration", m_duration );
	m_numTicks = ParseXmlAttribute( valueElement, "numTicks", m_numTicks );
	m_damagePerTick = ParseXmlAttribute( valueElement, "damagePerTick", m_damagePerTick );

	XmlElement const* nextValueChild = valueElement.FirstChildElement();
	for( ;; )
	{
		if( nextValueChild == nullptr )
			break;

		std::string childElementName = nextValueChild->Name();
		if( childElementName == "Modify" )
		{
			std::string statToModAsString = ParseXmlAttribute( *nextValueChild, "stat", "INVALID" );
			float amountToMod = ParseXmlAttribute( *nextValueChild, "amount", 0.f );
			m_statMods.emplace_back( Item::GetStatTypeFromString( statToModAsString ), amountToMod );
		}
		nextValueChild = nextValueChild->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
Target::Target( Target const& copyFrom )
	: Ability( copyFrom )
	, m_duration( copyFrom.m_duration )
	, m_numTicks( copyFrom.m_numTicks )
	, m_damagePerTick( copyFrom.m_damagePerTick )
	, m_range( copyFrom.m_range )
	, m_statMods( copyFrom.m_statMods )
{
}


//---------------------------------------------------------------------------------------------------------
Target::~Target()
{
}


//---------------------------------------------------------------------------------------------------------
void Target::Use()
{
	if( IsOnCooldown() )
		return;

	Enemy* targetedEnemy = g_theGame->GetHoveredEnemy();
	if( targetedEnemy == nullptr )
		return;

	if( GetDistance2D( m_owner->GetCurrentPosition(), targetedEnemy->GetCurrentPosition() ) <= m_range )
	{
		Ability::Use();

		for( uint statModIndex = 0; statModIndex < m_statMods.size(); ++statModIndex )
		{
			targetedEnemy->AddStatusEffect( m_statMods[statModIndex], m_duration );
		}
	}
}
