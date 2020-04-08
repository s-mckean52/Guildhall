#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <map>

struct Vec2;
struct Vec3;
struct IntVec2;

typedef std::map<std::string, std::string>::const_iterator MapConstIterator;

class NamedStrings
{
public:
	void			PopulateFromXmlElementAttribute( const XmlElement& element );
	void			PopulateFromString( std::string const& arguments );
	void			SetValue( const std::string& keyName, const std::string& newValue );

	bool			GetValue( const std::string& keyName, bool defaultValue ) const;
	int				GetValue( const std::string& keyName, int defaultValue ) const;
	float			GetValue( const std::string& keyName, float defaultValue ) const;
	double			GetValue( const std::string& keyName, double defaultValue ) const;
	std::string		GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string		GetValue( const std::string& keyName, const char* defaultValue ) const;
	Rgba8			GetValue( const std::string& keyName, const Rgba8& defaultValue ) const;
	Vec2			GetValue( const std::string& keyName, const Vec2& defaultValue ) const;
	Vec3			GetValue( const std::string& keyName, const Vec3& defaultValue ) const;
	IntVec2			GetValue( const std::string& keyName, const IntVec2& defaultValue ) const;

private:
	std::map< std::string, std::string >	m_keyValuePairs;
};