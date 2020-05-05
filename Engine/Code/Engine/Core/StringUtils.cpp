#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//---------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter( const std::string& originalString, char delimeterToSplitOn )
{
	Strings splitStrings;
	size_t substringStartIndex = 0;

	for( ;; )
	{
		size_t delimeterIndex = originalString.find( delimeterToSplitOn, substringStartIndex );
		size_t substringLength = delimeterIndex - substringStartIndex;

		std::string substring( originalString, substringStartIndex, substringLength );
		splitStrings.push_back( substring );

		if( delimeterIndex == std::string::npos )
			break;

		substringStartIndex = delimeterIndex + 1;
	}

	return splitStrings;
}


//---------------------------------------------------------------------------------------------------------
std::string FindNextWord( std::string const& string, unsigned int& startIndex )
{
	bool wordFound = false;
	std::string foundWord = "";
	unsigned int charIndex = 0;
	for( charIndex = startIndex; charIndex < string.length(); ++charIndex )
	{
		char currentChar = string[ charIndex ];
		if( !IsWhiteSpace( currentChar ) )
		{
			if( !wordFound )
			{
				startIndex = charIndex;
			}
			wordFound = true;
		}
		else if( IsWhiteSpace( currentChar ) && wordFound )
		{
			break;
		}
	}
	if( charIndex != startIndex )
	{
		foundWord = string.substr( startIndex, charIndex - startIndex );
		startIndex = charIndex + 1;
	}
	return foundWord;
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( int value )
{
	return Stringf( "%i", value );
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( uint value )
{
	return Stringf( "%u", value );
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( float value )
{
	return Stringf( "%f", value );
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( char value )
{
	return Stringf( "%c", value );
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( double value )
{
	return Stringf( "%f", value );
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( bool value )
{
	if( value == true )
	{
		return "true";
	}
	else
	{
		return "false";
	}
}

//---------------------------------------------------------------------------------------------------------
std::string ToString( char const* value )
{
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string ToString( std::string const& value )
{
	return value;
}


//---------------------------------------------------------------------------------------------------------
int SetFromText( char const* valueAsText, int defaultValue )
{
	int value = atoi( valueAsText );
	if( !IsStringEqual( valueAsText, "0" ) && value == 0 )
	{
		value = defaultValue;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
uint SetFromText( char const* valueAsText, uint defaultValue )
{
	uint value = atoi( valueAsText );
	if( !IsStringEqual( valueAsText, "0" ) && value == 0 )
	{
		value = defaultValue;
	}
	else if( value < 0 )
	{
		value = defaultValue;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
float SetFromText( char const* valueAsText, float defaultValue )
{
	float value = static_cast<float>( atof( valueAsText ) );
	if( !IsStringEqual( valueAsText, "0" ) && value == 0.f )
	{
		value = defaultValue;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
double SetFromText( char const* valueAsText, double defaultValue )
{
	double value = atof( valueAsText );
	if( !IsStringEqual( valueAsText, "0" ) && value == 0.0 )
	{
		value = defaultValue;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
char SetFromText( char const* valueAsText, char defaultValue )
{
	char value;
	int length = sizeof( valueAsText );
	if( length > 1 || length == 0 )
	{
		value = defaultValue;
	}
	else
	{
		value = valueAsText[0];
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
bool SetFromText( char const* valueAsText, bool defaultValue )
{
	bool value;
	if( IsStringEqual( valueAsText, "true" ) )
	{
		value = true;
	}
	else if( IsStringEqual( valueAsText, "false" ) )
	{
		value = false;
	}
	else
	{
		value = defaultValue;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string SetFromText( char const* valueAsText, char const* defaultValue )
{
	UNUSED( defaultValue );
	return valueAsText;
}


//---------------------------------------------------------------------------------------------------------
std::string SetFromText( char const* valueAsText, std::string const& defaultValue )
{
	UNUSED( defaultValue );
	return valueAsText;
}
