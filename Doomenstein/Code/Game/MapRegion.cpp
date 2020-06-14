#include "Game/MapRegion.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapMaterial.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::map<std::string, MapRegion*> MapRegion::s_mapRegions;


//---------------------------------------------------------------------------------------------------------
MapRegion::MapRegion( XmlElement const& xmlElement )
{
	m_name		= ParseXmlAttribute( xmlElement, "name", "MISSING" );
	m_isSolid	= ParseXmlAttribute( xmlElement, "isSolid", m_isSolid );

	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string materialName = ParseXmlAttribute( *nextChildElement, "material", "MISSING" );
		MapMaterial* materialToUse = MapMaterial::GetMaterialByName( materialName );

		char const* elementType = nextChildElement->Name();
		if( IsStringEqual( elementType, "Floor" ) )
		{
			m_floorMaterial = materialToUse;
		}
		else if( IsStringEqual( elementType, "Ceiling" ) )
		{
			m_ceilingMaterial = materialToUse;
		}
		else if( IsStringEqual( elementType, "Side" ) )
		{
			m_sideMaterial = materialToUse;
		}
		nextChildElement = nextChildElement->NextSiblingElement();
	}

	s_mapRegions.insert( { m_name, this } );
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapRegion::CreateMapRegionsFromXML( char const* filepath )
{
	const char* elementNameToLoad = "MapRegionTypes";

	XmlDocument mapMaterialTypesFile = new XmlDocument();
	mapMaterialTypesFile.LoadFile( filepath );
	if( mapMaterialTypesFile.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->ErrorString( "Failed to load %s for generating region types", filepath );
		g_theConsole->ErrorString( "  Error: %s", mapMaterialTypesFile.ErrorName() );
		g_theConsole->ErrorString( "  Error Line #%i", mapMaterialTypesFile.ErrorLineNum() );
		return;
	}

	const XmlElement* rootElement = mapMaterialTypesFile.RootElement();
	const XmlElement* nextChildElement = rootElement->FirstChildElement();

	if( !IsStringEqual( rootElement->Name(), elementNameToLoad ) )
	{
		g_theConsole->ErrorString( "Tried to load element %s - should be %s", rootElement->Name(), elementNameToLoad );
	}

	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		new MapRegion( *nextChildElement );
		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC MapRegion* MapRegion::GetRegionByName( std::string regionTypeName )
{
	return s_mapRegions[ regionTypeName ];
}

