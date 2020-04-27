#include "Engine/Core/StringUtils.hpp"
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