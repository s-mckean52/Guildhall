#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Polygon2D.hpp"
#include <math.h>


//---------------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians( float degrees )
{
	return degrees * degreesToRadiansRatio;
}


//---------------------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees( float radians )
{
	return radians * radiansToDegreesRatio;
}


//---------------------------------------------------------------------------------------------------------
float CosDegrees( float degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}


//---------------------------------------------------------------------------------------------------------
float SinDegrees( float degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}


//---------------------------------------------------------------------------------------------------------
float TanDegrees( float degrees )
{
	return tanf( ConvertDegreesToRadians( degrees ) );
}


//---------------------------------------------------------------------------------------------------------
float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}


//---------------------------------------------------------------------------------------------------------
float GetDistance2D( const Vec2& positionA, const Vec2& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;

	return sqrtf( (deltaX * deltaX) + (deltaY * deltaY) );
}


//---------------------------------------------------------------------------------------------------------
float GetDistanceSquared2D( const Vec2& positionA, const Vec2& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;

	return (deltaX * deltaX) + (deltaY * deltaY);
}


//---------------------------------------------------------------------------------------------------------
float GetAngleToPointDegrees( const Vec2& startPosition, const Vec2& positionToRotateTo )
{
	Vec2 localSpaceRotateToPosition = positionToRotateTo - startPosition;

	return localSpaceRotateToPosition.GetAngleDegrees();
}


//---------------------------------------------------------------------------------------------------------
float GetDistance3D( const Vec3& positionA, const Vec3& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	float deltaZ = positionA.z - positionB.z;

	return sqrtf( ( deltaX * deltaX ) + ( deltaY * deltaY ) + ( deltaZ * deltaZ ) );
}


//---------------------------------------------------------------------------------------------------------
float GetDistanceXY3D( const Vec3& positionA, const Vec3& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;

	return sqrtf( (deltaX * deltaX) + (deltaY * deltaY) );
}


//---------------------------------------------------------------------------------------------------------
float GetDistanceSquared3D( const Vec3& positionA, const Vec3& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	float deltaZ = positionA.z - positionB.z;

	return (deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ);
}


//---------------------------------------------------------------------------------------------------------
float GetDistanceXYSquared3D( const Vec3& positionA, const Vec3& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;

	return (deltaX * deltaX) + (deltaY * deltaY);
}


//---------------------------------------------------------------------------------------------------------
float Lerp( float rangeBegin, float rangeEnd, float fractionOfRangeEnd )
{
	return rangeBegin + fractionOfRangeEnd * ( rangeEnd - rangeBegin );
}


//---------------------------------------------------------------------------------------------------------
float RangeMapFloat( float fromBegin, float fromEnd, float toBegin, float toEnd, float fromValue )
{
	float fromDisplacement = fromValue - fromBegin;
	float fromRange = fromEnd - fromBegin;
	float fractionOfRange = fromDisplacement / fromRange;
	return Lerp( toBegin, toEnd, fractionOfRange );
}


//---------------------------------------------------------------------------------------------------------
int GetClamp( int valueToClamp, int lowerLimit, int upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		return lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		return upperLimit;
	}
	return valueToClamp;
}


//---------------------------------------------------------------------------------------------------------
float GetClamp( float valueToClamp, float lowerLimit, float upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		return lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		return upperLimit;
	}
	return valueToClamp;
}


//---------------------------------------------------------------------------------------------------------
double GetClamp( double valueToClamp, double lowerLimit, double upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		return lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		return upperLimit;
	}
	return valueToClamp;
}


//---------------------------------------------------------------------------------------------------------
void Clamp( int& valueToClamp, int lowerLimit, int upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		valueToClamp = lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		valueToClamp = upperLimit;
	}
}


//---------------------------------------------------------------------------------------------------------
void Clamp( float& valueToClamp, float lowerLimit, float upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		valueToClamp = lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		valueToClamp = upperLimit;
	}
}


//---------------------------------------------------------------------------------------------------------
void Clamp( double& valueToClamp, double lowerLimit, double upperLimit )
{
	if( valueToClamp < lowerLimit )
	{
		valueToClamp = lowerLimit;
	}
	else if( valueToClamp > upperLimit )
	{
		valueToClamp = upperLimit;
	}
}


//---------------------------------------------------------------------------------------------------------
void ClampZeroToOne( float& valueToClamp )
{
	Clamp( valueToClamp, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
bool ApproximatelyEqual( float valueToCheck, float valueToCheckAgainst, float percisionValue )
{
	float diff = valueToCheckAgainst - valueToCheck;
	if( diff > percisionValue || diff < -percisionValue )
	{
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
float Signf( float value )
{
	return ( value >= 0.f ) ? 1.0f : -1.0f;
}


//---------------------------------------------------------------------------------------------------------
float GetClampZeroToOne( float valueToClamp )
{
	return GetClamp( valueToClamp, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
bool DoDiscsOverlap( const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB )
{
	float distance = GetDistance2D( centerA, centerB );
	distance -= radiusA + radiusB;

	if( distance < 0 )
	{
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
bool DoSpheresOverlap( const Vec3& centerA, float radiusA, const Vec3& centerB, float radiusB )
{
	float distance = GetDistance3D( centerA, centerB );
	distance -= radiusA + radiusB;

	if( distance < 0 )
	{
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& position, float scale, float rotationDegrees, Vec2 translation )
{
	Vec2 transformedPos = position; 
	transformedPos *= scale;
	transformedPos.RotateDegrees( rotationDegrees );
	transformedPos += translation;
	return transformedPos;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& position, const Vec2& basisI, const Vec2& basisJ, const Vec2& translation )
{
	Vec2 newPosition = position.x * basisI;
	newPosition += position.y * basisJ;
	
	return newPosition + translation;
}


//---------------------------------------------------------------------------------------------------------
const Vec3 TransformPosition3DXY( const Vec3& position, float scaleXY, float zRotatioinDegrees, Vec2 translationXY )
{
	Vec2 transformedPosXY( position.x, position.y );
	transformedPosXY *= scaleXY;
	transformedPosXY.RotateDegrees( zRotatioinDegrees );
	transformedPosXY += translationXY;
	return Vec3( transformedPosXY.x, transformedPosXY.y, position.z );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 TransformPosition3DXY( const Vec3& position, const Vec2& basisI, const Vec2& basisJ, const Vec2& translationXY )
{
	Vec2 newPosition = position.x * basisI;
	newPosition += position.y * basisJ;
	newPosition += translationXY;

	return Vec3( newPosition.x, newPosition.y, position.z );
}


//---------------------------------------------------------------------------------------------------------
void TransformVertexArray( int numVerticies, Vertex_PCU* vertexArray, float scale, float rotation, const Vec2& translation )
{
	for( int verts = 0; verts < numVerticies; ++verts )
	{
		vertexArray[verts].m_position = TransformPosition3DXY( vertexArray[verts].m_position, scale, rotation, translation );
	}
}


//---------------------------------------------------------------------------------------------------------
void TranslateVertexArray( int numVerticies, Vertex_PCU* vertexArray, const Vec3& translation )
{
	for( int vertIndex = 0; vertIndex < numVerticies; ++vertIndex )
	{
		vertexArray[ vertIndex ].m_position += translation;
	}
}


//---------------------------------------------------------------------------------------------------------
void TransformVertexArray( std::vector<Vertex_PCU>& vertexArray, float scale, float rotation, const Vec2& translation )
{
	TransformVertexArray( static_cast<int>( vertexArray.size() ), &vertexArray[ 0 ], scale, rotation, translation );
}


//---------------------------------------------------------------------------------------------------------
void TranslateVertexArray( std::vector<Vertex_PCU>& vertexArray, const Vec3& translation )
{
	TranslateVertexArray( static_cast<int>( vertexArray.size() ), &vertexArray[ 0 ], translation );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnAABB2D( const Vec2& pointToCheck, const AABB2& box )
{
	return box.GetNearestPoint( pointToCheck );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnDisc2D( const Vec2& pointToCheck, const Vec2& discCenterPosition, float discRadius )
{
	Vec2 displacementVector = pointToCheck - discCenterPosition;
	displacementVector.ClampLength( discRadius );

	return discCenterPosition + displacementVector;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnInfiniteLine2D( const Vec2& refPos, const Vec2& somePointOnLine, const Vec2& anotherPointOnLine )
{
	Vec2 displacementToPointFromStart = refPos - somePointOnLine;
	Vec2 lineDisplacement = anotherPointOnLine - somePointOnLine;

	return somePointOnLine + GetProjectedOnto2D( displacementToPointFromStart, lineDisplacement );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnLineSegment2D( const Vec2& refPos, const Vec2& start, const Vec2& end )
{
	Vec2 displacementToPointFromStart = refPos - start;
	Vec2 lineDisplacement = end - start;

	Vec2 projectedVector = GetProjectedOnto2D( displacementToPointFromStart, lineDisplacement );
	float projectedVectorDot = DotProduct2D( projectedVector, lineDisplacement );

	if( projectedVectorDot < 0.f )
	{
		return start;
	}
	else if( projectedVector.GetLengthSquared() > lineDisplacement.GetLengthSquared() )
	{
		return end;
	}
	else
	{
		return start + projectedVector;
	}
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnCapsule2D( const Vec2& refPos, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	if( IsPointInsideCapsule2D( refPos, capsuleMidStart, capsuleMidEnd, capsuleRadius ) ) return refPos;

	Vec2 nearestPointOnCapsuleMidLine = GetNearestPointOnLineSegment2D( refPos, capsuleMidStart, capsuleMidEnd );
	Vec2 displacementToPoint = refPos - nearestPointOnCapsuleMidLine;

	return nearestPointOnCapsuleMidLine + displacementToPoint.GetClamped( capsuleRadius );
}


//---------------------------------------------------------------------------------------------------------
int RoundDownToInt( float valueToRound )
{
	return static_cast<int>( floorf( valueToRound ) );
}


//---------------------------------------------------------------------------------------------------------
bool DoAABB2sOverlap( const AABB2& boxA, const AABB2& boxB )
{
	if( boxA.maxes.x < boxB.mins.x )
	{
		return false;
	}
	else if( boxB.maxes.x < boxA.mins.x )
	{
		return false;
	}
	else if( boxA.maxes.y < boxB.mins.y )
	{
		return false;
	}
	else if( boxB.maxes.y < boxA.mins.y )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DoDiscAndAABB2Overlap( const AABB2& box, const Vec2& discCenterPosition, float discRadius )
{
	Vec2 nearestPointToDiscCenterOnBox = GetNearestPointOnAABB2D( discCenterPosition, box );
	float distanceToNearestPoint = ( nearestPointToDiscCenterOnBox - discCenterPosition ).GetLength();

	if( distanceToNearestPoint < discRadius )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool DoOBBAndOBBOverlap2D( const OBB2& boxA, const OBB2& boxB )
{
	Vec2 boxAWorldCornerPoints[ 4 ] = {};
	boxA.GetCornerPositions( boxAWorldCornerPoints );

	Vec2 boxBWorldCornerPoints[ 4 ] = {};
	boxB.GetCornerPositions( boxBWorldCornerPoints );

	Vec2 boxAIBasisNormalized = boxA.GetIBasisNormal();
	FloatRange boxAProjectedOnAIBasis = GetRangeOnProjectedAxis( 4, boxAWorldCornerPoints, Vec2(), boxAIBasisNormalized );
	FloatRange boxBProjectedOnAIBasis = GetRangeOnProjectedAxis( 4, boxBWorldCornerPoints, Vec2(), boxAIBasisNormalized );
	if( !boxAProjectedOnAIBasis.DoesOverlap( boxBProjectedOnAIBasis ) )
	{
		return false;
	}

	Vec2 boxAJBasisNormalized = boxA.GetJBasisNormal();
	FloatRange boxAProjectedOnAJBasis = GetRangeOnProjectedAxis( 4, boxAWorldCornerPoints, Vec2(), boxAJBasisNormalized );
	FloatRange boxBProjectedOnAJBasis = GetRangeOnProjectedAxis( 4, boxBWorldCornerPoints, Vec2(), boxAJBasisNormalized );
	if( !boxAProjectedOnAJBasis.DoesOverlap( boxBProjectedOnAJBasis ) )
	{
		return false;
	}

	Vec2 boxBIBasisNormalized = boxB.GetIBasisNormal();
	FloatRange boxAProjectedOnBIBasis = GetRangeOnProjectedAxis( 4, boxAWorldCornerPoints, Vec2(), boxBIBasisNormalized );
	FloatRange boxBProjectedOnBIBasis = GetRangeOnProjectedAxis( 4, boxBWorldCornerPoints, Vec2(), boxBIBasisNormalized );
	if( !boxAProjectedOnBIBasis.DoesOverlap( boxBProjectedOnBIBasis ) )
	{
		return false;
	}

	Vec2 boxBJBasisNormalized = boxB.GetJBasisNormal();
	FloatRange boxAProjectedOnBJBasis = GetRangeOnProjectedAxis( 4, boxAWorldCornerPoints, Vec2(), boxBJBasisNormalized );
	FloatRange boxBProjectedOnBJBasis = GetRangeOnProjectedAxis( 4, boxBWorldCornerPoints, Vec2(), boxBJBasisNormalized );
	if( !boxAProjectedOnBJBasis.DoesOverlap( boxBProjectedOnBJBasis ) )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DoOBBAndAABBOverlap2D( const OBB2& obb, const AABB2& aabb )
{
	OBB2 aabb2AsOBB2 = OBB2( aabb );

	return DoOBBAndOBBOverlap2D( obb, aabb2AsOBB2 );
}


//---------------------------------------------------------------------------------------------------------
bool DoOBBAndLineSegmentOverlap2D( const OBB2& obb, const Vec2& lineStart, const Vec2& lineEnd )
{
	Vec2 obbWorldCornerPoints[ 4 ] = {};
	obb.GetCornerPositions( obbWorldCornerPoints );

	Vec2 lineStartAndEndAsArray[ 2 ] = { lineStart, lineEnd };

	Vec2 obbIBasisNormalized = obb.GetIBasisNormal();
	FloatRange obbProjectedOnOBBIBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbIBasisNormalized );
	FloatRange lineProjectedOnOBBIBasis = GetRangeOnProjectedAxis( 2, lineStartAndEndAsArray, Vec2(), obbIBasisNormalized );
	if( !obbProjectedOnOBBIBasis.DoesOverlap( lineProjectedOnOBBIBasis ) )
	{
		return false;
	}

	Vec2 obbJBasisNormalized = obb.GetJBasisNormal();
	FloatRange obbProjectedOnOBBJBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbJBasisNormalized );
	FloatRange lineProjectedOnOBBJBasis = GetRangeOnProjectedAxis( 2, lineStartAndEndAsArray, Vec2(), obbJBasisNormalized );
	if( !obbProjectedOnOBBJBasis.DoesOverlap( lineProjectedOnOBBJBasis ) )
	{
		return false;
	}

	Vec2 lineDisplacement = lineEnd - lineStart;
	Vec2 lineBasisNormalized = lineDisplacement.GetNormalized();
	FloatRange obbProjectedOnLineBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), lineBasisNormalized );
	FloatRange lineProjectedOnLineBasis = GetRangeOnProjectedAxis( 2, lineStartAndEndAsArray, Vec2(), lineBasisNormalized );
	if( !obbProjectedOnLineBasis.DoesOverlap( lineProjectedOnLineBasis ) )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DoOBBAndCapsuleOverlap2D( const OBB2& obb, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	Vec2 obbWorldCornerPoints[ 4 ] = {};
	obb.GetCornerPositions( obbWorldCornerPoints );

	Vec2 capsuleMidStartAndEndAsArray[ 2 ] = { capsuleMidStart, capsuleMidEnd };

	Vec2 obbIBasisNormalized = obb.GetIBasisNormal();
	FloatRange obbProjectedOnOBBIBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbIBasisNormalized );
	FloatRange capsuleProjectedOnOBBIBasis = GetRangeOnProjectedAxis( 2, capsuleMidStartAndEndAsArray, Vec2(), obbIBasisNormalized );
	capsuleProjectedOnOBBIBasis.minimum -= capsuleRadius;
	capsuleProjectedOnOBBIBasis.maximum += capsuleRadius;
	if( !obbProjectedOnOBBIBasis.DoesOverlap( capsuleProjectedOnOBBIBasis ) )
	{
		return false;
	}

	Vec2 obbJBasisNormalized = obb.GetJBasisNormal();
	FloatRange obbProjectedOnOBBJBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbJBasisNormalized );
	FloatRange capsuleProjectedOnOBBJBasis = GetRangeOnProjectedAxis( 2, capsuleMidStartAndEndAsArray, Vec2(), obbJBasisNormalized );
	capsuleProjectedOnOBBJBasis.minimum -= capsuleRadius;
	capsuleProjectedOnOBBJBasis.maximum += capsuleRadius;
	if( !obbProjectedOnOBBJBasis.DoesOverlap( capsuleProjectedOnOBBJBasis ) )
	{
		return false;
	}

	Vec2 capsuleMidDisplacement = capsuleMidEnd - capsuleMidStart;
	Vec2 capsuleIBasisNormalized = capsuleMidDisplacement.GetNormalized();
	FloatRange obbProjectedOnCapsuleIBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), capsuleIBasisNormalized );
	FloatRange capsuleProjectedOnCapsuleIBasis = GetRangeOnProjectedAxis( 2, capsuleMidStartAndEndAsArray, Vec2(), capsuleIBasisNormalized );
	capsuleProjectedOnCapsuleIBasis.minimum -= capsuleRadius;
	capsuleProjectedOnCapsuleIBasis.maximum += capsuleRadius;
	if( !obbProjectedOnCapsuleIBasis.DoesOverlap( capsuleProjectedOnCapsuleIBasis ) )
	{
		return false;
	}

	Vec2 capsuleJBasisNormalized = capsuleIBasisNormalized.GetRotated90Degrees();
	FloatRange obbProjectedOnCapsuleJBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), capsuleJBasisNormalized );
	FloatRange capsuleProjectedOnCapsuleJBasis = GetRangeOnProjectedAxis( 2, capsuleMidStartAndEndAsArray, Vec2(), capsuleJBasisNormalized );
	capsuleProjectedOnCapsuleJBasis.minimum -= capsuleRadius;
	capsuleProjectedOnCapsuleJBasis.maximum += capsuleRadius;
	if( !obbProjectedOnCapsuleJBasis.DoesOverlap( capsuleProjectedOnCapsuleJBasis ) )
	{
		return false;
	}

	Vec2 nearestPointOnCapsuleMid = GetNearestPointOnLineSegment2D( obb.m_center, capsuleMidStart, capsuleMidEnd );
	Vec2 nearestPointOnOBB = GetNearestPointOnOBB2D( nearestPointOnCapsuleMid, obb );
	Vec2 displacementAxis = nearestPointOnCapsuleMid - nearestPointOnOBB;
	Vec2 displacementAxisNormal = displacementAxis.GetNormalized();
	displacementAxisNormal.Rotate90Degrees();

	FloatRange obbProjectedOnDisplacementAxis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), displacementAxisNormal );
	FloatRange capsuleProjectedOnDisplacementAxis = GetRangeOnProjectedAxis( 2, capsuleMidStartAndEndAsArray, Vec2(), displacementAxisNormal );
	capsuleProjectedOnDisplacementAxis.minimum -= capsuleRadius;
	capsuleProjectedOnDisplacementAxis.maximum += capsuleRadius;
	if( !obbProjectedOnDisplacementAxis.DoesOverlap( capsuleProjectedOnDisplacementAxis ) )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DoOBBAndDiscOverlap2D( const OBB2& obb, const Vec2& discCenter, float discRadius )
{
	Vec2 obbWorldCornerPoints[ 4 ] = {};
	obb.GetCornerPositions( obbWorldCornerPoints );

	Vec2 obbIBasisNormalized = obb.GetIBasisNormal();
	FloatRange obbProjectedOnOBBIBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbIBasisNormalized );

	FloatRange discCenterProjectedOnOBBIBasis = FloatRange( GetProjectedLength2D( discCenter, obbIBasisNormalized.GetRotatedMinus90Degrees() ) );
	discCenterProjectedOnOBBIBasis.minimum -= discRadius;
	discCenterProjectedOnOBBIBasis.maximum += discRadius;

	if( !obbProjectedOnOBBIBasis.DoesOverlap( discCenterProjectedOnOBBIBasis ) )
	{
		return false;
	}

	Vec2 obbJBasisNormalized = obb.GetJBasisNormal();
	FloatRange obbProjectedOnOBBJBasis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), obbJBasisNormalized );

	FloatRange discCenterProjectedOnOBBJBasis = FloatRange( GetProjectedLength2D( discCenter, obbJBasisNormalized.GetRotatedMinus90Degrees() ) );
	discCenterProjectedOnOBBJBasis.minimum -= discRadius;
	discCenterProjectedOnOBBJBasis.maximum += discRadius;

	if( !obbProjectedOnOBBJBasis.DoesOverlap( discCenterProjectedOnOBBJBasis ) )
	{
		return false;
	}

	Vec2 displacementAxis = discCenter - obb.m_center;
	Vec2 displacementAxisNormal = displacementAxis.GetNormalized();
	displacementAxisNormal.Rotate90Degrees();
	FloatRange obbProjectedOnAxis = GetRangeOnProjectedAxis( 4, obbWorldCornerPoints, Vec2(), displacementAxisNormal );

	FloatRange discCenterProjectedOnAxis = FloatRange( GetProjectedLength2D( discCenter, displacementAxisNormal.GetRotatedMinus90Degrees() ) );
	discCenterProjectedOnAxis.minimum -= discRadius;
	discCenterProjectedOnAxis.maximum += discRadius;

	if( !obbProjectedOnAxis.DoesOverlap( discCenterProjectedOnAxis ) )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DoPolygonAndDiscOverlap( const Polygon2D& polygon, const Vec2& discCenter, float discRadius )
{
	Vec2 nearestPointOnPolygon = polygon.GetClosestPoint( discCenter );
	Vec2 displacement = nearestPointOnPolygon - discCenter;
	if( displacement.GetLength() < discRadius )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
float SmoothStart2( float t )
{
	return t * t;
}


//---------------------------------------------------------------------------------------------------------
float SmoothStart3( float t )
{
	return t * t * t;
}


//---------------------------------------------------------------------------------------------------------
float SmoothStart4( float t )
{
	return t * t * t * t;
}


//---------------------------------------------------------------------------------------------------------
float SmoothStart5( float t )
{
	return t * t * t * t * t;
}


//---------------------------------------------------------------------------------------------------------
float SmoothStop2( float t )
{
	float oneMinusT = 1 - t;
	return 1 - ( oneMinusT * oneMinusT );
}


//---------------------------------------------------------------------------------------------------------
float SmoothStop3( float t )
{
	float oneMinusT = 1 - t;
	return 1 - ( oneMinusT * oneMinusT * oneMinusT );
}


//---------------------------------------------------------------------------------------------------------
float SmoothStop4( float t )
{
	float oneMinusT = 1 - t;
	return 1 - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//---------------------------------------------------------------------------------------------------------
float SmoothStop5( float t )
{
	float oneMinusT = 1 - t;
	return 1 - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//---------------------------------------------------------------------------------------------------------
float SmoothStep3( float t )
{
	float s = 3 * ( t * t );
	float n = 2 * ( t * t * t );
	return s - n;
}


//---------------------------------------------------------------------------------------------------------
void PushDiscOutOfAABB2( Vec2& discCenterPosition, float discRadius, const AABB2& box )
{
	if( !DoDiscAndAABB2Overlap( box, discCenterPosition, discRadius ) ) return;

	Vec2 nearestPointOnBox = GetNearestPointOnAABB2D( discCenterPosition, box );
	Vec2 displacementToBox = nearestPointOnBox - discCenterPosition;

	float distanceToBox = displacementToBox.GetLength();
	displacementToBox.Normalize();

	float overlapDistance = distanceToBox - discRadius;
	
	discCenterPosition += displacementToBox * overlapDistance;
}


//---------------------------------------------------------------------------------------------------------
void PushDiscsOutOfEachOther2D( Vec2& discACenterPosition, float discARadius, Vec2& discBCenterPosition, float discBRadius )
{
	if( !DoDiscsOverlap( discACenterPosition, discARadius, discBCenterPosition, discBRadius) ) return;

	Vec2 displacementBetweenDiscs = discBCenterPosition - discACenterPosition;

	float distanceBetweenDiscs = displacementBetweenDiscs.GetLength();
	displacementBetweenDiscs.Normalize();

	float overlapDistance = ( discARadius + discBRadius ) - distanceBetweenDiscs;

	Vec2 discBCorrection = displacementBetweenDiscs * (overlapDistance * 0.5f);
	Vec2 discACorrection = -discBCorrection;

	discBCenterPosition += discBCorrection;
	discACenterPosition	+= discACorrection;
}


//---------------------------------------------------------------------------------------------------------
void PushDiscOutOfPoint2D( Vec2& discCenterPosition, float discRadius, const Vec2& pointToPushOutOf )
{
	PushDiscOutOfDisc2D( discCenterPosition, discRadius, pointToPushOutOf, 0.f );
}


//---------------------------------------------------------------------------------------------------------
void PushDiscOutOfDisc2D( Vec2& pushedDiscCenterPosition, float pushedDiscRadius, const Vec2& staticDiscCenterPosition, float staticDiscRadius )
{
	if( !DoDiscsOverlap( pushedDiscCenterPosition, pushedDiscRadius, staticDiscCenterPosition, staticDiscRadius ) ) return;

	Vec2 displacementBetweenDiscs = pushedDiscCenterPosition - staticDiscCenterPosition;

	float distanceBetweenDiscs = displacementBetweenDiscs.GetLength();
	displacementBetweenDiscs.Normalize();

	float overlapDistance = ( staticDiscRadius + pushedDiscRadius ) - distanceBetweenDiscs;

	Vec2 pushedDiscCorrection = displacementBetweenDiscs * overlapDistance;
	pushedDiscCenterPosition += pushedDiscCorrection;
}


//---------------------------------------------------------------------------------------------------------
FloatRange GetRangeOnProjectedAxis( int numPoints, const Vec2* points, const Vec2& realativePos, const Vec2& axisNormal )
{
	FloatRange projectedRange = FloatRange( 999999999.f, -999999999.f );

	for( int pointIndex = 0; pointIndex < numPoints; ++pointIndex )
	{
		Vec2 pointRelativePos = points[ pointIndex ] + realativePos;
		float projectedValue = GetProjectedLength2D( pointRelativePos, axisNormal.GetRotatedMinus90Degrees() );

		if( projectedValue < projectedRange.minimum )
		{
			projectedRange.minimum = projectedValue;
		}
		
		if( projectedValue > projectedRange.maximum )
		{
			projectedRange.maximum = projectedValue;
		}
	}

	return projectedRange;
}


//---------------------------------------------------------------------------------------------------------
int PositiveMod( int numberToMod, int numberToModBy )
{
	return abs( numberToMod - ( ( numberToMod / numberToModBy ) * numberToModBy ) );
}


//---------------------------------------------------------------------------------------------------------
float GetShortestAngularDisplacement( float startAngle, float endAngle )
{
	float angularDisplacement = endAngle - startAngle;

	while( angularDisplacement > 180.f || angularDisplacement < -180.f )
	{
		if( angularDisplacement > 180.f )
		{
			angularDisplacement -= 360.f;
		}
		else if( angularDisplacement < -180.f )
		{
			angularDisplacement += 360.f;
		}
	}

	return angularDisplacement;
}


//---------------------------------------------------------------------------------------------------------
float GetTurnedToward( float startAngle, float endAngle, float amountToTurnDegrees )
{
	float shortestAngularDisplacement = GetShortestAngularDisplacement( startAngle, endAngle );
	if( fabsf( shortestAngularDisplacement ) < amountToTurnDegrees )
	{
		return endAngle;
	}
	
	if( shortestAngularDisplacement > 0.f )
	{
		return startAngle + amountToTurnDegrees;
	}
	else
	{
		return startAngle - amountToTurnDegrees;
	}
}


//---------------------------------------------------------------------------------------------------------
float DotProduct2D( const Vec2& a, const Vec2& b )
{
	float firstElement = a.x * b.x;
	float secondElement = a.y * b.y;

	return firstElement + secondElement;
}


//---------------------------------------------------------------------------------------------------------
float DotProduct3D( const Vec3& a, const Vec3& b )
{
	float firstElement	= a.x * b.x;
	float secondElement	= a.y * b.y;
	float thirdElement	= a.z * b.z;

	return firstElement + secondElement + thirdElement;
}


//---------------------------------------------------------------------------------------------------------
float DotProduct4D( const Vec4& a, const Vec4& b )
{
	float firstElement	= a.x * b.x;
	float secondElement	= a.y * b.y;
	float thirdElement	= a.z * b.z;
	float fourthElement	= a.w * b.w;

	return firstElement + secondElement + thirdElement + fourthElement;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnOBB2D( const Vec2& refPos, const OBB2& box )
{
	return box.GetNearestPoint( refPos );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnPolygon2D( const Vec2& refPos, const Polygon2D& polygon )
{
	return polygon.GetClosestPoint( refPos );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnEdgePolygon2D( const Vec2& refPos, const Polygon2D& polygon )
{
	return polygon.GetClosestPointOnEdge( refPos );
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInsideDisk2D( const Vec2& point, const Vec2& discCenter, float discRadius )
{
	Vec2 displacementToPoint = point - discCenter;
	if( displacementToPoint.GetLength() <= discRadius )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInsideAABB2D( const Vec2& point, const AABB2& box )
{
	return box.IsPointInside( point );
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	Vec2 displacementToPointFromStart = point - capsuleMidStart;
	Vec2 capsuleVector = capsuleMidEnd - capsuleMidStart;
	float projectedLength = DotProduct2D( displacementToPointFromStart, capsuleVector.GetNormalized() );

	if( projectedLength < 0 )
	{
		return IsPointInsideDisk2D( point, capsuleMidStart, capsuleRadius );
	}
	else if( projectedLength > capsuleVector.GetLength() )
	{
		return IsPointInsideDisk2D( point, capsuleMidEnd, capsuleRadius );
	}
	else
	{
		Vec2 projectedVector = GetProjectedOnto2D( displacementToPointFromStart, capsuleVector );
		Vec2 perpendicularComponent = displacementToPointFromStart - projectedVector;
		if( perpendicularComponent.GetLength() <= capsuleRadius )
		{
			return true;
		}
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInsideOBB2D( const Vec2& point, const OBB2& box )
{
	return box.IsPointInside( point );
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInsidePolygon2D( const Vec2& point, const Polygon2D& polygon )
{
	return polygon.IsPointInside( point );
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInForwardSector2D( const Vec2& pointToCheck, const Vec2& startPosition, float maxDistance, const Vec2& fwdDir, float aperatureDegrees )
{
	Vec2 displacement = pointToCheck - startPosition;

	if( displacement.GetLength() < maxDistance )
	{
		Vec2 displacementNormal = displacement.GetNormalized();
		float cosBetweenNormals = DotProduct2D( displacementNormal, fwdDir );
		if( cosBetweenNormals >= CosDegrees( aperatureDegrees * 0.5f ) )
		{
			return true;
		}
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool IsPointInForwardSector2D( const Vec2& pointToCheck, const Vec2& startPosition, float fwdDirOrientationDegrees, float aperatureDegrees, float maxDistance )
{
	Vec2 fwdDir = Vec2::MakeFromPolarDegrees( fwdDirOrientationDegrees );
	return IsPointInForwardSector2D( pointToCheck, startPosition, maxDistance, fwdDir, aperatureDegrees);
}


//---------------------------------------------------------------------------------------------------------
const Vec2 GetProjectedOnto2D( const Vec2& vectorToProject, const Vec2& vectorToProjectOn )
{
	Vec2 normalVectorToProjectOn = vectorToProjectOn.GetNormalized();
	return DotProduct2D( vectorToProject, normalVectorToProjectOn ) * normalVectorToProjectOn;
}


//---------------------------------------------------------------------------------------------------------
int GetTaxicabDistance2D( const IntVec2& startPosition, const IntVec2& endPosition )
{
	IntVec2 displacement = endPosition - startPosition; 
	return displacement.GetTaxiCabLength(); 
}


//---------------------------------------------------------------------------------------------------------
float GetProjectedLength2D( const Vec2& vectorToProject, const Vec2& vectorToProjectOn )
{
	Vec2 normalVectorToProjectOn = vectorToProjectOn.GetNormalized();
	return DotProduct2D( vectorToProject, normalVectorToProjectOn );
}


//---------------------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors2D( const Vec2& a, const Vec2& b )
{
	//float cosDeltaDegrees = DotProduct2D( a, b );
	float aOrientation = a.GetAngleDegrees();
	float bOrientation = b.GetAngleDegrees();

	return fabsf( aOrientation - bOrientation);
}