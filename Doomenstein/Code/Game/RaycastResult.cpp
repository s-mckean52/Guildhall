#include "Game/RaycastResult.hpp"


//---------------------------------------------------------------------------------------------------------
RaycastResult::RaycastResult(	Vec3 const& setStartPosition,
								Vec3 const& setForwardNormal,
								float		setMaxDistance,
								Vec3		setImpactPosition,
								bool		setDidImpact,		
								float		setImpactDistance,
								Vec3		setImpactNormal,
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