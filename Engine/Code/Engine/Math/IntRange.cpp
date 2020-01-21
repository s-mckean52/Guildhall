#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
IntRange::IntRange( int minAndMax )
	: minimum( minAndMax )
	, maximum( minAndMax )
{
}


//---------------------------------------------------------------------------------------------------------
IntRange::IntRange( int min, int max )
	: minimum( min )
	, maximum( max )
{
}


//---------------------------------------------------------------------------------------------------------
IntRange::IntRange( const char* asText )
{
	Strings minAndMaxAsString = SplitStringOnDelimiter( asText, '~' );
	if( minAndMaxAsString.size() == 1 )
	{
		minimum = atoi( minAndMaxAsString[ 0 ].c_str() );
		maximum = minimum;
	}
	else if( minAndMaxAsString.size() == 2 )
	{
		minimum = atoi( minAndMaxAsString[ 0 ].c_str() );
		maximum = atoi( minAndMaxAsString[ 1 ].c_str() );
	}
	else
	{
		ERROR_AND_DIE( "IntRange Constructor( const char* asText ) is malformated" );
	}
}


//---------------------------------------------------------------------------------------------------------
bool IntRange::IsInRange( int value ) const
{
	if( value >= minimum && value <= maximum)
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool IntRange::DoesOverlap( const IntRange& otherRange ) const
{
	if( otherRange.maximum >= minimum && otherRange.maximum <= maximum )
	{
		return true;
	}
	else if( otherRange.minimum >= minimum && otherRange.minimum <= maximum )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
std::string IntRange::GetAsString() const
{
	return Stringf( "%i~%i", minimum, maximum );
}


//---------------------------------------------------------------------------------------------------------
int IntRange::GetRandomInRange( RandomNumberGenerator& rng ) const
{
	return rng.RollRandomIntInRange( minimum, maximum );
}


//---------------------------------------------------------------------------------------------------------
void IntRange::Set( int newMinimum, int newMaximum )
{
	minimum = newMinimum;
	maximum = newMaximum;
}


//---------------------------------------------------------------------------------------------------------
bool IntRange::SetFromText( const char* asText )
{
	Strings minAndMaxAsString = SplitStringOnDelimiter( asText, '~' );
	if( minAndMaxAsString.size() == 1 )
	{
		minimum = atoi( minAndMaxAsString[ 0 ].c_str() );
		maximum = minimum;
		return true;
	}
	else if( minAndMaxAsString.size() == 2 )
	{
		minimum = atoi( minAndMaxAsString[ 0 ].c_str() );
		maximum = atoi( minAndMaxAsString[ 1 ].c_str() );
		return true;
	}
	return false;
}
