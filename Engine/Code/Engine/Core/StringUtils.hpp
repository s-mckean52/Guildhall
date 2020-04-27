#pragma once
//---------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector< std::string > Strings;

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