#pragma once
#include <vector>

struct	Vec2;
struct	Vec3;
struct	Vec4;
struct	IntVec2;
struct	Vertex_PCU;
struct	AABB2;
struct	OBB2;
struct	FloatRange;
struct	Rgba8;
class	Polygon2D;

constexpr float radiansToDegreesRatio = 180.f / 3.14159265f;
constexpr float degreesToRadiansRatio = 3.14159265f / 180.f;

float		ConvertDegreesToRadians	( float degrees );
float		ConvertRadiansToDegrees	( float radians );
float		CosDegrees				( float degrees );
float		SinDegrees				( float degrees );
float		TanDegrees				( float degrees );
float		Atan2Degrees			( float y, float x );

float		GetDistance2D			( const Vec2& positionA, const Vec2& positionB );
float		GetDistanceSquared2D	( const Vec2& positionA, const Vec2& positionB );
float		GetAngleToPointDegrees	( const Vec2& startPosition, const Vec2& positionToRotateTo );

float		GetDistance3D			( const Vec3& positoinA, const Vec3& positionB );
float		GetDistanceXY3D			( const Vec3& positionA, const Vec3& positionB );
float		GetDistanceSquared3D	( const Vec3& positionA, const Vec3& positionB );
float		GetDistanceXYSquared3D	( const Vec3& positionA, const Vec3& positionB );

float		Lerp					( float rangeBegin, float rangeEnd, float fractionOfRangeEnd );
Rgba8		Rgba8Lerp				( const Rgba8& beginColor, const Rgba8& endColor, float fractionOfRange );
float		RangeMapFloat			( float fromBegin, float fromEnd, float toBegin, float toEnd, float fromValue );
int			GetClamp				( int valueToClamp, int lowerLimit, int upperLimit );
float		GetClamp				( float valueToClamp, float lowerLimit, float upperLimit );
double		GetClamp				( double valueToClamp, double lowerLimit, double upperLimit );
float		GetClampZeroToOne		( float valueToClamp );
void		Clamp					( int& valueToClamp, int lowerLimit, int upperLimit );
void		Clamp					( float& valueToClamp, float lowerLimit, float upperLimit );
void		Clamp					( double& valueToClamp, double lowerLimit, double upperLimit );
void		ClampZeroToOne			( float& valueToClamp );
bool		ApproximatelyEqual		( float valueToCheck, float valueToCheckAgainst, float percisionValue = 0.0001f );
float		Signf					( float value );	

const Vec2	TransformPosition2D		( const Vec2& position, float uniformScale, float rotationDegrees, Vec2 translation);
const Vec2	TransformPosition2D		( const Vec2& position, const Vec2& newBasisI, const Vec2& newBasisJ, const Vec2& translation);

const Vec3	TransformPosition3DXY	( const Vec3& position, float uniformScale, float rotationAroundZDegrees, Vec2 translationXY);
const Vec3	TransformPosition3DXY	( const Vec3& position, const Vec2& basisI, const Vec2& basisJ, const Vec2& translation );

void		TransformVertexArray	( int numVerticies, Vertex_PCU* vertexArray, float scale, float rotation, const Vec2& translation );
void		TransformVertexArray	( std::vector<Vertex_PCU>& vertexArray, float scale, float rotation, const Vec2& translation );

void		TranslateVertexArray	( int numVerticies, Vertex_PCU* vertexArray, const Vec3& translation );
void		TranslateVertexArray	( std::vector<Vertex_PCU>& vertexArray, const Vec3& translation );

int			PositiveMod( int numberToMod, int numberToModBy );
int			RoundDownToInt					( float valueToRound );
float		GetShortestAngularDisplacement	( float startAngle, float endAngle );
float		GetTurnedToward					( float startAngle, float endAngle, float amountToTurnDegrees );

float		DotProduct2D					( const Vec2& a, const Vec2& b );
float		DotProduct3D					( const Vec3& a, const Vec3& b );
float		DotProduct4D					( const Vec4& a, const Vec4& b );

Vec3		CrossProduct3D					( const Vec3& vecFrom, const Vec3& vecTo );
Vec2		TripleCrossProduct2D			( const Vec2& vecFrom, const Vec2& vecTo, const Vec2& vecBackTo );
Vec3		TripleCrossProduct3D			( const Vec3& vecFrom, const Vec3& vecTo, const Vec3& vecBackTo );

int			GetTaxicabDistance2D			( const IntVec2& startPosition, const IntVec2& endPosition );



//---------------------------------------------------------------------------------------------------------
// Geometric Utilities
//
bool		DoDiscsOverlap( const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB );
bool		DoSpheresOverlap( const Vec3& centerA, float radiusA, const Vec3& centerB, float radiusB );
bool		DoAABB2sOverlap( const AABB2& boxA, const AABB2& boxB );
bool		DoDiscAndAABB2Overlap( const AABB2& box, const Vec2& discCenterPosition, float discRadius );
bool		DoOBBAndOBBOverlap2D( const OBB2& boxA, const OBB2& boxB );
bool		DoOBBAndAABBOverlap2D( const OBB2& obb, const AABB2& aabb );
bool		DoOBBAndLineSegmentOverlap2D( const OBB2& obb, const Vec2& lineStart, const Vec2& lineEnd );
bool		DoOBBAndCapsuleOverlap2D( const OBB2& obb, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
bool		DoOBBAndDiscOverlap2D( const OBB2& obb, const Vec2& discCenter, float discRadius );
bool		DoPolygonAndDiscOverlap( const Polygon2D& polygon, const Vec2& discCenter, float discRadius );
bool		DoPolygonsOverlap( Polygon2D polygonA, Polygon2D polygonB );

void		PushDiscOutOfAABB2( Vec2& discCenterPosition, float discRadius, const AABB2& box );
void		PushDiscsOutOfEachOther2D( Vec2& discACenterPosition, float discARadius, Vec2& discBCenterPosition, float discBRadius );
void		PushDiscOutOfPoint2D( Vec2& discCenterPosition, float discRadius, const Vec2& pointToPushOutOf );
void		PushDiscOutOfDisc2D( Vec2& pushedDiscCenterPosition, float pushedDiscRadius, const Vec2& staticDiscCenterPosition, float staticDiscRadius );

FloatRange	GetRangeOnProjectedAxis( int numPoints, const Vec2* points, const Vec2& realativePos, const Vec2& axisNormal );
float		GetProjectedLength2D( const Vec2& vectorToProject, const Vec2& vectorToProjectOn );
const Vec2	GetProjectedOnto2D( const Vec2& vectorToProject, const Vec2& vectorToProjectOn );
float		GetAngleDegreesBetweenVectors2D( const Vec2& a, const Vec2& b );

const Vec2	GetNearestPointOnAABB2D( const Vec2& pointToCheck, const AABB2& box );
const Vec2	GetNearestPointOnDisc2D( const Vec2& pointToCheck, const Vec2& discCenterPosition, float discRadius );
const Vec2	GetNearestPointOnInfiniteLine2D( const Vec2& refPos, const Vec2& somePointOnLine, const Vec2& anotherPointOnLine );
const Vec2	GetNearestPointOnLineSegment2D( const Vec2& refPos, const Vec2& start, const Vec2& end );
const Vec2	GetNearestPointOnCapsule2D( const Vec2& refPos, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
const Vec2	GetNearestPointOnOBB2D( const Vec2& refPos, const OBB2& box );
const Vec2	GetNearestPointOnPolygon2D( const Vec2& refPos, const Polygon2D& polygon );
const Vec2	GetNearestPointOnEdgePolygon2D( const Vec2& refPos, const Polygon2D& polygon );

bool		IsPointInsideDisk2D( const Vec2& point, const Vec2& discCenter, float discRadius);
bool		IsPointInsideAABB2D( const Vec2& point, const AABB2& box );
bool		IsPointInsideCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
bool		IsPointInsideOBB2D( const Vec2& point, const OBB2& box );
bool		IsPointInsidePolygon2D( const Vec2& point, const Polygon2D& polygon );
bool		IsPointInForwardSector2D( const Vec2& pointToCheck, const Vec2& startPosition, float maxDistance, const Vec2& fwdDir, float aperatureDegrees );
bool		IsPointInForwardSector2D( const Vec2& pointToCheck, const Vec2& startPosition, float fwdDirOrientationDegrees, float aperatureDegrees, float maxDistance );


//---------------------------------------------------------------------------------------------------------
// SmoothingFunctions
//
float SmoothStart2( float t );
float SmoothStart3( float t );
float SmoothStart4( float t );
float SmoothStart5( float t );

float SmoothStop2( float t );
float SmoothStop3( float t );
float SmoothStop4( float t );
float SmoothStop5( float t );

float SmoothStep3( float t );