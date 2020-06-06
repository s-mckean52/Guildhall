#include "Game/Ability_Blink.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Blink::Blink( Game* theGame, Entity* owner )
	: Ability( theGame, owner, 5.f, "Blink" )
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

	float maxDistanceToMove = 3.5f;
	Vec2 cursorPosition = m_theGame->GetCursorPosition();
	Vec2 displacementTowardCursor = cursorPosition - ownerCurrentPosition;
	Vec2 directionTowardCursor = displacementTowardCursor.GetNormalized();

	float distanceToCursor = displacementTowardCursor.GetLength();
	float distanceToMove = Minf( maxDistanceToMove, distanceToCursor );

	Vec2 positionToBlinkTo = directionTowardCursor * distanceToMove;
	positionToBlinkTo += ownerCurrentPosition;
	m_owner->SetCurrentPosition( positionToBlinkTo );
}

