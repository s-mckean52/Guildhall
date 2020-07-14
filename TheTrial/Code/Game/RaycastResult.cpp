#include "Game/RaycastResult.hpp"


//---------------------------------------------------------------------------------------------------------
RaycastResult::RaycastResult(	Vec2 const& setStartPosition,
								Vec2 const& setForwardNormal,
								float		setMaxDistance,
								Vec2		setImpactPosition,
								bool		setDidImpact,		
								float		setImpactDistance,
								Vec2		setImpactNormal,
								Entity*		setImpactEntity		)
	: startPosition( setStartPosition )
	, forwardNormal( setForwardNormal )
	, impactPosition( setImpactPosition )
	, maxDistance( setMaxDistance )
	, didImpact( setDidImpact )
	, impactDistance( setImpactDistance )
	, impactNormal( setImpactNormal )
	, impactEntity( setImpactEntity)
{
}