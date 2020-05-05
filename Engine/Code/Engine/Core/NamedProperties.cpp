#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
NamedProperties::~NamedProperties()
{
	for( auto iter : m_keyValuePairs )
	{
		delete iter.second;
	}
	m_keyValuePairs.clear();
}


//---------------------------------------------------------------------------------------------------------
void NamedProperties::PopulateFromString( std::string arguments )
{
	Strings splitString = SplitStringOnDelimiter( arguments, ' ' );

	for( int argIndex = 0; argIndex < splitString.size(); ++argIndex )
	{
		std::string keyValueString = splitString[argIndex];
		Strings keyValuePair = SplitStringOnDelimiter( keyValueString, '=' );

		GUARANTEE_OR_DIE(keyValuePair.size() == 2, "Named populate from string format not 'key=value'");

		std::string keyName = keyValuePair[0];
		std::string keyValue = keyValuePair[1];

		SetValue( keyName, keyValue );
	}
}


//---------------------------------------------------------------------------------------------------------
void NamedProperties::SetValue( std::string const& keyName, char const* value )
{
	SetValue<std::string>( keyName, value );
}


//---------------------------------------------------------------------------------------------------------
std::string NamedProperties::GetValue( std::string const& keyName, char const* defValue ) const
{
	return GetValue<std::string>( keyName, defValue );
}


//---------------------------------------------------------------------------------------------------------
TypedPropertyBase* NamedProperties::FindInMap( std::string const& key ) const
{
	auto iter = m_keyValuePairs.find( key );
	if( iter != m_keyValuePairs.end() )
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}