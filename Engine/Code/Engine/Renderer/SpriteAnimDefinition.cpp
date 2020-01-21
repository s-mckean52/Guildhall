#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

//---------------------------------------------------------------------------------------------------------
SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlayBackType playbackType )
	: m_spriteSheet( sheet )
	, m_startSpriteIndex( startSpriteIndex )
	, m_endSpriteIndex( endSpriteIndex )
	, m_durationSeconds( durationSeconds )
	, m_playbackType( playbackType )
{
}


//---------------------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	switch( m_playbackType )
	{
	case SpriteAnimPlayBackType::ONCE:
		return GetSpriteDefAtTimeForPlaybackTypeOnce( seconds );

	case SpriteAnimPlayBackType::LOOP:
		return GetSpriteDefAtTimeForPlaybackTypeLoop( seconds );

	case SpriteAnimPlayBackType::PINGPONG:
		return GetSpriteDefAtTimeForPlaybackTypePingPong( seconds );

	default:
		ERROR_AND_DIE( Stringf( "Sprite Definition has an unsupported playback Type #%i", m_playbackType ) );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTimeForPlaybackTypeOnce( float seconds ) const
{
	int numFrames = 1 + ( m_endSpriteIndex - m_startSpriteIndex );
	float secondsPerFrame = m_durationSeconds / static_cast<float>( numFrames );
	int frameNum = RoundDownToInt( seconds / secondsPerFrame );

	int currentFrame = GetClamp( frameNum, m_startSpriteIndex, numFrames - 1 );
	return m_spriteSheet.GetSpriteDefinition( currentFrame );
}


//---------------------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTimeForPlaybackTypeLoop( float seconds ) const
{
	int numFrames = 1 + ( m_endSpriteIndex - m_startSpriteIndex );
	float secondsPerFrame = m_durationSeconds / static_cast<float>( numFrames );
	int frameNum =  RoundDownToInt( seconds / secondsPerFrame);

	int currentFrame = PositiveMod( frameNum, numFrames );
	return m_spriteSheet.GetSpriteDefinition( currentFrame );
}


//---------------------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTimeForPlaybackTypePingPong( float seconds ) const
{
	int numFrames = 2 * ( m_endSpriteIndex - m_startSpriteIndex );
	float secondsPerFrame = m_durationSeconds / static_cast<float>(numFrames);
	int frameNum =  RoundDownToInt( seconds / secondsPerFrame );

	int currentFrame = PositiveMod( frameNum, numFrames );

	if( currentFrame <= ( numFrames * 0.5f ) )
	{
		return m_spriteSheet.GetSpriteDefinition( m_startSpriteIndex + currentFrame );
	}
	else
	{
		return m_spriteSheet.GetSpriteDefinition( ( numFrames - currentFrame ) + m_startSpriteIndex );
	}
}
