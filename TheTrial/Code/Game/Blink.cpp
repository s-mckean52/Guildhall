#include "Game/Blink.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Blink::Blink( Game* theGame, Actor* owner, float range )
	: Ability( theGame, owner, 5.f, "Default_Blink" )
{
	m_type = ABILITY_TYPE_BLINK;
	m_range = range;
}


//---------------------------------------------------------------------------------------------------------
Blink::Blink( XmlElement const& xmlElement )
	: Ability( xmlElement )
{
	XmlElement const& valueElement = *xmlElement.FirstChildElement( "values" );
	
	m_range	= ParseXmlAttribute( valueElement, "range", m_range );
	m_type	= ABILITY_TYPE_BLINK;
}


//---------------------------------------------------------------------------------------------------------
Blink::Blink( Blink const& copyFrom )
	: Ability( copyFrom )
	, m_range( copyFrom.m_range )
{
}


//---------------------------------------------------------------------------------------------------------
Blink::~Blink()
{
}


//---------------------------------------------------------------------------------------------------------
void Blink::Use()
{
	if( IsOnCooldown() )
		return;

	Ability::Use();

	Vec2 ownerCurrentPosition = m_owner->GetCurrentPosition();

	Vec2 cursorPosition = m_theGame->GetCursorPosition();
	Vec2 displacementTowardCursor = cursorPosition - ownerCurrentPosition;
	Vec2 directionTowardCursor = displacementTowardCursor.GetNormalized();

	float distanceToCursor = displacementTowardCursor.GetLength();
	float distanceToMove = Minf( m_range, distanceToCursor );

	Vec2 positionToBlinkTo = directionTowardCursor * distanceToMove;
	positionToBlinkTo += ownerCurrentPosition;
	m_owner->SetCurrentPosition( positionToBlinkTo );
	m_owner->SetIsMoving( false );
}

