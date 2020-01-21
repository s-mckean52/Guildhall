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




