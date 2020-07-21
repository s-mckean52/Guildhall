#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::map< std::string, TileDefinition* > TileDefinition::s_tileDefinitions;
std::map< std::string, SpriteSheet* > TileDefinition::s_spriteSheets;


//---------------------------------------------------------------------------------------------------------
void TileDefinition::CreateSpriteSheet( XmlElement const& element )
{
	std::string name				= ParseXmlAttribute( element, "name", "TestTerrain" );
	IntVec2 spriteSheetDimensions	= ParseXmlAttribute( element, "layout", IntVec2( 8, 8 ) );

	XmlElement const& diffuseElement = *element.FirstChildElement( "Diffuse" );
	std::string diffuseFilepath = ParseXmlAttribute( diffuseElement, "image", "Data/Images/Terrain_8x8.png" );

	Texture* diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile( diffuseFilepath.c_str() );
	s_spriteSheets[ name ] = new SpriteSheet( *diffuseTexture, spriteSheetDimensions );
}


//---------------------------------------------------------------------------------------------------------
void TileDefinition::CreateTileDefinition( XmlElement const& element )
{
	TileDefinition* newTileDefinition = new TileDefinition( element );
	s_tileDefinitions[newTileDefinition->m_name] = newTileDefinition;
}


//---------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( const XmlElement& definitionXmlElement )
{
	m_name				= ParseXmlAttribute( definitionXmlElement, "name", m_name );
	m_spriteTint		= ParseXmlAttribute( definitionXmlElement, "spriteTint", m_spriteTint );
	m_texelColorForSet	= ParseXmlAttribute( definitionXmlElement, "texelColorForSet", m_texelColorForSet );
	m_isSolid			= ParseXmlAttribute( definitionXmlElement, "isSolid", m_isSolid );

	std::string sheetName		= ParseXmlAttribute( definitionXmlElement, "sheet", "TestTerrain" );
	IntVec2		spriteCoords	= ParseXmlAttribute( definitionXmlElement, "spriteCoords", IntVec2( 0, 0 ) );

	m_spriteSheet = s_spriteSheets[ sheetName ];
	m_spriteSheet->GetSpriteUVs( m_spriteUVBox.mins, m_spriteUVBox.maxes, spriteCoords );
}


//---------------------------------------------------------------------------------------------------------
void TileDefinition::InitializeTileDefinitions()
{
	XmlDocument tileDefinitionXml = new XmlDocument();
	tileDefinitionXml.LoadFile( "Data/Definitions/TileDefinitions.xml" );
	GUARANTEE_OR_DIE( tileDefinitionXml.ErrorID() == 0, "TileDefinitions.xml does not exist in Run/Data/Definitions" );

	const XmlElement* rootElement = tileDefinitionXml.RootElement();
	const XmlElement* nextDefinition = rootElement->FirstChildElement();

	while( nextDefinition )
	{
		std::string nodeName = nextDefinition->Name();
		if( nodeName == "SpriteSheet" )
		{
			CreateSpriteSheet( *nextDefinition );
		}
		else if( nodeName == "TileDefinition" )
		{
			CreateTileDefinition( *nextDefinition );
		}
		nextDefinition = nextDefinition->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
TileDefinition* TileDefinition::GetTileDefByName( std::string const& tileDefName )
{
	TileDefinition* tileDef = s_tileDefinitions[ tileDefName ];
	if( tileDef != nullptr )
	{
		return tileDef;
	}
	return nullptr;
}


//---------------------------------------------------------------------------------------------------------
TileDefinition* TileDefinition::GetTileDefWithSetColor( const Rgba8& texelColor )
{
	const unsigned char colorTolerance = 1;

	auto tileDefIterator = s_tileDefinitions.begin();
	while( tileDefIterator != s_tileDefinitions.end() )
	{
		TileDefinition* currentTileDef = tileDefIterator->second;
		Rgba8 tileColorToSet = currentTileDef->GetTexelColorForSet();
		if( ( tileColorToSet.r >= texelColor.r - colorTolerance && tileColorToSet.r <= texelColor.r + colorTolerance ) && 
			( tileColorToSet.g >= texelColor.g - colorTolerance && tileColorToSet.g <= texelColor.g + colorTolerance ) &&
			( tileColorToSet.b >= texelColor.b - colorTolerance && tileColorToSet.b <= texelColor.b + colorTolerance ) )
		{
			return currentTileDef;
		}
		++tileDefIterator;
	}
	return nullptr;
}
