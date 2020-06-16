#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"


//---------------------------------------------------------------------------------------------------------
int ParseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	int value = defaultValue;
	if( attributeValueText )
	{
		value = atoi( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
char ParseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	char value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText[ 0 ];
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
bool ParseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	bool value = defaultValue;
	if( attributeValueText )
	{
		if( _stricmp( attributeValueText, "0" ) == 0 )
			value = false;
		else if( _stricmp( attributeValueText, "1" ) == 0 )
			value = true;


		if( _stricmp( attributeValueText, "F" ) == 0 )
			value = false;
		else if( _stricmp( attributeValueText, "T" ) == 0 )
			value = true;


		if( _stricmp( attributeValueText, "false" ) == 0 )
			value = false;
		else if( _stricmp( attributeValueText, "true" ) == 0 )
			value = true;
		else
			ERROR_AND_DIE( "Invalid bool XML parse" );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
float ParseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	float value = defaultValue;
	if( attributeValueText )
	{
		value = static_cast<float>( atof( attributeValueText ) );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Rgba8 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Rgba8& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Rgba8 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Vec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Vec2 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
IntVec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntVec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntVec2 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const std::string& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	std::string value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const char* defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	std::string value = defaultValue;
	if( attributeValueText )
	{
		value = attributeValueText;
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Strings ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Strings& defaultValue, char delimiter )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Strings value = defaultValue;
	if( attributeValueText )
	{
		value = SplitStringOnDelimiter( attributeValueText, delimiter );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
IntRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntRange& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntRange value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
FloatRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const FloatRange& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	FloatRange value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
AABB2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const AABB2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	AABB2 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
Vec3 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec3& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Vec3 value = defaultValue;
	if( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//---------------------------------------------------------------------------------------------------------
double ParseXmlAttribute( const XmlElement& element, const char* attributeName, double defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	double value = defaultValue;
	if( attributeValueText )
	{
		value = atof( attributeValueText );
	}
	return value;
}
