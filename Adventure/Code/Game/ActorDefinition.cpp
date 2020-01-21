#include "Game/ActorDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/StringUtils.hpp"

std::map< std::string, ActorDefinition* > ActorDefinition::s_actorDefinitions;


//---------------------------------------------------------------------------------------------------------
ActorDefinition::ActorDefinition( const XmlElement& element )
{
	m_name		= ParseXmlAttribute( element, "name", m_name );
	m_faction	= ParseXmlAttribute( element, "faction", m_faction );
	m_defaultSpriteIndex = ParseXmlAttribute( element, "defaultSpriteIndex", m_defaultSpriteIndex );

	const XmlElement* actorDefElement = element.FirstChildElement( "Size" );
	if( actorDefElement )
	{
		m_physicsRadius = ParseXmlAttribute( *actorDefElement, "physicsRadius", m_physicsRadius );
		m_spriteBounds	= ParseXmlAttribute( *actorDefElement, "localDrawBounds", m_spriteBounds );
	}

	actorDefElement = actorDefElement->NextSiblingElement( "Movement" );
	if( actorDefElement )
	{
		m_speed			= ParseXmlAttribute( *actorDefElement, "speed", m_speed );
	}

	actorDefElement = actorDefElement->NextSiblingElement( "Health" );
	if( actorDefElement )
	{
		m_maxHealth		= ParseXmlAttribute( *actorDefElement, "max", m_maxHealth );
		m_startHealth	= ParseXmlAttribute( *actorDefElement, "start", m_startHealth );
	}

	actorDefElement = actorDefElement->NextSiblingElement( "SpriteAnimSet" );
	if( actorDefElement )
	{
		std::string spriteSheetName = "";
		IntVec2		spriteLayout;
		int			fps = 0;
		spriteSheetName = ParseXmlAttribute( *actorDefElement, "spriteSheet", spriteSheetName );
		spriteLayout	= ParseXmlAttribute( *actorDefElement, "spriteLayout", spriteLayout );
		fps				= ParseXmlAttribute( *actorDefElement, "fps", fps );
		std::string imageFileFolder = Stringf( "Data/Images/%s", spriteSheetName.c_str() );
		Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile( imageFileFolder.c_str() );
		m_spriteSheet = new SpriteSheet( *spriteSheetTexture, spriteLayout );
	}
}


//---------------------------------------------------------------------------------------------------------
void ActorDefinition::InitializeActorDefinitions()
{
	XmlDocument tileDefinitionXml = new XmlDocument();
	tileDefinitionXml.LoadFile( "Data/Definitions/ActorDefinitions.xml" );
	GUARANTEE_OR_DIE( tileDefinitionXml.ErrorID() == 0, "ActorDefinitions.xml does not exist in Run/Data/Definitions" );

	const XmlElement* rootElement = tileDefinitionXml.RootElement();
	const XmlElement* nextDefinition = rootElement->FirstChildElement();

	while( nextDefinition )
	{
		ActorDefinition* newTileDefinition = new ActorDefinition( *nextDefinition );
		s_actorDefinitions[ newTileDefinition->m_name ] = newTileDefinition;
		nextDefinition = nextDefinition->NextSiblingElement();
	}
}
