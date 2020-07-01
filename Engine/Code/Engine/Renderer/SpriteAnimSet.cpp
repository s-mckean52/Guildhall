#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
SpriteAnimSet::SpriteAnimSet( std::map<std::string, SpriteAnimDefinition*> const& spriteAnimMap )
{
	m_spriteAnimMap = spriteAnimMap;
}


//---------------------------------------------------------------------------------------------------------
SpriteDefinition const* SpriteAnimSet::GetSpriteAnimAtTime( std::string const& animState, float seconds ) const
{
	auto animDefIter = m_spriteAnimMap.find( animState );
	if( animDefIter != m_spriteAnimMap.end() )
	{
		SpriteAnimDefinition* animDef = animDefIter->second;
		SpriteDefinition const& spriteDef = animDef->GetSpriteDefAtTime( seconds );
		return &spriteDef;
	}
	return nullptr;
}
