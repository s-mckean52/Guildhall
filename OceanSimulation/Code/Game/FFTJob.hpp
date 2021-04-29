#pragma once
#include "Engine/Core/JobSystem.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include <vector>


class FFTJob : public Job
{
public:
	FFTJob( FFTWaveSimulation* waveSimulation, std::vector<WaveSurfaceVertex> const& data, int stride, int offset );
	~FFTJob();

	void Execute() override;
	void OnCompleteCallback() override;
	void CalculateFFT( std::vector<WaveSurfaceVertex>& data, int stride, int offset );

private:
	FFTWaveSimulation* m_owner = nullptr;

	//Simulation Data
	uint	m_numSamples	= 0;
	uint	m_log2N			= 0;
	uint	which			= 0;

	std::vector<uint> m_bitReversedIndices;
	std::vector<ComplexFloatVector> m_c;
	std::vector<ComplexFloatVector> m_Ts;
	std::vector<WaveSurfaceVertexVector> m_switchArray;

	//FFT Data
	int m_stride = 0;
	int m_offset = 0;
	std::vector<WaveSurfaceVertex> m_data;
};