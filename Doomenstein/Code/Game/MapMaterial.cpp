#include "Game/MapMaterial.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::map<std::string, MapMaterial*> MapMaterial::s_mapMaterials;
STATIC std::map<std::string, SpriteSheet*> MapMaterial::s_materialSheets;

//---------------------------------------------------------------------------------------------------------
MapMaterial::MapMaterial( XmlElement const& xmlElement )
{
	IntVec2 spritePosition = IntVec2( -1, -1 );

	m_name			= ParseXmlAttribute( xmlElement, "name", "MISSING" );
	m_sheetName		= ParseXmlAttribute( xmlElement, "sheet", "MISSING" );
	spritePosition	= ParseXmlAttribute( xmlElement, "spriteCoords", spritePosition );

	SpriteSheet* spriteSheetToUse = s_materialSheets[ m_sheetName ];
	SpriteDefinition spriteDefinition = spriteSheetToUse->GetSpriteDefinitionByPosition( spritePosition );
	spriteDefinition.GetUVs( m_spriteUVBox.mins, m_spriteUVBox.maxes );

	s_mapMaterials.insert( { m_name, this } );
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapMaterial::CreateMaterialSheet( XmlElement const& xmlElement )
{
	std::string sheetName = ParseXmlAttribute( xmlElement, "name", "MISSING" );
	IntVec2 dimensions =	ParseXmlAttribute( xmlElement, "layout", IntVec2( -1, -1 ) );

	const XmlElement& childElement = *xmlElement.FirstChildElement( "Diffuse" );
	std::string filePath = ParseXmlAttribute( childElement, "image", "MISSING" );

	Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile( filePath.c_str() );
	SpriteSheet* newSpriteSheet = new SpriteSheet( *spriteTexture, dimensions );

	s_materialSheets.insert( { sheetName, newSpriteSheet } );
}


//---------------------------------------------------------------------------------------------------------
STATIC MapMaterial* MapMaterial::GetMaterialByName( std::string materialName )
{
	return s_mapMaterials[ materialName ];
}


//---------------------------------------------------------------------------------------------------------
STATIC void MapMaterial::CreateMapMaterialsFromXML( char const* filepath )
{
	const char* elementNameToLoad = "MapMaterialTypes";

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
	const XmlElement* nextChildElement = rootElement->FirstChildElement();

	if( !IsStringEqual( rootElement->Name(), elementNameToLoad ) )
	{
		g_theConsole->ErrorString( "Tried to load element %s - should be %s", rootElement->Name(), elementNameToLoad );
	}

	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		if( IsStringEqual( nextChildElement->Name(), "MaterialsSheet" ) )
		{
			CreateMaterialSheet( *nextChildElement );
		}
		else if( IsStringEqual( nextChildElement->Name(), "MaterialType" ) )
		{
			new MapMaterial( *nextChildElement );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
	}
}

