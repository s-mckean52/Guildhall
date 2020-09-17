#pragma once
#include "Game/WaveSimulation.hpp"

class FFTWaveSimulation : public WaveSimulation
{
public:
	~FFTWaveSimulation() = default;
	FFTWaveSimulation( Vec2 const& dimensions, uint samples );

	void Simulate() override;

	float CalculateFFTHeight( Vec2 const& initialPosition, Vec2 const& k, float timeElapsed );
	float PhillipsEquation( Vec2 const& waveDirection );
};