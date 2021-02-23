#include "Game/FFTJob.hpp"


//---------------------------------------------------------------------------------------------------------
FFTJob::FFTJob( FFTWaveSimulation* waveSimulation, std::vector<WaveSurfaceVertex> const& data, int stride, int offset )
	: Job()
{
	m_owner = waveSimulation;

	m_numSamples = waveSimulation->m_numSamples;
	m_log2N = waveSimulation->m_log2N;
	
	m_c.resize(2);
	m_c[0].resize( m_numSamples );
	m_c[1].resize( m_numSamples );

	m_switchArray.resize(2);
	m_switchArray[0].resize( m_numSamples );
	m_switchArray[1].resize( m_numSamples );

	m_bitReversedIndices = waveSimulation->m_bitReversedIndices;
	m_Ts = waveSimulation->m_Ts;

	m_stride = stride;
	m_offset = offset;
	m_data = data;
}


//---------------------------------------------------------------------------------------------------------
FFTJob::~FFTJob()
{
	m_owner = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void FFTJob::Execute()
{
	CalculateFFT( m_data, m_stride, m_offset );
}

//---------------------------------------------------------------------------------------------------------
void FFTJob::OnCompleteCallback()
{
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = sampleIndex * m_stride + m_offset;
		WaveSurfaceVertex& vertexDataTo = m_owner->m_waveSurfaceVerts[dataIndex];
		WaveSurfaceVertex& vertexDataFrom = m_switchArray[which][sampleIndex];
		vertexDataTo.m_hTilde = vertexDataFrom.m_hTilde;
		vertexDataTo.m_position[0] = vertexDataFrom.m_position[0];
		vertexDataTo.m_position[1] = vertexDataFrom.m_position[1];
		vertexDataTo.m_surfaceSlope[0] = vertexDataFrom.m_surfaceSlope[0];
		vertexDataTo.m_surfaceSlope[1] = vertexDataFrom.m_surfaceSlope[1];
	}
}


//---------------------------------------------------------------------------------------------------------
void FFTJob::CalculateFFT( std::vector<WaveSurfaceVertex>& data, int stride, int offset )
{
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = m_bitReversedIndices[sampleIndex] * stride + offset;
		m_switchArray[which][sampleIndex] = data[ dataIndex ];
	}

	int w_ = 0;
	int numLoops = m_numSamples >> 1;
	int currentIterationSize = 2;
	int lastIterationSize = 1;

	for( uint i = 0; i < m_log2N; ++i )//512 = 9 loops
	{
		which ^= 1;
		for( int j = 0; j < numLoops; ++j )//256, 128, 64, 32, 16, 8, 4, 2, 1 loops
		{
			for( int k = 0; k < lastIterationSize; ++k )//1, 2, 4, 8, 16, 32, ...
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				WaveSurfaceVertex& waveStorage = m_switchArray[which][jSizePlusK];
				WaveSurfaceVertex& waveToModify = m_switchArray[which^1][jSizePlusK];
				WaveSurfaceVertex& lastWaveToModify = m_switchArray[which^1][jSizePlusK + lastIterationSize];
				ComplexFloat currentTValue = m_Ts[w_][k];

				waveStorage.m_hTilde = waveToModify.m_hTilde + lastWaveToModify.m_hTilde * currentTValue;
				waveStorage.m_position[0] = waveToModify.m_position[0] + lastWaveToModify.m_position[0] * currentTValue;
				waveStorage.m_position[1] = waveToModify.m_position[1] + lastWaveToModify.m_position[1] * currentTValue;
				waveStorage.m_surfaceSlope[0] = waveToModify.m_surfaceSlope[0] + lastWaveToModify.m_surfaceSlope[0] * currentTValue;
				waveStorage.m_surfaceSlope[1] = waveToModify.m_surfaceSlope[1] + lastWaveToModify.m_surfaceSlope[1] * currentTValue;
			}

			for( int k = lastIterationSize; k < currentIterationSize; ++k )
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				WaveSurfaceVertex& waveStorage = m_switchArray[which][jSizePlusK];
				WaveSurfaceVertex& waveToModify = m_switchArray[which^1][jSizePlusK];
				WaveSurfaceVertex& lastWaveToModify = m_switchArray[which^1][jSizePlusK - lastIterationSize];
				ComplexFloat currentTValue = m_Ts[w_][k - lastIterationSize];

				waveStorage.m_hTilde = lastWaveToModify.m_hTilde - waveToModify.m_hTilde * currentTValue;
				waveStorage.m_position[0] = lastWaveToModify.m_position[0] - waveToModify.m_position[0] * currentTValue;
				waveStorage.m_position[1] = lastWaveToModify.m_position[1] - waveToModify.m_position[1] * currentTValue;
				waveStorage.m_surfaceSlope[0] = lastWaveToModify.m_surfaceSlope[0] - waveToModify.m_surfaceSlope[0] * currentTValue;
				waveStorage.m_surfaceSlope[1] = lastWaveToModify.m_surfaceSlope[1] - waveToModify.m_surfaceSlope[1] * currentTValue;
			}
		}
		numLoops				>>= 1;
		currentIterationSize	<<= 1;
		lastIterationSize		<<= 1;
		++w_;
	}
}