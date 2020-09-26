#pragma once
#include "Game/WaveSimulation.hpp"

struct WavePoint
{
	std::complex<float>	m_height;
	Vec2				m_position	= Vec2::ZERO;
};


class DFTWaveSimulation : public WaveSimulation
{
public:
	DFTWaveSimulation( Vec2 const& dimensions, uint samples );
	~DFTWaveSimulation();

	void Simulate() override;

	WavePoint			GetHeightAtPosition( Vec2 const& initialPosition, float time );
};