#include "Game/Buff.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Buff::Buff( XmlElement const& xmlElement )
	: Ability( xmlElement )
{
	
	XmlElement const* valueElement = xmlElement.FirstChildElement( "values" );
	XmlElement const* nextValueChild = valueElement->FirstChildElement();
	for( ;; )
	{
		if( nextValueChild == nullptr )
			break;

		std::string childElementName = nextValueChild->Name();
		if( childElementName == "Modify" )
		{
			std::string statToModAsString = ParseXmlAttribute( *nextValueChild, "stat", "INVALID" );
			float amountToMod = ParseXmlAttribute( *nextValueChild, "amount", 0.f );
			m_statMods.emplace_back( Actor::GetStatTypeFromString( statToModAsString ), amountToMod );
		}
		nextValueChild = nextValueChild->NextSiblingElement();
	}

	m_type		= ABILITY_TYPE_BUFF;
	m_tint		= ParseXmlAttribute( *valueElement, "tint", m_tint );
	m_duration	= ParseXmlAttribute( *valueElement, "duration", m_duration );
}


//---------------------------------------------------------------------------------------------------------
Buff::Buff( Buff const& copyFrom )
	: Ability( copyFrom )
	, m_tint( copyFrom.m_tint )
	, m_duration( copyFrom.m_duration )
	, m_statMods( copyFrom.m_statMods )
{
}


//---------------------------------------------------------------------------------------------------------
Buff::~Buff()
{
}


//---------------------------------------------------------------------------------------------------------
void Buff::Use()
{
	if( IsOnCooldown() )
		return;

	Ability::Use();

	for( uint statModIndex = 0; statModIndex < m_statMods.size(); ++statModIndex )
	{
		m_owner->AddStatusEffect( m_statMods[statModIndex], m_duration );
	}
}
