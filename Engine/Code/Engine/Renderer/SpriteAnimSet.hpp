#pragma once
#include <string>
#include <map>

class SpriteAnimDefinition;
class SpriteDefinition;

class SpriteAnimSet
{
public:
	SpriteAnimSet( std::map<std::string, SpriteAnimDefinition*> const& spriteAnimMap );

	SpriteDefinition const* GetSpriteAnimAtTime( std::string const& animState, float seconds ) const;

private:
	std::map<std::string, SpriteAnimDefinition*> m_spriteAnimMap;
};