#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"

enum class SpriteAnimPlayBackType
{
	ONCE,
	LOOP,
	PINGPONG,
};


//---------------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	explicit SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlayBackType playbackType = SpriteAnimPlayBackType::LOOP );

	const SpriteDefinition& GetSpriteDefAtTime( float seconds ) const;

private:
	const SpriteDefinition& GetSpriteDefAtTimeForPlaybackTypeOnce( float seconds ) const;
	const SpriteDefinition& GetSpriteDefAtTimeForPlaybackTypeLoop( float seconds ) const;
	const SpriteDefinition& GetSpriteDefAtTimeForPlaybackTypePingPong( float seconds ) const;

private:
	const SpriteSheet&		m_spriteSheet;
	int						m_startSpriteIndex = -1;
	int						m_endSpriteIndex = -1;
	float					m_durationSeconds = 1.f;
	SpriteAnimPlayBackType	m_playbackType = SpriteAnimPlayBackType::LOOP;
};
