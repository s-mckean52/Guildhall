#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"


//---------------------------------------------------------------------------------------------------------
void NamedStrings::PopulateFromXmlElementAttribute( const XmlElement& element )
{
	const XmlAttribute* nextAttribute;
	for( nextAttribute = element.FirstAttribute(); nextAttribute; nextAttribute = nextAttribute->Next() )
	{
		std::string keyName		= nextAttribute->Name();
		std::string keyValue	= nextAttribute->Value();

		m_keyValuePairs.insert( { keyName, keyValue } );
	}
}


//---------------------------------------------------------------------------------------------------------
void NamedStrings::PopulateFromString( std::string const& arguments )
{
	Strings splitString = SplitStringOnDelimiter( arguments, ' ' );
	
	for( int argIndex = 0; argIndex < splitString.size(); ++argIndex )
	{
		std::string keyValueString = splitString[ argIndex ];
		Strings keyValuePair = SplitStringOnDelimiter( keyValueString, '=' );

		//GUARANTEE_OR_DIE( keyValuePair.size() == 2, "NamedStrings populate from string format not 'key=value'" );
		if( keyValuePair.size() != 2 )
		{
			return;
		}

		std::string keyName = keyValuePair[ 0 ];
		std::string keyValue = keyValuePair[ 1 ];

		m_keyValuePairs.insert( { keyName, keyValue } );
	}
}


//---------------------------------------------------------------------------------------------------------
void NamedStrings::SetValue( const std::string& keyName, const std::string& newValue )
{
	std::map<std::string, std::string>::iterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		mapIterator->second = newValue;
	}
	else
	{
		m_keyValuePairs.insert( { keyName, newValue } );
	}
}


//---------------------------------------------------------------------------------------------------------
bool NamedStrings::GetValue( const std::string& keyName, bool defaultValue ) const
{
	bool value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		if( valueText == "true" )
		{
			value = true;
		}
		else if( valueText == "false" )
		{
			value = false;
		}
		else
		{
			ERROR_AND_DIE( "GetValue failed for type bool" );
		}
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
int NamedStrings::GetValue( const std::string& keyName, int defaultValue ) const
{
	int value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value = atoi( valueText.c_str() );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
float NamedStrings::GetValue( const std::string& keyName, float defaultValue ) const
{
	float value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value = static_cast<float>( atof( valueText.c_str() ) );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	std::string value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		value = mapIterator->second;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	std::string value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		value = mapIterator->second;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Rgba8 NamedStrings::GetValue( const std::string& keyName, const Rgba8& defaultValue ) const
{
	Rgba8 value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value.SetFromText( valueText.c_str() );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Vec2 NamedStrings::GetValue( const std::string& keyName, const Vec2& defaultValue ) const
{
	Vec2 value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value.SetFromText( valueText.c_str() );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
IntVec2 NamedStrings::GetValue( const std::string& keyName, const IntVec2& defaultValue ) const
{
	IntVec2 value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value.SetFromText( valueText.c_str() );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
double NamedStrings::GetValue( const std::string& keyName, double defaultValue ) const
{
	double value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value = atof( valueText.c_str() );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Vec3 NamedStrings::GetValue( const std::string& keyName, const Vec3& defaultValue ) const
{
	Vec3 value = defaultValue;
	MapConstIterator mapIterator = m_keyValuePairs.find( keyName );
	if( mapIterator != m_keyValuePairs.cend() )
	{
		std::string valueText = mapIterator->second;
		value.SetFromText( valueText.c_str() );
	}
	return value;
}
