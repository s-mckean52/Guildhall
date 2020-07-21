#include "Game/Portal.hpp"
#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"

//---------------------------------------------------------------------------------------------------------
Portal::Portal( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element )
	: Entity( theGame, theWorld, theMap, entityDef, element )
{
	m_mass					= 1.f;
	m_canBePushedByWalls	= false;
	m_canBePushedByEntities = false;
	m_canPushEntities		= false;
	
	SetValuesFromXML( element );
}


//---------------------------------------------------------------------------------------------------------
Portal::~Portal()
{
}


//---------------------------------------------------------------------------------------------------------
void Portal::SetValuesFromXML( XmlElement const& element )
{
	const float	invalidYaw = ~0;
	const Vec2	invalidPos = Vec2( -1.f, -1.f );

	m_destinationMapName = ParseXmlAttribute( element, "destMap", "" );
// 	if( m_destinationMapName == "MISSING" )
// 	{
// 		g_theConsole->ErrorString( "Failed to parse \"destMap\" for Portal at line %i", element.GetLineNum() );
// 	}
	
	m_destinationPos = ParseXmlAttribute( element, "destPos", invalidPos );
	if( m_destinationPos == invalidPos )
	{
		m_destinationPos = Vec2( 1.f, 1.f );
		g_theConsole->ErrorString( "Failed to parse \"destPos\" of Portal at line %i", element.GetLineNum() );
		g_theConsole->ErrorString( "Destination Position set to ( 1, 1 )" );
	}

	m_destinationYawOffset = ParseXmlAttribute( element, "destYawOffset", invalidYaw );
	if( m_destinationYawOffset == invalidYaw )
	{
		m_destinationYawOffset = 0.f;
		g_theConsole->ErrorString( "Failed to parse \"destYawOffset\" of Portal at line %i", element.GetLineNum() );
		g_theConsole->ErrorString( "Destination Yaw Offset set to 0 degrees" );
	}
}


//---------------------------------------------------------------------------------------------------------
void Portal::UsePortal( Entity* entityUsingPortal )
{
	Map* mapToTeleportTo = nullptr;
	if( m_destinationMapName == "" )
	{
		mapToTeleportTo = m_theWorld->GetCurrentMap();
	}
	else
	{
		mapToTeleportTo = m_theWorld->GetLoadedMapByName( m_destinationMapName );
		if( mapToTeleportTo == nullptr )
		{
			g_theConsole->ErrorString( "The map \"%s\" does not exist", m_destinationMapName.c_str() );
			return;
		}
	}

	//g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, "Using teleporter to map %s...", m_destinationMapName.c_str() );
	if( mapToTeleportTo != m_theWorld->GetCurrentMap() && entityUsingPortal->IsPossessed() )
	{
		m_theWorld->SetCurrentMap( mapToTeleportTo );
		m_theMap->RemoveEntityFromMap( entityUsingPortal );
		mapToTeleportTo->AddEntityToMap( entityUsingPortal );

		entityUsingPortal->SetPosition( Vec3( m_destinationPos, 0.f ) );
		entityUsingPortal->AddYaw( m_destinationYawOffset );
	}
	else if( mapToTeleportTo == m_theWorld->GetCurrentMap() )
	{
		entityUsingPortal->SetPosition( Vec3( m_destinationPos, 0.f ) );
		entityUsingPortal->AddYaw( m_destinationYawOffset );
	}
}