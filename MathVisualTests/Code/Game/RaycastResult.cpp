#include "Game/RaycastResult.hpp"


//---------------------------------------------------------------------------------------------------------
RaycastResult::RaycastResult( Vec2 impactPos, bool didImpact, float impactDist, float impactFraction )
	: m_impactPos( impactPos )
	, m_didImpact( didImpact )
	, m_impactDist( impactDist)
	, m_impactFraction( impactFraction )
{
}