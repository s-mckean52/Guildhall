#include <math.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
IntVec2::IntVec2( int initialX, int initialY )
	: x( initialX )
	, y( initialY )
{
}


//---------------------------------------------------------------------------------------------------------
IntVec2::IntVec2( const IntVec2& copyFrom )
	: x( copyFrom.x )
	, y( copyFrom.y )
{
}


//---------------------------------------------------------------------------------------------------------
float IntVec2::GetLength() const
{
	return sqrtf( static_cast<float>( ( x * x ) + ( y * y ) ) );
}


//---------------------------------------------------------------------------------------------------------
int IntVec2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}


//---------------------------------------------------------------------------------------------------------
int IntVec2::GetTaxiCabLength() const
{
	return abs( x ) + abs( y );
}


//---------------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees( static_cast< float >( y ), static_cast< float >( x ) );
}


//---------------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationRadians() const
{
	return atan2f( static_cast< float >( y ), static_cast< float >( x ) );
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y, x);
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2( y, -x);
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::SetFromText( const char* text )
{
	Strings splitText = SplitStringOnDelimiter( text, ',' );

	GUARANTEE_OR_DIE( splitText.size() == 2, Stringf( "Invalid Set From Text Format for IntVec2, %i values in string", splitText.size() ) );

	x = atoi( splitText[ 0 ].c_str() );
	y = atoi( splitText[ 1 ].c_str() );
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::Rotate90Degrees()
{
	int tempX = x;

	x = -y;
	y = tempX;
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::RotateMinus90Degrees()
{
	int tempX = x;

	x = y;
	y = -tempX;
}


//---------------------------------------------------------------------------------------------------------
bool IntVec2::operator==( const IntVec2& compare ) const
{
	if( x == compare.x && y == compare.y )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool IntVec2::operator!=( const IntVec2& compare ) const
{
	if( x != compare.x || y != compare.y )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator+( const IntVec2& intVecToAdd ) const
{
	return IntVec2( x + intVecToAdd.x, y + intVecToAdd.y );
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-( const IntVec2& intVecToSubtract ) const
{
	return IntVec2( x - intVecToSubtract.x, y - intVecToSubtract.y );
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-() const
{
	return IntVec2( -x , -y);
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*( int uniformScale ) const
{
	return IntVec2( x * uniformScale, y * uniformScale );
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*( const IntVec2& intVecToMultiply ) const
{
	return IntVec2( x * intVecToMultiply.x, y * intVecToMultiply.y );
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::operator+=( const IntVec2& intVecToAdd )
{
	x += intVecToAdd.x;
	y += intVecToAdd.y;
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::operator-=( const IntVec2& intVecToSubtract )
{
	x -= intVecToSubtract.x;
	y -= intVecToSubtract.y;
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::operator*=( const int uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//---------------------------------------------------------------------------------------------------------
void IntVec2::operator=( const IntVec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//---------------------------------------------------------------------------------------------------------
const IntVec2 operator*( int uniformScale, const IntVec2& intVecToScale )
{
	return IntVec2( intVecToScale.x * uniformScale, intVecToScale.y * uniformScale );
}
