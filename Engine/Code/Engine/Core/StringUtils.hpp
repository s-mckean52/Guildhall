#pragma once
//---------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector< std::string > Strings;
typedef unsigned int uint;
//---------------------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );


//---------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter( const std::string& originalString, char delimeterToSplitOn );


//---------------------------------------------------------------------------------------------------------
inline bool IsUpperCase( char const c )			{ return ((c >= 'A') && (c <= 'Z')); }
inline bool IsLowerCase( char const c )			{ return ((c >= 'a') && (c <= 'z')); }
inline bool IsWhiteSpace( char const c )		{ return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')); }
inline bool IsNewLine( char const c )			{ return ((c == '\n') || (c == '\r' ) || (c == '\t')); }
inline bool IsAlpha( char const c )				{ return (IsUpperCase(c) || IsLowerCase(c)); }
inline bool IsNumeric( char const c )			{ return ((c >= '0') && (c <= '9')); }
inline bool IsAlphaNumeric( char const c )		{ return (IsAlpha(c) || IsNumeric(c)); }

//---------------------------------------------------------------------------------------------------------
inline bool IsStringEqual( char const* a, char const* b )	{ return ( strcmp( a, b ) == 0 ); }

//---------------------------------------------------------------------------------------------------------
std::string FindNextWord( std::string const& stringToParse, unsigned int& startIndex );

//---------------------------------------------------------------------------------------------------------
std::string ToString( int value );
std::string ToString( uint value );
std::string ToString( uint16_t value );
std::string ToString( float value );
std::string ToString( double value );
std::string ToString( char value );
std::string ToString( bool value );
std::string ToString( char const* value );
std::string ToString( std::string const& value );

template<typename T>
std::string ToString( T value )
{
	return T::ToString( value );
}


//---------------------------------------------------------------------------------------------------------
int			SetFromText( char const* valueAsText, int defaultValue );
uint		SetFromText( char const* valueAsText, uint defaultValue );
float		SetFromText( char const* valueAsText, float defaultValue );
double		SetFromText( char const* valueAsText, double defaultValue );
char		SetFromText( char const* valueAsText, char defaultValue );
bool		SetFromText( char const* valueAsText, bool defaultValue );
std::string	SetFromText( char const* valueAsText, char const* defaultValue );
std::string	SetFromText( char const* valueAsText, std::string const& defaultValue );

template<typename T>
T SetFromText( char const* valueAsText, T defaultValue )
{
	T value;
	value.SetFromText( valueAsText );
	return value;
}