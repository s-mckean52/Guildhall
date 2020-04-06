#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <math.h>


//---------------------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec3& copyFrom )
	: x( copyFrom.x )
	, y( copyFrom.y )
	, z( copyFrom.z )
{
}


//---------------------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY, float initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}


//---------------------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec2& copyFrom, float z )
	: x( copyFrom.x )
	, y( copyFrom.y )
	, z( z )
{
}


//---------------------------------------------------------------------------------------------------------
Vec3::Vec3( float initialXYZ )
	: x( initialXYZ )
	, y( initialXYZ )
	, z( initialXYZ )
{
}


//---------------------------------------------------------------------------------------------------------
void Vec3::Normalize()
{
	if( x != 0 || y != 0 || z != 0 )
	{
		float scale = 1.f / GetLength();
		x *= scale;
		y *= scale;
		z *= scale;
	}
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetLength() const
{
	return sqrtf( ( x * x ) + ( y * y ) + ( z * z ) );
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXY() const
{
	return sqrtf( ( x * x ) + ( y * y ) );
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetLengthSquared() const
{
	return ( x * x ) + ( y * y ) + ( z * z );
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXYSquared() const
{
	return ( x * x ) + ( y * y );
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZRadians() const
{
	return atan2f( y, x );
}


//---------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZDegrees() const
{
	return Atan2Degrees( y, x );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	float r = GetLengthXY();
	float thetaRadians = GetAngleAboutZRadians();

	thetaRadians += deltaRadians;

	return Vec3( r * cosf( thetaRadians ), r * sinf( thetaRadians ), z );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::GetRotatedAboutZDegrees( float deltaDegrees ) const
{
	float r = GetLengthXY();
	float thetaDegrees = GetAngleAboutZDegrees();

	thetaDegrees += deltaDegrees;

	return Vec3( r * CosDegrees( thetaDegrees ), r * SinDegrees( thetaDegrees ), z );
}


const Vec3 Vec3::GetClamped( float maxLength )
{
	float currentLength = GetLength();
	
	if( currentLength > maxLength )
	{
		if( x != 0 || y != 0 || z != 0 )
		{
			float scale = 1.f / currentLength;
			return Vec3( x * scale, y * scale, z * scale );
		}
	}
	return Vec3( x, y, z );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::GetNormalize() const
{
	if( x != 0 || y != 0 || z != 0 )
	{
		float scale = 1.f / GetLength();
		return Vec3( x * scale, y * scale, z * scale );
	}
	return Vec3( x, y, z );
}


//---------------------------------------------------------------------------------------------------------
void Vec3::SetFromText(const char* text)
{
	Strings splitText = SplitStringOnDelimiter( text, ',' );

	GUARANTEE_OR_DIE( splitText.size() == 3, Stringf( "Invalid Format for Vec2, %i values in string", splitText.size() ) );

	x = static_cast<float>( atof(splitText[ 0 ].c_str() ) );
	y = static_cast<float>( atof(splitText[ 1 ].c_str() ) );
	z = static_cast<float>( atof(splitText[ 2 ].c_str() ) );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator+( const Vec3& vecToAdd ) const
{
	return Vec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const
{
	return Vec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const
{
	return Vec3( x / inverseScale, y / inverseScale, z / inverseScale);
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const
{
	return Vec3( x * uniformScale, y * uniformScale, z * uniformScale);
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator/=( const float inverseScale )
{
	x /= inverseScale;
	y /= inverseScale;
	z /= inverseScale;
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//---------------------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//---------------------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const
{
	if( x != compare.x || y != compare.y || z != compare.z )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const
{
	if( x == compare.x && y == compare.y && z == compare.z )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToMultiply )
{
	return Vec3( vecToMultiply.x * uniformScale, vecToMultiply.y * uniformScale, vecToMultiply.z * uniformScale );
}


//---------------------------------------------------------------------------------------------------------
STATIC const Vec3 Vec3::ZERO	( 0.0f, 0.0f, 0.0f );
STATIC const Vec3 Vec3::RIGHT	( 1.0f, 0.0f, 0.0f );
STATIC const Vec3 Vec3::UP		( 0.0f, 1.0f, 0.0f );
STATIC const Vec3 Vec3::INTO	( 0.0f, 0.0f, 1.0f );
STATIC const Vec3 Vec3::FORWARD	( 0.0f, 0.0f, -1.0f );
STATIC const Vec3 Vec3::UNIT	( 1.0f, 1.0f, 1.0f );