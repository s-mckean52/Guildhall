#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntRange.hpp"

class Map;

class MapGenStep_Worms : public MapGenStep
{
public:
	explicit MapGenStep_Worms( const XmlElement& element );

	virtual void RunStepOnce( Map& map ) override;

protected:
	IntRange m_wormLength	= IntRange( 12 );
	IntRange m_numWorms		= IntRange( 1 );
};