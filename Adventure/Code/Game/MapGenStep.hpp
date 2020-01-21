#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

class TileDefinition;
class Map;

class MapGenStep
{
public:
	explicit MapGenStep( const XmlElement& element );

	void RunStep( Map& map );

public:
	static MapGenStep* CreateMapGenStep( const XmlElement& element );

protected:
	virtual void RunStepOnce( Map& map ) = 0;

protected:
	int				m_iterations	= 1;
	float			m_chanceToRun	= 1.f;
	FloatRange		m_chancePerTile = FloatRange( 1.f );
	TileDefinition*	m_ifTile		= nullptr;
	TileDefinition*	m_setTile		= nullptr;
};