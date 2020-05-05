#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <map>

template<typename T>
class TypedProperty;

class TypedPropertyBase;
class NamedProperties;


//---------------------------------------------------------------------------------------------------------
// TypedPropertyBase
//---------------------------------------------------------------------------------------------------------
class TypedPropertyBase
{
public:
	virtual ~TypedPropertyBase() {}

	virtual std::string GetAsString() const = 0;
	virtual void const* GetUniqueID() const = 0;

	template<typename T>
	bool Is() const;
};


//---------------------------------------------------------------------------------------------------------
template<typename T>
bool TypedPropertyBase::Is() const
{
	return GetUniqueID() == TypedProperty<T>::StaticUniqueID();
}


//---------------------------------------------------------------------------------------------------------
// TypedProperty
//---------------------------------------------------------------------------------------------------------
template<typename VALUE_TYPE>
class TypedProperty : public TypedPropertyBase
{
public:
	virtual std::string GetAsString() const final { return ToString( m_value ); }
	virtual void const* GetUniqueID() const final { return StaticUniqueID(); }

public:
	VALUE_TYPE m_value;

public:
	static void const* const StaticUniqueID();
};


//---------------------------------------------------------------------------------------------------------
template<typename VALUE_TYPE>
void const* const TypedProperty<VALUE_TYPE>::StaticUniqueID()
{
	static int s_local = 0;
	return &s_local;
}


//---------------------------------------------------------------------------------------------------------
// NamedProperties
//---------------------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	~NamedProperties();
	NamedProperties() {}

public:
	template<typename T>
	void SetValue( std::string const& keyName, T const& value );

	template<typename T>
	T GetValue( std::string const& keyName, T const& defValue ) const;


public:
	void PopulateFromString( std::string arguments );
	void SetValue( std::string const& keyName, char const* value );
	std::string GetValue( std::string const& keyName, char const* defValue ) const;


private:
	TypedPropertyBase* FindInMap( std::string const& key ) const;
	
private:
	std::map<std::string, TypedPropertyBase*> m_keyValuePairs;
};


//---------------------------------------------------------------------------------------------------------
template<typename T>
void NamedProperties::SetValue( std::string const& keyName, T const& value )
{
	TypedPropertyBase* base = FindInMap( keyName );
	if( base == nullptr )
	{
		TypedProperty<T>* prop = new TypedProperty<T>();
		prop->m_value = value;
		m_keyValuePairs[keyName] = prop;
	}
	else
	{
		if( base->Is<T>() )
		{
			TypedProperty<T>* prop = (TypedProperty<T>*)base;
			prop->m_value = value;
		}
		else
		{
			delete base;
			TypedProperty<T>* prop = new TypedProperty<T>();
			prop->m_value = value;
			m_keyValuePairs[keyName] = prop;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
template<typename T>
T NamedProperties::GetValue( std::string const& keyName, T const& defValue ) const
{
	TypedPropertyBase* base = FindInMap( keyName );
	if( base != nullptr )
	{
		if( base->Is<T>() )
		{
			TypedProperty<T>* prop = (TypedProperty<T>*)base;
			return prop->m_value;
		}
		else
		{
			std::string strValue = base->GetAsString();
			return SetFromText( strValue.c_str(), defValue );
		}
	}
	else
	{
		return defValue;
	}
}