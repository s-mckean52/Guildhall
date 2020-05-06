#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Vec4::Vec4( float initialX, float initialY, float initialZ, float initialW )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
	, w( initialW )
{
}


//---------------------------------------------------------------------------------------------------------
Vec4::Vec4( const Vec4& toCopy )
	: x( toCopy.x )
	, y( toCopy.y )
	, z( toCopy.z )
	, w( toCopy.w )
{
}


//---------------------------------------------------------------------------------------------------------
Vec4::Vec4( const Vec3& toCopy, float initialW )
	: x( toCopy.x )
	, y( toCopy.y )
	, z( toCopy.z )
	, w( initialW )
{
}


//---------------------------------------------------------------------------------------------------------
Vec4::Vec4( const Vec2& toCopyFirst, const Vec2& toCopySecond )
{
	x = toCopyFirst.x;
	y = toCopyFirst.y;
	z = toCopySecond.x;
	w = toCopySecond.y;
}


//---------------------------------------------------------------------------------------------------------
std::string Vec4::ToString( Vec4 const& value )
{
	return Stringf( "%f,%f,%f,%f", value.x, value.y, value.z, value.w );
}


//---------------------------------------------------------------------------------------------------------
void Vec4::SetFromText( const char* text )
{
	Strings splitText = SplitStringOnDelimiter( text, ',' );

	GUARANTEE_OR_DIE( splitText.size() == 4, Stringf( "Invalid Format for Vec4, %i values in string", splitText.size() ) );

	x = static_cast<float>( atof( splitText[ 0 ].c_str() ) );
	y = static_cast<float>( atof( splitText[ 1 ].c_str() ) );
	z = static_cast<float>( atof( splitText[ 2 ].c_str() ) );
	w = static_cast<float>( atof( splitText[ 3 ].c_str() ) );
}


//---------------------------------------------------------------------------------------------------------
bool Vec4::operator==( const Vec4& toCompare ) const
{
	if( x == toCompare.x && y == toCompare.y && z == toCompare.z && w == toCompare.w )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Vec4::operator!=( const Vec4& toCompare ) const
{
	if( x == toCompare.x && y == toCompare.y && z == toCompare.z && w == toCompare.w )
	{
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator+( const Vec4& vecToAdd ) const
{
	float newX = x + vecToAdd.x; 
	float newY = y + vecToAdd.y; 
	float newZ = z + vecToAdd.z; 
	float newW = w + vecToAdd.w;

	return Vec4( newX, newY, newZ, newW );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-( const Vec4& vecToSubtract ) const
{
	float newX = x - vecToSubtract.x;
	float newY = y - vecToSubtract.y;
	float newZ = z - vecToSubtract.z;
	float newW = w - vecToSubtract.w;

	return Vec4( newX, newY, newZ, newW );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4( -x, -y, -z, -w );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*( const Vec4& vecToMultiply ) const
{
	float newX = x * vecToMultiply.x;
	float newY = y * vecToMultiply.y;
	float newZ = z * vecToMultiply.z;
	float newW = w * vecToMultiply.w;

	return Vec4( newX, newY, newZ, newW );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*( float uniformScale ) const
{
	float newX = x * uniformScale;
	float newY = y * uniformScale;
	float newZ = z * uniformScale;
	float newW = w * uniformScale;

	return Vec4( newX, newY, newZ, newW );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/( float inverseScale ) const
{
	float newX = x / inverseScale;
	float newY = y / inverseScale;
	float newZ = z / inverseScale;
	float newW = w / inverseScale;

	return Vec4( newX, newY, newZ, newW );
}


//---------------------------------------------------------------------------------------------------------
void Vec4::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//---------------------------------------------------------------------------------------------------------
void Vec4::operator-=( const Vec4& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//---------------------------------------------------------------------------------------------------------
void Vec4::operator+=( const Vec4& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//---------------------------------------------------------------------------------------------------------
void Vec4::operator/=( const float inverseScale )
{
	x /= inverseScale;
	y /= inverseScale;
	z /= inverseScale;
	w /= inverseScale;
}


//---------------------------------------------------------------------------------------------------------
void Vec4::operator=( const Vec4& vecToCopy )
{
	x = vecToCopy.x;
	y = vecToCopy.y;
	z = vecToCopy.z;
	w = vecToCopy.w;
}


//---------------------------------------------------------------------------------------------------------
const Vec4 operator*( float uniformScale, const Vec4& vecToMultiply )
{
	float newX = vecToMultiply.x * uniformScale;
	float newY = vecToMultiply.x * uniformScale;
	float newZ = vecToMultiply.x * uniformScale;
	float newW = vecToMultiply.x * uniformScale;

	return Vec4( newX, newY, newZ, newW );
}