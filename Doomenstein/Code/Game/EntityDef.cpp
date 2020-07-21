#include "Game/EntityDef.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::map<std::string, EntityDef*>	EntityDef::s_entityDefs;
STATIC std::map<std::string, SpriteSheet*>	EntityDef::s_spriteSheets;


//---------------------------------------------------------------------------------------------------------
EntityDef::EntityDef( XmlElement const& xmlElement, EntityType entityType )
{
	bool hasFailedToParse		= false;
	bool hasParsedPhysics		= false;
	bool hasParsedAppearance	= false;

	m_entityType = entityType;

	m_name = ParseXmlAttribute( xmlElement, "name", "MISSING" );
	if( m_name == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse attribute \"name\" of entity def at line %i", xmlElement.GetLineNum() );
		return;
	}
	
	if( s_entityDefs[m_name] != nullptr )
	{
		g_theConsole->ErrorString( "Entity of name \"%s\" already exists", m_name.c_str() );
		return;
	}

	//if( entityType != ENTITY_TYPE_PORTAL )
	{
		XmlElement const* nextChildElement = xmlElement.FirstChildElement();
		for( ;; )
		{
			if( nextChildElement == nullptr )
				break;

			if( IsStringEqual( nextChildElement->Name(), "Physics" ) )
			{
				if( hasParsedPhysics )
				{
					g_theConsole->ErrorString( "Additional \"Physics\" child element found at line %s. Using first one defined", nextChildElement->GetLineNum() );
				}
				else if( !ParsePhysicsNode( *nextChildElement ) )
				{
					g_theConsole->ErrorString( "Failed to parse \"Physics\" child element of entity at line %i", xmlElement.GetLineNum() );
					hasFailedToParse = true;
					nextChildElement = nextChildElement->NextSiblingElement();
					continue;
				}
				hasParsedPhysics = true;
			}
			else if( IsStringEqual( nextChildElement->Name(), "Appearance" ) )
			{
				if( hasParsedAppearance )
				{
					g_theConsole->ErrorString( "Additional \"Appearance\" child element found at line %s. Using first one defined", nextChildElement->GetLineNum() );
				}
				else if( !ParseAppearanceNode( *nextChildElement ) )
				{
					g_theConsole->ErrorString( "Failed to parse \"Appearance\" child element at line %i", xmlElement.GetLineNum() );
					hasFailedToParse = true;
					nextChildElement = nextChildElement->NextSiblingElement();
					continue;
				}
				hasParsedAppearance = true;
			}
			else
			{
				g_theConsole->ErrorString( "Unsupported element type \"%s\" at line %i", nextChildElement->Name(), nextChildElement->GetLineNum() );
			}

			nextChildElement = nextChildElement->NextSiblingElement();
		}
	}

	if( hasFailedToParse )
	{
		g_theConsole->ErrorString( "Failed to create entity of name \"%s\"", m_name.c_str() );
		return;
	}
	s_entityDefs[ m_name ] = this;
}


//---------------------------------------------------------------------------------------------------------
bool EntityDef::HasAnimationDirectionInSet( std::string const& animationSetName, std::string const& directionName ) const
{
	SpriteDefinition const* directionAnimSprite = nullptr;

	auto animationSetIter = m_animations.find( animationSetName );
	if( animationSetIter != m_animations.end() )
	{
		SpriteAnimSet* animationSet = animationSetIter->second;
		directionAnimSprite = animationSet->GetSpriteAnimAtTime( directionName, 0.f );
	}

	if( directionAnimSprite != nullptr )
	{
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------------------------
std::string EntityDef::GetEntityTypeAsString() const
{
	std::string entityTypeAsString = "";
	switch( m_entityType )
	{
	case ENTITY_TYPE_ENTITY:		entityTypeAsString = "Entity"; break;
	case ENTITY_TYPE_ACTOR:			entityTypeAsString = "Actor"; break;
	case ENTITY_TYPE_PROJECTILE:	entityTypeAsString = "Projectile"; break;
	case ENTITY_TYPE_PORTAL:		entityTypeAsString = "Portal"; break;
	default:
		ERROR_AND_DIE( Stringf( "Failed to get entity type of entity def \"%s\" as string", m_name.c_str() ) );
		break;
	}
	return entityTypeAsString;
}


//---------------------------------------------------------------------------------------------------------
SpriteDefinition const* EntityDef::GetSpriteDefinitionForAnimSetAtTime( std::string const& animSetName, std::string const& direction, float time ) const
{
	auto animationsIter = m_animations.find( animSetName );
	if( animationsIter == m_animations.end() )
	{
		ERROR_AND_DIE( Stringf( "Failed to access anim set \"%s\" in entity def \"%s\"", animSetName.c_str(), m_name.c_str() ) );
	}
	SpriteAnimSet const* animSet = animationsIter->second;
	return animSet->GetSpriteAnimAtTime( direction, time );
}


//---------------------------------------------------------------------------------------------------------
bool EntityDef::ParsePhysicsNode( XmlElement const& element )
{
	m_physicsRadius = ParseXmlAttribute( element, "radius", -1.f );
	if( m_physicsRadius == -1.f )
	{
		g_theConsole->ErrorString( "Failed to parse Physics attribute \"radius\" at line %i", element.GetLineNum() );
		return false;
	}

	m_height = ParseXmlAttribute( element, "height", -1.f );
	if( m_height == -1.f )
	{
		g_theConsole->ErrorString( "Failed to parse Physics attribute \"height\" at line %i", element.GetLineNum() );
		return false;
	}

	if( m_entityType == ENTITY_TYPE_ACTOR )
	{
		m_eyeHeight = ParseXmlAttribute( element, "eyeHeight", -1.f );
		if( m_eyeHeight == -1.f )
		{
			g_theConsole->ErrorString( "Failed to parse Physics attribute \"eyeHeight\" at line %i", element.GetLineNum() );
			return false;
		}
		
		m_speed = ParseXmlAttribute( element, "walkSpeed", static_cast<float>( ~0 ) );
		if( m_speed == ~0 )
		{
			g_theConsole->ErrorString( "Failed to parse Physics attribute \"walkSpeed\" at line %i", element.GetLineNum() );
			return false;
		}
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool EntityDef::ParseAppearanceNode( XmlElement const& element )
{
	//Validate Size
	m_size = ParseXmlAttribute( element, "size", Vec2( -1.f, -1.f ) );
	if( m_size == Vec2( -1.f, -1.f ) )
	{
		g_theConsole->ErrorString( "Failed to parse Appearance attribute \"size\" at line %i", element.GetLineNum() );
		return false;
	}

	//Validate billboad type
	std::string billboardTypeAsString = ParseXmlAttribute( element, "billboard", "MISSING" );
	if( billboardTypeAsString == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse Appearance attribute \"billboard\" at line %i", element.GetLineNum());
		return false;
	}
	else
	{
		m_billboardType = GetBillboardTypeFromString( billboardTypeAsString.c_str() );
		if( m_billboardType == BillboardType::INVALID_BILLBOARD_TYPE )
		{
			g_theConsole->ErrorString( "Unsupported billboard type \"%s\"", billboardTypeAsString.c_str() );
			return false;
		}
	}

	//Generate SpriteSheet
	std::string spriteSheetFilepath = ParseXmlAttribute( element, "spriteSheet", "MISSING" );
	if( spriteSheetFilepath == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse Appearance attribute \"spriteSheet\" at line %i", element.GetLineNum());
		return false;
	}
	IntVec2 spriteSheetLayout = ParseXmlAttribute( element, "layout", IntVec2( -1, -1 ) );
	if( spriteSheetLayout == IntVec2( -1, -1 ) )
	{
		g_theConsole->ErrorString( "Failed to parse Appearance attribute \"layout\" at line %i", element.GetLineNum());
		return false;
	}
	m_spriteSheet = GetOrCreateEntitySpriteSheet( spriteSheetFilepath.c_str(), spriteSheetLayout );

	//Create Animation States
	XmlElement const* nextChildElement = element.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		CreateAnimState( *nextChildElement );
		nextChildElement = nextChildElement->NextSiblingElement();
	}
	return true;
}



//---------------------------------------------------------------------------------------------------------
bool EntityDef::CreateAnimState( XmlElement const& element )
{
	std::string animStateName = element.Name();
	if( animStateName != "Walk" && animStateName != "Attack" && animStateName != "Pain" && animStateName != "Death" )
	{
		g_theConsole->ErrorString(							"Failed to create unsupported Anim State \"%s\" at line %i", element.GetLineNum() );
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"Supported Anim States:" );
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Walk" );
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Attack" );
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Pain" );
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Death" );
		return false;
	}

	if( m_animations[ animStateName ] != nullptr )
	{
		g_theConsole->ErrorString( "Anim State \"%s\" at line %i has already been created", animStateName.c_str(), element.GetLineNum() );
		return false;
	}

	//Create New Anim State
	XmlAttribute const* nextAttribute = element.FirstAttribute();
	std::map<std::string, SpriteAnimDefinition*> animStateMap;
	for( ;; )
	{
		if( nextAttribute == nullptr )
			break;

		char const* nextAttributeName = nextAttribute->Name();
		if( animStateMap[nextAttributeName] != nullptr )
		{
			g_theConsole->ErrorString( "The anim definition for \"%s\" at line %i has already been created", nextAttributeName, element.GetLineNum() );
			nextAttribute = nextAttribute->Next();
			continue;
		}

		SpriteAnimDefinition* newAnim = CreateAnimDefinition( element, nextAttributeName );
		if( newAnim != nullptr )
		{
			animStateMap[nextAttributeName] = newAnim;
		}
		nextAttribute = nextAttribute->Next();
	}

	m_animations[animStateName] = new SpriteAnimSet( animStateMap );
	return true;
}


//---------------------------------------------------------------------------------------------------------
SpriteAnimDefinition* EntityDef::CreateAnimDefinition( XmlElement const& element, char const* attributeName )
{
	if( !IsSupportedAnimDefType( attributeName ) )
	{
		g_theConsole->ErrorString( "\"%s\" at line %i is an unsuported anim def type", attributeName, element.GetLineNum() );
		return nullptr;
	}
	else
	{
		Strings spriteNumsAsString;
		spriteNumsAsString = ParseXmlAttribute( element, attributeName, spriteNumsAsString );

		size_t numSprites = spriteNumsAsString.size();
		if( numSprites == 0 )
		{
			g_theConsole->ErrorString( "Cannot create \"%s\" sprite anim def with 0 sprites at line %i", attributeName, element.GetLineNum() );
			return nullptr;
		}

		int firstSpriteIndex = atoi( spriteNumsAsString[0].c_str() );
		int lastSpriteIndex = atoi( spriteNumsAsString[numSprites - 1].c_str() );

		if( firstSpriteIndex < 0 )
		{
			g_theConsole->ErrorString( "First Sprite index for sprite anim def \"%s\" at line %i is out of range" );
			return nullptr;
		}
		else if( lastSpriteIndex > m_spriteSheet->GetNumSprite() - 1 )
		{
			g_theConsole->ErrorString( "First Sprite index for sprite anim def \"%s\" at line %i is out of range" );
			return nullptr;
		}

		return new SpriteAnimDefinition( *m_spriteSheet, firstSpriteIndex, lastSpriteIndex, 1.f );
	}
}


//---------------------------------------------------------------------------------------------------------
bool EntityDef::IsSupportedAnimDefType( std::string const& animDefType )
{
	if( animDefType == "front"		||
		animDefType == "back"		||
		animDefType == "left"		||
		animDefType == "right"		||
		animDefType == "backRight"	||
		animDefType == "backLeft"	||
		animDefType == "frontRight"	||
		animDefType == "frontLeft"		)
	{
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
STATIC void EntityDef::CreateEntityDefsFromXML( char const* filepath )
{
	g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, "Loading Entity Definitions from %s...", filepath );

	std::string const elementNameToLoad		= "EntityTypes";
	std::string const entityNodeName		= "Entity";
	std::string const actorNodeName			= "Actor";
	std::string const projectileNodeName	= "Projectile";
	std::string const portalNodeName		= "Portal";

	XmlDocument entityDefsTypesFile = new XmlDocument();
	entityDefsTypesFile.LoadFile( filepath );
	if( entityDefsTypesFile.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->ErrorString( "Failed to load %s for generating Entity Definitions", filepath );
		g_theConsole->ErrorString( "  Error: %s", entityDefsTypesFile.ErrorName() );
		g_theConsole->ErrorString( "  Error Line #%i", entityDefsTypesFile.ErrorLineNum() );
		return;
	}

	const XmlElement* rootElement = entityDefsTypesFile.RootElement();
	if( !IsStringEqual( rootElement->Name(), elementNameToLoad.c_str() ) )
	{
		g_theConsole->ErrorString( "Tried to load element %s - should be %s", rootElement->Name(), elementNameToLoad.c_str() );
	}

	const XmlElement* nextChildElement = rootElement->FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string nextChildName = nextChildElement->Name();
		EntityDef* createdEntityDef = nullptr;
		if( nextChildName ==  "Entity" )
		{
			createdEntityDef = new EntityDef( *nextChildElement, ENTITY_TYPE_ENTITY );
		}
		else if( nextChildName == "Actor" )
		{
			createdEntityDef = new EntityDef( *nextChildElement, ENTITY_TYPE_ACTOR );
		}
		else if( nextChildName == "Projectile" )
		{
			createdEntityDef = new EntityDef( *nextChildElement, ENTITY_TYPE_PROJECTILE );
		}
		else if( nextChildName == "Portal" )
		{
			createdEntityDef = new EntityDef( *nextChildElement, ENTITY_TYPE_PORTAL );
		}
		else
		{
			g_theConsole->ErrorString(							"Unrecognized Node Type: %s", nextChildElement->Name() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"Supported Node Types:" );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", entityNodeName.c_str() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", actorNodeName.c_str() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", projectileNodeName.c_str() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  %s", portalNodeName.c_str() );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC SpriteSheet* EntityDef::GetOrCreateEntitySpriteSheet( char const* filepath, IntVec2 const& layout )
{
	SpriteSheet* spriteSheet = s_spriteSheets[ filepath ];
	if( spriteSheet == nullptr )
	{
		Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile( filepath );
		spriteSheet = new SpriteSheet( *spriteSheetTexture, layout );
		s_spriteSheets[ filepath ] = spriteSheet;
	}
	return spriteSheet;
}


//---------------------------------------------------------------------------------------------------------
STATIC EntityDef* EntityDef::GetEntityDefByName( std::string const& entityName )
{
	return s_entityDefs[entityName];
}


//---------------------------------------------------------------------------------------------------------
STATIC EntityType EntityDef::GetEntityTypeFromString( std::string entityTypeAsString )
{
	EntityType entityType = INVALID_ENTITY_TYPE;

	if( entityTypeAsString == "Entity" )			entityType = ENTITY_TYPE_ENTITY;
	else if( entityTypeAsString == "Actor" )		entityType = ENTITY_TYPE_ACTOR;
	else if( entityTypeAsString == "Projectile" )	entityType = ENTITY_TYPE_PROJECTILE;
	else if( entityTypeAsString == "Portal" )		entityType = ENTITY_TYPE_PORTAL;

	return entityType;
}

