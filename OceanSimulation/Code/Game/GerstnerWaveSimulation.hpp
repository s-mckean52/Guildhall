#pragma once
#include "Game/WaveSimulation.hpp"

//---------------------------------------------------------------------------------------------------------
class GersternWaveSimulation : public WaveSimulation
{
public:
	~GersternWaveSimulation() = default;
	GersternWaveSimulation( Vec2 const& dimensions, uint samples );

	void Simulate() override;

	Vec3 GetSurfacePositionAtTime( Vec3 const& initialPosition );
};