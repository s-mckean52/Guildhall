#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include <string>

typedef tinyxml2::XMLElement	XmlElement;
typedef tinyxml2::XMLAttribute	XmlAttribute;
typedef tinyxml2::XMLDocument	XmlDocument;	

struct Rgba8;
struct Vec2;
struct Vec3;
struct IntVec2;
struct IntRange;
struct FloatRange;
struct AABB2;


//---------------------------------------------------------------------------------------------------------
int ParseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue );
char ParseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue );
bool ParseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue );
float ParseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue );
double ParseXmlAttribute( const XmlElement& element, const char* attributeName, double defaultValue );
Rgba8 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Rgba8& defaultValue );
Vec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec2& defaultValue );
Vec3 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec3& defaultValue );
IntVec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntVec2& defaultValue );
IntRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const FloatRange& defaultValue );
AABB2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const AABB2& defaultValue );
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const std::string& defaultValue );
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const char* defaultValue );
Strings ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Strings& defaultValue, char delimiter = ',' );