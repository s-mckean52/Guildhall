#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Core/XmlUtils.hpp"

class Map;

class MapGenStep_FromImage : public MapGenStep
{
public:
	MapGenStep_FromImage( const XmlElement& element );

	virtual void RunStepOnce( Map& map ) override;

private:
	std::string m_imageFilePath = "";
	FloatRange m_alignmentX = FloatRange( 0.f, 1.f );
	FloatRange m_alignmentY = FloatRange( 0.f, 1.f );
};