#include "Game/TileDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::map< std::string, TileDefinition* > TileDefinition::s_tileDefinitions;


//---------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( const XmlElement& definitionXmlElement )
{
	m_name				= ParseXmlAttribute( definitionXmlElement, "name", m_name );
	m_spriteTint		= ParseXmlAttribute( definitionXmlElement, "spriteTint", m_spriteTint );
	m_texelColorForSet	= ParseXmlAttribute( definitionXmlElement, "texelColorForSet", m_texelColorForSet );
	m_allowSight		= ParseXmlAttribute( definitionXmlElement, "allowsSight", m_allowSight);
	m_allowSwimming		= ParseXmlAttribute( definitionXmlElement, "allowsSwimming", m_allowSwimming );
	m_allowWalking		= ParseXmlAttribute( definitionXmlElement, "allowsWalking", m_allowWalking );
	m_allowFlying		= ParseXmlAttribute( definitionXmlElement, "allowsFlying", m_allowFlying	);

	IntVec2 spriteCoords = ParseXmlAttribute( definitionXmlElement, "spriteCoords", IntVec2( 0, 0 ) );
	g_tileSpriteSheet->GetSpriteUVs( m_spriteUVBox.mins, m_spriteUVBox.maxes, spriteCoords );
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
		TileDefinition* newTileDefinition = new TileDefinition( *nextDefinition );
		s_tileDefinitions[ newTileDefinition->m_name ] = newTileDefinition;
		nextDefinition = nextDefinition->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
TileDefinition* TileDefinition::GetTileDefWithSetColor( const Rgba8& texelColor )
{
	std::map< std::string, TileDefinition* >::iterator tileDefIterator = s_tileDefinitions.begin();
	while( tileDefIterator != s_tileDefinitions.end() )
	{
		TileDefinition* currentTileDef = tileDefIterator->second;
		Rgba8 tileColorToSet = currentTileDef->GetTexelColorForSet();
		if( tileColorToSet.r == texelColor.r && tileColorToSet.g == texelColor.g && tileColorToSet.b == texelColor.b )
		{
			return currentTileDef;
		}
		tileDefIterator++;
	}
	return nullptr;
}
