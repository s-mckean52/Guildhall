#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::FFTWaveSimulation( Vec2 const& dimensions, uint samples )
	: WaveSimulation( dimensions, samples )
{
	int samplesSquared = samples * samples;
	m_hTilde.resize( samplesSquared );
	m_hTilde_dx.resize( samplesSquared );
	m_hTilde_dy.resize( samplesSquared );
	m_slopeX.resize( samplesSquared );
	m_slopeY.resize( samplesSquared );

	m_c.resize(2);
	m_c[0].resize( samples );
	m_c[1].resize( samples );

	m_log2N = static_cast<uint>( std::log2( samples ) );
	m_pi2 = 2.f * PI_VALUE;

	CreateBitReversedIndicies();
	CalculateTForIndices();

	m_hTilde0Data.resize( m_numSamples * m_numSamples );
	m_surfaceVerts.resize( m_numSamples * m_numSamples );
	for( int i = 0; i < m_initialSurfacePositions.size(); ++i )
	{	
		int m = i / m_numSamples;
		int n = i - ( m * m_numSamples );
		
		m_waveSurfaceVerts[i] = WaveSurfaceVertex( n, m, IntVec2( m_numSamples, m_numSamples ), m_dimensions );
	}
}


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::~FFTWaveSimulation()
{
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::Simulate()
{
	float elapsedTime = static_cast<float>( m_simulationClock->GetTotalElapsedSeconds() );
	for( int positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
// 		int m = positionIndex / m_numSamples;
// 		int n = positionIndex - ( m * m_numSamples );
// 
// 		GetHeightAtPosition( n, m, elapsedTime );
		m_waveSurfaceVerts[positionIndex].CalculateValuesAtTime( elapsedTime );
	}

	for( uint mIndex = 0; mIndex < m_numSamples; ++mIndex )
	{
		CalculateFFT( m_waveSurfaceVerts, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde, m_hTilde, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde_dx, m_hTilde_dx, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde_dy, m_hTilde_dy, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_slopeX, m_slopeX, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_slopeY, m_slopeY, 1, mIndex * m_numSamples );
	}
	for( uint nIndex = 0; nIndex < m_numSamples; ++nIndex )
	{
		CalculateFFT( m_waveSurfaceVerts, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde, m_hTilde, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde_dx, m_hTilde_dx, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde_dy, m_hTilde_dy, m_numSamples, nIndex );
// 		CalculateFFT( m_slopeX, m_slopeX, m_numSamples, nIndex );
// 		CalculateFFT( m_slopeY, m_slopeY, m_numSamples, nIndex );
	}

	for( uint positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		int m = positionIndex / m_numSamples;
		int n = positionIndex - ( m * m_numSamples );

		float sign = 1.f - ( 2.f * ( ( n + m ) % 2 ) );

		Vec3 translation;
		translation.x = -m_hTilde_dx[positionIndex].real() * sign;
		translation.y = -m_hTilde_dy[positionIndex].real() * sign;
		translation.z = m_hTilde[positionIndex].real() *sign;

		Vec3 normal;
		normal.x = m_slopeX[positionIndex].real() * sign;
		normal.y = m_slopeY[positionIndex].real() * sign;
		normal.z = 0.f;

		normal = Vec3::UNIT_POSITIVE_Y - normal / sqrtf( 1.f + DotProduct3D( normal, normal ) );
		//normal.Normalize();

		Vertex_PCUTBN& currentVert = m_surfaceVerts[positionIndex];
		currentVert.m_position = m_initialSurfacePositions[positionIndex] + translation;
		currentVert.m_normal = normal;
	}

	MeshGenerateTangents( m_surfaceVerts );
	m_surfaceMesh->UpdateVerticies( static_cast<uint>( m_surfaceVerts.size() ), &m_surfaceVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
uint FFTWaveSimulation::ReverseBits( uint value )
{
	//---------------------------------------------------------------------------------------------------------
	//http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
	unsigned int v = value;		// input bits to be reversed
	unsigned int r = v;			// r will be reversed bits of v; first get LSB of v
	int s = m_log2N - 1;		// extra shift needed at end

	for( v >>= 1; v; v >>= 1 )
	{
		r <<= 1;
		r |= v & 1;
		s--;
	}
	r <<= s;					// shift when v's highest bits are zero
	r &= m_numSamples - 1;		// Zero out bits that are beyond number of samples
	//---------------------------------------------------------------------------------------------------------
	return r;
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::CreateBitReversedIndicies()
{
	GUARANTEE_OR_DIE( IsValidNumSamples( m_numSamples ), "Number of samples needs to be a power of 2" );
	m_bitReversedIndices.resize( m_numSamples );
	for( uint index = 0; index < m_numSamples; ++index )
	{
		m_bitReversedIndices[index] = ReverseBits( index );
	}
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::CalculateTForIndices()
{
	uint pow2 = 1;
	m_Ts.resize( m_log2N );
	for( uint i = 0; i < m_log2N; ++i )
	{
		m_Ts[i].resize( pow2 );
		for( uint j = 0; j < pow2; ++j )
		{
			m_Ts[i][j] = GetTCalculation( j, pow2 * 2 );
		}
		pow2 *= 2;
	}
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat FFTWaveSimulation::GetTCalculation( uint x, uint samplesAtDimension )
{
	float value = ( m_pi2 * x ) / samplesAtDimension;
	return ComplexFloat( cos( value ), sin( value ) );
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::CalculateFFT( std::vector<ComplexFloat>& data_in, std::vector<ComplexFloat>& data_out, int stride, int offset )
{
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = m_bitReversedIndices[sampleIndex] * stride + offset;
		m_c[which][sampleIndex] = data_in[ dataIndex ];
	}

	int w_ = 0;
	int numLoops = m_numSamples >> 1;
	int currentIterationSize = 2;
	int lastIterationSize = 1;

	for( uint i = 0; i < m_log2N; ++i )
	{
		which ^= 1;
		for( int j = 0; j < numLoops; ++j )
		{
			for( int k = 0; k < lastIterationSize; ++k )
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				m_c[which][jSizePlusK] = m_c[which^1][jSizePlusK] + m_c[which^1][jSizePlusK + lastIterationSize] * m_Ts[w_][k];
			}

			for( int k = lastIterationSize; k < currentIterationSize; ++k )
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				m_c[which][jSizePlusK] = m_c[which ^ 1][jSizePlusK - lastIterationSize] - m_c[which ^ 1][jSizePlusK] * m_Ts[w_][k - lastIterationSize];
			}
		}
		numLoops	>>= 1;
		currentIterationSize		<<= 1;
		lastIterationSize	<<= 1;
		++w_;
	}

	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = sampleIndex * stride + offset;
		data_out[dataIndex] = m_c[which][sampleIndex];
	}
}


void FFTWaveSimulation::CalculateFFT( std::vector<WaveSurfaceVertex>& data, int stride, int offset )
{
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = m_bitReversedIndices[sampleIndex] * stride + offset;
		m_c[which][sampleIndex] = data[ dataIndex ];
	}

	int w_ = 0;
	int numLoops = m_numSamples >> 1;
	int currentIterationSize = 2;
	int lastIterationSize = 1;

	for( uint i = 0; i < m_log2N; ++i )
	{
		which ^= 1;
		for( int j = 0; j < numLoops; ++j )
		{
			for( int k = 0; k < lastIterationSize; ++k )
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				m_c[which][jSizePlusK] = m_c[which^1][jSizePlusK] + m_c[which^1][jSizePlusK + lastIterationSize] * m_Ts[w_][k];
			}

			for( int k = lastIterationSize; k < currentIterationSize; ++k )
			{
				int jSizePlusK = ( j * currentIterationSize ) + k;
				m_c[which][jSizePlusK] = m_c[which ^ 1][jSizePlusK - lastIterationSize] - m_c[which ^ 1][jSizePlusK] * m_Ts[w_][k - lastIterationSize];
			}
		}
		numLoops	>>= 1;
		currentIterationSize		<<= 1;
		lastIterationSize	<<= 1;
		++w_;
	}

	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = sampleIndex * stride + offset;
		data_out[dataIndex] = m_c[which][sampleIndex];
	}
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::GetHeightAtPosition( int n, int m, float time )
{
	int index = m * m_numSamples + n;

	Vec2 k = GetK( n, m );
	
	m_hTilde[index] = hTilde( n, m, time );
	m_slopeX[index] = m_hTilde[index] * ComplexFloat( 0.f, k.x );
	m_slopeY[index] = m_hTilde[index] * ComplexFloat( 0.f, k.y );
	if( k.GetLength() < 0.000001f )
	{
		m_hTilde_dx[index] = ComplexFloat( 0.f, 0.f );
		m_hTilde_dy[index] = ComplexFloat( 0.f, 0.f );
	}
	else
	{
		m_hTilde_dx[index] = m_hTilde[index] * ComplexFloat( 0.f, -k.x / k.GetLength() );
		m_hTilde_dy[index] = m_hTilde[index] * ComplexFloat( 0.f, -k.y / k.GetLength() );
	}
}

