#include "Game/Item.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
STATIC std::map<std::string, SpriteSheet*>	Item::s_itemSpriteSheets;
STATIC std::map<std::string, Item>			Item::s_itemDefinitions;


//---------------------------------------------------------------------------------------------------------
Item::Item( XmlElement const& element )
	: Entity( g_theGame )
{
	m_name = ParseXmlAttribute( element, "name", "Default" );
	m_tint = ParseXmlAttribute( element, "tint", Rgba8::WHITE );

	std::string spriteSheetName = ParseXmlAttribute( element, "sheet", "Robes" );
	m_spriteIndex				= ParseXmlAttribute( element, "spriteIndex", 0 );
	m_spriteSheet				= s_itemSpriteSheets[ spriteSheetName ];

	m_spriteSheet->GetSpriteUVs( m_spriteUVBounds.mins, m_spriteUVBounds.maxes, m_spriteIndex );

	XmlElement const* nextChildElement = element.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string statTypeAsString = ParseXmlAttribute( *nextChildElement, "stat", "" );
		ActorStat statType = GetStatTypeFromString( statTypeAsString );
		float amountToMod = ParseXmlAttribute( *nextChildElement, "amount", 0.f );

		m_statMods.push_back( StatMod( statType, amountToMod ) );

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
Item::Item( Item const& copyFrom )
	: Entity( g_theGame )
{
	m_name = copyFrom.m_name;
	m_tint = copyFrom.m_tint;
	m_statMods = copyFrom.m_statMods;
	m_spriteSheet = copyFrom.m_spriteSheet;
	m_spriteIndex = copyFrom.m_spriteIndex;
	m_spriteUVBounds = copyFrom.m_spriteUVBounds;
}


//---------------------------------------------------------------------------------------------------------
Item::Item()
	: Entity( g_theGame )
{
}

//---------------------------------------------------------------------------------------------------------
Item::~Item()
{
}


//---------------------------------------------------------------------------------------------------------
void Item::Render() const
{
	std::vector<Vertex_PCU> itemVerts;
	AppendVertsForAABB2D( itemVerts, m_renderBounds, m_tint, m_spriteUVBounds.mins, m_spriteUVBounds.maxes );
	TransformVertexArray( itemVerts, 1.f, 0.f, m_currentPosition );

	g_theRenderer->BindTexture( &m_spriteSheet->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( itemVerts );

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Item::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	Player* thePlayer = m_theGame->GetPlayer();
	if( !IsDead() && DoDiscsOverlap( thePlayer->GetCurrentPosition(), thePlayer->GetPhysicsRadius(), m_currentPosition, m_physicsRadius ) )
	{
		thePlayer->PickUpItem( *this );
		m_isDead = true;
	}
}


//---------------------------------------------------------------------------------------------------------
StatMod const& Item::GetStatModAtIndex( int index ) const
{
	return m_statMods[ index ];
}

//---------------------------------------------------------------------------------------------------------
STATIC void Item::CreateItemsFromXML( const char* filepath )
{
	XmlDocument abilityFile = new XmlDocument();
	abilityFile.LoadFile( filepath );
	GUARANTEE_OR_DIE( abilityFile.ErrorID() == 0, "Failed to load ItemDefinitions.xml" );

	XmlElement const* rootElement = abilityFile.RootElement();
	XmlElement const* nextChildElement = rootElement->FirstChildElement();

	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string nodeName = nextChildElement->Name();
		if( nodeName == "SpriteSheet")	{ CreateSpriteSheet( *nextChildElement ); }
		else if( nodeName == "Item" )	{ CreateItem( *nextChildElement ); }

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC Item const& Item::GetItemDefByName( std::string const& itemName )
{
	auto itemIter = s_itemDefinitions.find( itemName );
	if( itemIter == s_itemDefinitions.end() )
	{
		ERROR_AND_DIE( Stringf( "Item \"%s\" does not exist", itemName.c_str() ) )
	}
	return itemIter->second;
}

//---------------------------------------------------------------------------------------------------------
STATIC ActorStat Item::GetStatTypeFromString( std::string const& statTypeAsString )
{
	if( statTypeAsString == "CritMultiplier" )		{ return STAT_CRIT_MULTIPLIER; }
	else if( statTypeAsString == "CritChance" )		{ return STAT_CRIT_CHANCE; }
	else if( statTypeAsString == "MoveSpeed" )		{ return STAT_MOVEMENT_SPEED; }
	else if( statTypeAsString == "AttackSpeed" )	{ return STAT_ATTACK_SPEED; }
	else if( statTypeAsString == "AttackDamage" )	{ return STAT_ATTACK_DAMAGE; }
	else
	{
		ERROR_AND_DIE( "Read an unsupported string for actor stat" );
		//return STAT_ATTACK_DAMAGE;
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC void Item::CreateSpriteSheet( XmlElement const& element )
{
	std::string name					= ParseXmlAttribute( element, "name", "TestTerrain" );
	IntVec2		spriteSheetDimensions	= ParseXmlAttribute( element, "layout", IntVec2( 8, 8 ) );

	XmlElement const&	diffuseElement	= *element.FirstChildElement( "Diffuse" );
	std::string			diffuseFilepath = ParseXmlAttribute( diffuseElement, "image", "Data/Images/Terrain_8x8.png" );

	Texture* diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile( diffuseFilepath.c_str() );
	s_itemSpriteSheets[ name ] = new SpriteSheet( *diffuseTexture, spriteSheetDimensions );
}


//---------------------------------------------------------------------------------------------------------
STATIC void Item::CreateItem( XmlElement const& element )
{
	std::string itemName			= ParseXmlAttribute( element, "name", "Default" );
	s_itemDefinitions[ itemName ]	= Item( element );
}


//---------------------------------------------------------------------------------------------------------
STATIC Item* Item::GetRandomItem()
{
	auto itemDefIter = s_itemDefinitions.begin();
	int randomItemIndex = g_RNG->RollRandomIntInRange( 0, static_cast<int>( s_itemDefinitions.size() ) - 1 );

	std::advance( itemDefIter, randomItemIndex );
	Item* randomItem = new Item( itemDefIter->second );
	return randomItem;
}
