#include "Game/MapRegion.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapMaterial.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::string							MapRegion::s_defaultMapRegionName = "";
STATIC std::map<std::string, MapRegion*>	MapRegion::s_mapRegions;


//---------------------------------------------------------------------------------------------------------
MapRegion::MapRegion( XmlElement const& xmlElement )
{
	Strings errorMessages;

	m_isSolid = ParseXmlAttribute( xmlElement, "isSolid", m_isSolid );
	m_name = ParseXmlAttribute( xmlElement, "name", "MISSING" );
	if( m_name == "MISSING" )
	{
		errorMessages.push_back( "  Failed to parse attribute: 'name'" );
	}

	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string materialName = ParseXmlAttribute( *nextChildElement, "material", "MISSING" );
		if( materialName == "MISSING" )
		{
			errorMessages.push_back( Stringf( "  Failed to parse element %s's attribute: 'material'", nextChildElement->Name() ) );
		}

		MapMaterial* materialToUse = MapMaterial::GetMaterialByName( materialName );
		if( materialToUse == nullptr )
		{
			errorMessages.push_back( Stringf( "  Using default material for element '%s'", nextChildElement->Name() ) );
			materialToUse = MapMaterial::GetDefaultMaterial();
			if( materialToUse == nullptr )
			{
				errorMessages.push_back( "  Failed to load default material - no material loaded" );
			}
		}

		char const* elementType = nextChildElement->Name();
		if( IsStringEqual( elementType, "Floor" ) )
		{
			if( m_floorMaterial != nullptr )
				errorMessages.push_back( Stringf( "  Only one floor material may be specified. Using %s", m_floorMaterial->GetName().c_str() ) );
			else if( m_isSolid )
				errorMessages.push_back( Stringf( "  Solid tiles only use side material" ) );
			else
				m_floorMaterial = materialToUse;
		}
		else if( IsStringEqual( elementType, "Ceiling" ) )
		{
			if( m_ceilingMaterial != nullptr )
				errorMessages.push_back( Stringf( "  Only one ceiling material may be specified. Using %s", m_ceilingMaterial->GetName().c_str() ) );
			else if( m_isSolid )
				errorMessages.push_back( Stringf( "  Solid tiles only use side material" ) );
			else
				m_ceilingMaterial = materialToUse;
		}
		else if( IsStringEqual( elementType, "Side" ) )
		{
			if( m_sideMaterial != nullptr )
				errorMessages.push_back( Stringf( "  Only one side material may be specified. Using %s", m_sideMaterial->GetName().c_str() ) );
			else if( !m_isSolid )
				errorMessages.push_back( Stringf( "  Open tiles only use ceiling/floor materials" ) );
			else
				m_sideMaterial = materialToUse;
		}
		else
		{
			errorMessages.push_back( Stringf(	"  Unsupported element type of %s used", elementType ) );
			errorMessages.push_back( Stringf(	"  Supported Types:", elementType ) );
			errorMessages.push_back(			"    Floor" );
			errorMessages.push_back(			"    Ceiling" );
			errorMessages.push_back(			"    Side" );
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}
	
	if( errorMessages.size() > 0 )
	{
		g_theConsole->ErrorString( "Failed to load Material Region at line #%i", xmlElement.GetLineNum() );
		for( int errorMessageIndex = 0; errorMessageIndex < errorMessages.size(); ++errorMessageIndex )
		{
			g_theConsole->ErrorString( errorMessages[errorMessageIndex].c_str() );
		}
		return;
	}

	s_mapRegions.insert( { m_name, this } );
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapRegion::CreateMapRegionsFromXML( char const* filepath )
{
	g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, "Loading Map Regions from %s...", filepath );

	std::string elementNameToLoad = "MapRegionTypes";

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

	if( rootElement->Name() != elementNameToLoad )
	{
		g_theConsole->ErrorString( "Tried to load root %s - should be %s", rootElement->Name(), elementNameToLoad.c_str() );
	}

	s_defaultMapRegionName = ParseXmlAttribute( *rootElement, "default", "MISSING" );
	if( s_defaultMapRegionName == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to get 'default' Map Region Name" );
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


//---------------------------------------------------------------------------------------------------------
STATIC MapRegion* MapRegion::GetDefaultRegion()
{
	return s_mapRegions[ s_defaultMapRegionName ];
}

