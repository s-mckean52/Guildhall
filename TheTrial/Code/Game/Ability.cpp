#include "Game/Ability.hpp"
#include "Game/Blink.hpp"
#include "Game/Buff.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/DevConsole.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
STATIC std::map<std::string, Ability*> Ability::s_abilities;


//---------------------------------------------------------------------------------------------------------
Ability::Ability( Game* theGame, Actor* owner, double cooldownSeconds, std::string const& name )
{
	m_theGame = theGame;
	m_owner = owner;
	m_name = name;
	m_baseCooldownSeconds = cooldownSeconds;

	m_cooldownTimer.SetSeconds( theGame->GetGameClock(), 0.0 );
}


//---------------------------------------------------------------------------------------------------------
Ability::Ability( XmlElement const& xmlElement )
{
	m_name					= ParseXmlAttribute( xmlElement, "name", m_name );
	m_baseCooldownSeconds	= ParseXmlAttribute( xmlElement, "cooldown", m_baseCooldownSeconds );
}


//---------------------------------------------------------------------------------------------------------
Ability::Ability( Ability const& copyFrom )
	: m_theGame( copyFrom.m_theGame )
	, m_owner( copyFrom.m_owner )
	, m_type( copyFrom.m_type )
	, m_name( copyFrom.m_name )
	, m_baseCooldownSeconds( copyFrom.m_baseCooldownSeconds )
	, m_cooldownTimer( m_cooldownTimer )
{
}


//---------------------------------------------------------------------------------------------------------
Ability::~Ability()
{

}


//---------------------------------------------------------------------------------------------------------
void Ability::Update()
{
}


//---------------------------------------------------------------------------------------------------------
void Ability::Render( Vec2 const& minPosition ) const
{
	Vec2 abilityRectDimensions = Vec2( ABILITY_UI_WIDTH, ABILITY_UI_HEIGHT );
	Vec2 abilityRectMaxPos = minPosition + abilityRectDimensions;
	AABB2 abilityRect = AABB2( minPosition, abilityRectMaxPos );


	std::vector<Vertex_PCU> abilityVerts;
	g_theRenderer->BindMaterial( nullptr );
	AppendVertsForAABB2D( abilityVerts, abilityRect, Rgba8::WHITE );
	g_theRenderer->DrawVertexArray( abilityVerts );

	std::string abilityStatusString = "Ready";
	if( IsOnCooldown() )
	{
		double elapsedTime = m_cooldownTimer.GetSecondsRemaining();
		abilityStatusString = Stringf( "%.1f", elapsedTime );
	}
	
	std::vector<Vertex_PCU> abilityTextVerts;
	g_devConsoleFont->AddVertsForTextInBox2D( abilityTextVerts, abilityRect, 0.1f, abilityStatusString, Rgba8::BLUE );
	g_devConsoleFont->AddVertsForTextInBox2D( abilityTextVerts, abilityRect, 0.1f, m_name, Rgba8::BLUE, 1.f, ALIGN_TOP_CENTERED );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->DrawVertexArray( abilityTextVerts );
}


//---------------------------------------------------------------------------------------------------------
void Ability::Use()
{
	double cooldownSeconds = m_baseCooldownSeconds;
	m_cooldownTimer.SetSeconds( m_theGame->GetGameClock(), cooldownSeconds );
}


//---------------------------------------------------------------------------------------------------------
bool Ability::IsOnCooldown() const
{
	return !m_cooldownTimer.HasElapsed();
}


//---------------------------------------------------------------------------------------------------------
void Ability::SetGame( Game* theGame )
{
	m_theGame = theGame;
	m_cooldownTimer.SetSeconds( theGame->GetGameClock(), 0.0 );
}


//---------------------------------------------------------------------------------------------------------
void Ability::SetOwner( Actor* owner )
{
	m_owner = owner;
}

//---------------------------------------------------------------------------------------------------------
STATIC void Ability::CreateAbilitiesFromXML( char const* filepath )
{
	XmlDocument abilityFile = new XmlDocument();
	abilityFile.LoadFile( filepath );
	GUARANTEE_OR_DIE( abilityFile.ErrorID() == 0, "Failed to load AbilityDefinitions.xml" );

	XmlElement const* rootElement = abilityFile.RootElement();
	XmlElement const* nextChildElement = rootElement->FirstChildElement();

	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		Ability* newAbility = nullptr;
		std::string abilityTypeAsString = nextChildElement->Name();
		AbilityType abilityTypeToCreate = GetAbilityTypeFromString( abilityTypeAsString );
		switch( abilityTypeToCreate )
		{
		case ABILITY_TYPE_BLINK:		newAbility = new Blink( *nextChildElement ); break;
		case ABILITY_TYPE_BUFF:			newAbility = new Buff( *nextChildElement ); break;
		case ABILITY_TYPE_SKILLSHOT:	
		case ABILITY_TYPE_TARGET:
		case INVALID_ABILITY_TYPE:
		default:
			g_theConsole->ErrorString( "Unsupported ability type: %s", abilityTypeAsString.c_str() );
			break;
		}
		
		if( newAbility != nullptr )
		{
			s_abilities.insert( { newAbility->m_name, newAbility } );
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
AbilityType Ability::GetAbilityTypeFromString( std::string const& abilityTypeAsString )
{
	if		( abilityTypeAsString == "Blink" )		{ return ABILITY_TYPE_BLINK; }
	else if	( abilityTypeAsString == "Projectile" )	{ return ABILITY_TYPE_SKILLSHOT; }
	else if	( abilityTypeAsString == "Target" )		{ return ABILITY_TYPE_TARGET; }
	else if	( abilityTypeAsString == "Buff" )		{ return ABILITY_TYPE_BUFF; }
	else											{ return INVALID_ABILITY_TYPE; };
}


//---------------------------------------------------------------------------------------------------------
STATIC std::string Ability::GetAbilityTypeAsString( AbilityType abilityType )
{
	switch( abilityType )
	{
	case ABILITY_TYPE_BLINK:		return "Blink";			break;
	case ABILITY_TYPE_SKILLSHOT:	return "Projectile";	break;
	case ABILITY_TYPE_TARGET:		return "Target";		break;
	case ABILITY_TYPE_BUFF:			return "Buff";			break;

	default:
		ERROR_AND_DIE( "Tried to get the string of an unsupported ability type" );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
STATIC Ability* Ability::GetNewAbilityByName( std::string const& abilityName )
{
	auto abilityIter = s_abilities.find( abilityName );
	if( abilityIter != s_abilities.end() )
	{
		Ability* abilityDefinition = abilityIter->second;
		switch( abilityDefinition->m_type )
		{
		case ABILITY_TYPE_BLINK:		return new Blink( *(Blink*)abilityDefinition ); break;
		case ABILITY_TYPE_BUFF:			return new Buff( *(Buff*)abilityDefinition ); break;
		case ABILITY_TYPE_SKILLSHOT:	
		case ABILITY_TYPE_TARGET:
		default:
			break;
		}

	}
	return nullptr;
}