#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


//---------------------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromPolarDegrees( float degrees, float r )
{
	return Vec2( r * CosDegrees( degrees ), r * SinDegrees( degrees ) );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromPolarRadians( float radians, float r )
{
	return Vec2( r * cosf( radians ), r * sinf( radians ) );
}


//---------------------------------------------------------------------------------------------------------
float Vec2::GetLength() const
{
	return sqrtf( ( x * x ) + ( y * y ) );
}


//---------------------------------------------------------------------------------------------------------
float Vec2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}


//---------------------------------------------------------------------------------------------------------
float Vec2::GetAngleDegrees() const
{
	return Atan2Degrees( y, x );
}


//---------------------------------------------------------------------------------------------------------
float Vec2::GetAngleRadians() const
{
	return atan2f( y, x );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotated90Degrees() const
{
	return Vec2( -y, x );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2( y, -x );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedDegrees( float deltaDegrees ) const
{
	float r = GetLength();
	float thetaDegrees = GetAngleDegrees();

	thetaDegrees += deltaDegrees;

	return Vec2( r * CosDegrees(thetaDegrees), r * SinDegrees(thetaDegrees) );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedRadians( float deltaRadians ) const
{
	float r = GetLength();
	float thetaRadians = GetAngleRadians();

	thetaRadians += deltaRadians;

	return Vec2( r * cosf( thetaRadians ), r * sinf( thetaRadians ) );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetClamped( float maxLength ) const
{
	float currentLength = GetLength();
	if( currentLength > maxLength )
	{
		if( x != 0 || y != 0 )
		{
			float scale = maxLength / currentLength;
			return Vec2( x * scale, y * scale );
		}
	}
	return Vec2( x, y );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetNormalized() const
{
	if( x != 0 || y != 0 )
	{
		float scale = 1.f / GetLength();
		return Vec2( x * scale, y * scale );
	}
	return Vec2( x, y );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Vec2::GetReflected( const Vec2& vectorToRelectAround ) const
{
	Vec2 projectedOnNormal = GetProjectedOnto2D( *this, vectorToRelectAround );
	return *this - ( projectedOnNormal * 2 );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetFromText( const char* text )
{
	Strings splitText = SplitStringOnDelimiter( text, ',' );

	GUARANTEE_OR_DIE( splitText.size() == 2, Stringf( "Invalid Format for Vec2, %i values in string", splitText.size() ) );

	x = static_cast<float>( atof( splitText[ 0 ].c_str() ) );
	y = static_cast<float>( atof( splitText[ 1 ].c_str() ) );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetLength( float newLength )
{
	float oldLength = GetLength();

	if(oldLength != 0.f)
	{
		float scale = newLength / oldLength;
		x *= scale;
		y *= scale;
	}
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetAngleDegrees( float newOrientationDegrees )
{
	float r = GetLength();
	
	x = r * CosDegrees( newOrientationDegrees );
	y = r * SinDegrees( newOrientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetAngleRadians( float newOrientationRadians )
{
	float r = GetLength();

	x = r * cosf( newOrientationRadians );
	y = r * sinf( newOrientationRadians );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetPolarDegrees( float newOrientationDegrees, float newLength )
{
	x = newLength * CosDegrees( newOrientationDegrees );
	y = newLength * SinDegrees( newOrientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::SetPolarRadians( float newOrientationRadians, float newLength )
{
	x = newLength * cosf( newOrientationRadians );
	y = newLength * sinf( newOrientationRadians );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::RotateDegrees( float deltaDegrees )
{
	float r = GetLength();
	float thetaDegrees = GetAngleDegrees();

	thetaDegrees += deltaDegrees;

	x = r * CosDegrees( thetaDegrees );
	y = r * SinDegrees( thetaDegrees );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::RotateRadians( float deltaRadians )
{
	float r = GetLength();
	float thetaRadians = GetAngleRadians();

	thetaRadians += deltaRadians;

	x = r * cosf( thetaRadians );
	y = r * sinf( thetaRadians );
}


//---------------------------------------------------------------------------------------------------------
void Vec2::Rotate90Degrees()
{
	float tempX = x;

	x = -y;
	y = tempX;
}


//---------------------------------------------------------------------------------------------------------
void Vec2::RotateMinus90Degrees()
{
	float tempX = x;

	x = y;
	y = -tempX;
}


//---------------------------------------------------------------------------------------------------------
void Vec2::ClampLength( float maxLength )
{
	float currentLength = GetLength();
	if( currentLength > maxLength )
	{
		if( x != 0 || y != 0 )
		{
			float scale = maxLength / currentLength;
			x *= scale;
			y *= scale;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Vec2::Normalize()
{
	if( x != 0 || y != 0 )
	{
		float scale = 1.f / GetLength();
		x *= scale;
		y *= scale;
	}
}


//---------------------------------------------------------------------------------------------------------
float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	if( x != 0 && y != 0 )
	{
		float scale = 1.f / length;
		x *= scale;
		y *= scale;
	}
	return length;
}

void Vec2::Reflect( const Vec2& vectorToReflectAround )
{
	Vec2 projectedOnNormal = GetProjectedOnto2D( *this, vectorToReflectAround);
	*this -= projectedOnNormal * 2.f;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator+ ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}																										 


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x / inverseScale, y / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	if( x == compare.x && y == compare.y ) 
	{
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	if( x != compare.x || y != compare.y )
	{
		return true;
	}
	return false;
}



//---------------------------------------------------------------------------------------------------------
STATIC const Vec2 Vec2::ZERO	( 0.0f, 0.0f );
STATIC const Vec2 Vec2::RIGHT	( 1.0f, 0.0f );
STATIC const Vec2 Vec2::LEFT	( -1.0f, 0.0f );
STATIC const Vec2 Vec2::UP		( 0.0f, 1.0f );
STATIC const Vec2 Vec2::DOWN	( 0.0f, -1.0f );