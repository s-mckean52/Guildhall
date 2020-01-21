#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
FloatRange::FloatRange( float minAndMax )
	: minimum( minAndMax )
	, maximum( minAndMax )
{
}


//---------------------------------------------------------------------------------------------------------
FloatRange::FloatRange( float min, float max )
	: minimum( min )
	, maximum( max )
{
}


//---------------------------------------------------------------------------------------------------------
FloatRange::FloatRange( const char* asText )
{
	Strings minAndMaxAsString = SplitStringOnDelimiter( asText, '~' );
	if( minAndMaxAsString.size() == 1 )
	{
		minimum = static_cast<float>( atof( minAndMaxAsString[ 0 ].c_str() ) );
		maximum = minimum;
	}
	else if( minAndMaxAsString.size() == 2 )
	{
		minimum = static_cast<float>( atof( minAndMaxAsString[ 0 ].c_str() ) );
		maximum = static_cast<float>( atof( minAndMaxAsString[ 1 ].c_str() ) );
	}
	else
	{
		ERROR_AND_DIE( "Float range Constructor( const char* asText ) is malformated" );
	}
}


//---------------------------------------------------------------------------------------------------------
bool FloatRange::IsInRange( float value ) const
{
	if( value >= minimum && value <= maximum)
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool FloatRange::DoesOverlap( const FloatRange& otherRange ) const
{
	if( otherRange.maximum >= minimum && otherRange.maximum <= maximum )
	{
		return true;
	}
	else if( otherRange.minimum >= minimum && otherRange.minimum <= maximum )
	{
		return true;
	}
	
	if( maximum >= otherRange.minimum && maximum <= otherRange.maximum )
	{
		return true;
	}
	else if( minimum >= otherRange.minimum && minimum <= otherRange.maximum )
	{
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
std::string FloatRange::GetAsString() const
{
	return Stringf( "%f~%f", minimum, maximum );
}


//---------------------------------------------------------------------------------------------------------
float FloatRange::GetRandomInRange( RandomNumberGenerator& rng ) const
{
	return rng.RollRandomFloatInRange( minimum, maximum );
}


//---------------------------------------------------------------------------------------------------------
void FloatRange::Set( float newMinimum, float newMaximum )
{
	minimum = newMinimum;
	maximum = newMaximum;
}


//---------------------------------------------------------------------------------------------------------
bool FloatRange::SetFromText( const char* asText )
{
	Strings minAndMaxAsString = SplitStringOnDelimiter( asText, '~' );
	if( minAndMaxAsString.size() == 1 )
	{
		minimum = static_cast<float>( atof( minAndMaxAsString[ 0 ].c_str() ) );
		maximum = minimum;
		return true;
	}
	else if( minAndMaxAsString.size() == 2 )
	{
		minimum = static_cast<float>( atof( minAndMaxAsString[ 0 ].c_str() ) );
		maximum = static_cast<float>( atof( minAndMaxAsString[ 1 ].c_str() ) );
		return true;
	}
	return false;
}
