#include "Game/MapMaterial.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::string							MapMaterial::s_defaultMaterialName = "";
STATIC std::map<std::string, MapMaterial*>	MapMaterial::s_mapMaterials;
STATIC std::map<std::string, SpriteSheet*>	MapMaterial::s_materialSheets;

//---------------------------------------------------------------------------------------------------------
MapMaterial::MapMaterial( XmlElement const& xmlElement )
{
	Strings errorStrings;
	IntVec2 spritePosition = IntVec2( -1, -1 );

	m_name = ParseXmlAttribute( xmlElement, "name", "MISSING" );
	if( m_name == "MISSING" )
	{
		errorStrings.push_back( "  Could not parse attribute: 'name'" );
	}

	SpriteSheet* spriteSheetToUse = nullptr;
	std::string sheetName = ParseXmlAttribute( xmlElement, "sheet", "MISSING" );
	if( sheetName == "MISSING" )
	{
		errorStrings.push_back( "  Could not parse attribute: 'sheet'" );
	}
	else
	{
		spriteSheetToUse = s_materialSheets[sheetName];
		m_spriteTexture = &spriteSheetToUse->GetTexture();
		if( spriteSheetToUse == nullptr )
		{
			errorStrings.push_back( Stringf( "  Failed to find sprite sheet with name: %s", sheetName.c_str() ) );
		}
	}

	spritePosition	= ParseXmlAttribute( xmlElement, "spriteCoords", spritePosition );
	if( spritePosition == IntVec2( -1, -1 ) )
	{
		errorStrings.push_back( "  Could not parse attribute: 'spriteCoords'" );
	}
	else if( spriteSheetToUse != nullptr )
	{
		if( spritePosition.x > spriteSheetToUse->GetGridSize().x || spritePosition.y > spriteSheetToUse->GetGridSize().y )
		{
			errorStrings.push_back( Stringf( "  Specified sprite coords are out of range on sprite sheet %s", sheetName .c_str() ) );
		}
		else
		{
			SpriteDefinition spriteDefinition = spriteSheetToUse->GetSpriteDefinitionByPosition( spritePosition );
			spriteDefinition.GetUVs( m_spriteUVBox.mins, m_spriteUVBox.maxes );
		}
	}

	if( errorStrings.size() > 0 )
	{
		g_theConsole->ErrorString( "Failed to load Map Material at line #%i", xmlElement.GetLineNum() );
		for( int errorMessageIndex = 0; errorMessageIndex < errorStrings.size(); ++errorMessageIndex )
		{
			g_theConsole->ErrorString( errorStrings[ errorMessageIndex ].c_str() );
		}
		return;
	}

	s_mapMaterials.insert( { m_name, this } );
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapMaterial::CreateMaterialSheet( XmlElement const& xmlElement )
{
	Strings errorMessages;
	std::string diffuseFilePath;

	std::string sheetName = ParseXmlAttribute( xmlElement, "name", "MISSING" );
	if( sheetName == "MISSING" )
	{
		errorMessages.push_back( Stringf( "  Failed to parse attribute: 'name'", xmlElement.GetLineNum() ) );
	}

	IntVec2 dimensions = ParseXmlAttribute( xmlElement, "layout", IntVec2( -1, -1 ) );
	if( dimensions == IntVec2( -1, -1 ) )
	{
		errorMessages.push_back( Stringf( "  Failed to parse attribute: 'layout'", xmlElement.GetLineNum() ) );
	}

	const XmlElement* childElement = xmlElement.FirstChildElement( "Diffuse" );
	if( childElement != nullptr )
	{
		diffuseFilePath = ParseXmlAttribute( *childElement, "image", "MISSING" );
		if( sheetName == "MISSING" )
		{
			errorMessages.push_back( "  Failed to parse Diffuse attribute: 'image'" );
		}
	}
	else
	{
		errorMessages.push_back( "  Failed to find child element: 'Diffuse'" );
	}

	if( errorMessages.size() > 0 )
	{
		g_theConsole->ErrorString( "Failed to load Material Sheet at line #%i:", xmlElement.GetLineNum() );
		for( int errorMessageIndex = 0; errorMessageIndex < errorMessages.size(); ++errorMessageIndex )
		{
			g_theConsole->ErrorString( errorMessages[errorMessageIndex].c_str() );
		}
		return;
	}

	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( diffuseFilePath.c_str() );
	SpriteSheet* newSpriteSheet = new SpriteSheet( *spriteTexture, dimensions );

	s_materialSheets.insert( { sheetName, newSpriteSheet } );
}


//---------------------------------------------------------------------------------------------------------
STATIC MapMaterial* MapMaterial::GetMaterialByName( std::string materialName )
{
	return s_mapMaterials[ materialName ];
}


//---------------------------------------------------------------------------------------------------------
STATIC MapMaterial* MapMaterial::GetDefaultMaterial()
{
	return s_mapMaterials[ s_defaultMaterialName ];
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapMaterial::CreateMapMaterialsFromXML( char const* filepath )
{
	g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, "Loading Map Materials from %s...", filepath );

	std::string const elementNameToLoad		= "MapMaterialTypes";
	std::string const materialSheetNodeName	= "MaterialsSheet";
	std::string const materialTypeNodeName	= "MaterialType";

	XmlDocument mapMaterialTypesFile = new XmlDocument();
	mapMaterialTypesFile.LoadFile( filepath );
	if( mapMaterialTypesFile.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->ErrorString( "Failed to load %s for generating material types", filepath );
		g_theConsole->ErrorString( "  Error: %s", mapMaterialTypesFile.ErrorName() );
		g_theConsole->ErrorString( "  Error Line #%i", mapMaterialTypesFile.ErrorLineNum() );
		return;
	}

	const XmlElement* rootElement = mapMaterialTypesFile.RootElement();
	if( !IsStringEqual( rootElement->Name(), elementNameToLoad.c_str() ) )
	{
		g_theConsole->ErrorString( "Tried to load element %s - should be %s", rootElement->Name(), elementNameToLoad.c_str() );
	}

	s_defaultMaterialName = ParseXmlAttribute( *rootElement, "default", "MISSING" );
	if( s_defaultMaterialName == "MISSING" )
	{
		g_theConsole->ErrorString( "Could not parse attribute: 'default' in root element" );
	}

	const XmlElement* nextChildElement = rootElement->FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string nextChildName = nextChildElement->Name();
		if( nextChildName ==  materialSheetNodeName )
		{
			CreateMaterialSheet( *nextChildElement );
		}
		else if( nextChildName == materialTypeNodeName )
		{
			new MapMaterial( *nextChildElement );
		}
		else
		{
			g_theConsole->ErrorString(							"Unrecognized Node Type: %s", nextChildElement->Name() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"Supported Node Types:" );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", materialSheetNodeName.c_str() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", materialTypeNodeName.c_str() );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
	}
}

