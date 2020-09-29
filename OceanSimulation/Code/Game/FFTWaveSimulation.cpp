#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
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

	m_c.resize(2);
	m_c[0].resize( samples );
	m_c[1].resize( samples );

	m_log2N = static_cast<uint>( std::log2( samples ) );
	m_pi2 = 2.f * PI_VALUE;

	CreateBitReversedIndicies();
	CalculateTForIndicies();

	m_hTilde0Data.resize( m_numSamples * m_numSamples );
	for( int i = 0; i < m_initialSurfacePositions.size(); ++i )
	{	
		int m = i / m_numSamples;
		int n = i - ( m * m_numSamples );

		HTilde0Data hTilde0Data;
		hTilde0Data.m_htilde0		= hTilde0( n, m );
		hTilde0Data.m_htilde0Conj	= std::conj( hTilde0( n, m, true ) );

		m_hTilde0Data[i] = hTilde0Data;
	}
}


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::~FFTWaveSimulation()
{
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::Simulate()
{
	float elapsedTime = static_cast<float>( g_theGame->GetGameClock()->GetTotalElapsedSeconds() );
	for( int positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		int m = positionIndex / m_numSamples;
		int n = positionIndex - ( m * m_numSamples );

		GetHeightAtPosition( n, m, elapsedTime );
	}

	for( uint mIndex = 0; mIndex < m_numSamples; ++mIndex )
	{
		CalculateFFT( m_hTilde, m_hTilde, 1, mIndex * m_numSamples );
		CalculateFFT( m_hTilde_dx, m_hTilde_dx, 1, mIndex * m_numSamples );
		CalculateFFT( m_hTilde_dy, m_hTilde_dy, 1, mIndex * m_numSamples );
	}
	for( uint nIndex = 0; nIndex < m_numSamples; ++nIndex )
	{
		CalculateFFT( m_hTilde, m_hTilde, m_numSamples, nIndex );
		CalculateFFT( m_hTilde_dx, m_hTilde_dx, m_numSamples, nIndex );
		CalculateFFT( m_hTilde_dy, m_hTilde_dy, m_numSamples, nIndex );
	}

	for( uint positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		int m = positionIndex / m_numSamples;
		int n = positionIndex - ( m * m_numSamples );

		float sign = 1.f - ( 2.f * ( ( n + m ) % 2 ) );

		Vec3 translation;
		translation.x = 0.f; //-m_hTilde_dx[positionIndex].real() * sign;
		translation.y = 0.f; //-m_hTilde_dy[positionIndex].real() * sign;
		translation.z = m_hTilde[positionIndex].real() *sign;

		m_surfaceVerts[positionIndex].m_position = m_initialSurfacePositions[positionIndex] + translation;
	}

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
	m_bitReversedIndicies.resize( m_numSamples );
	for( uint index = 0; index < m_numSamples; ++index )
	{
		m_bitReversedIndicies[index] = ReverseBits( index );
	}
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::CalculateTForIndicies()
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
		int dataIndex = m_bitReversedIndicies[sampleIndex] * stride + offset;
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


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::GetHeightAtPosition( int n, int m, float time )
{
	int index = m * m_numSamples + n;

	Vec2 k = GetK( n, m );
	
	m_hTilde[index] = hTilde( n, m, time );
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

