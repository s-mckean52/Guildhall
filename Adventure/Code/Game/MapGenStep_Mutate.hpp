#pragma once
#include "Game/MapGenStep.hpp"

class Map;

class MapGenStep_Mutate : public MapGenStep
{
public:
	explicit MapGenStep_Mutate( const XmlElement& element );

	virtual void RunStepOnce( Map& map ) override;

protected:
	int m_maxNumTilesToChange = 999999;
};