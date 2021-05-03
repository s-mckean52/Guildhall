#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/IWave.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/FFTJob.hpp"


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::FFTWaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed )
	: WaveSimulation( dimensions, samples, windSpeed )
{
	InitializeValues();
}



//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::FFTWaveSimulation( XmlElement const& element )
	: WaveSimulation( element )
{
	InitializeValues();
}


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::FFTWaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed, Vec2 const& windDir, float aConstant, float waveSuppression )
	: WaveSimulation( dimensions, samples, windSpeed )
{
	m_windDirection = windDir;
	m_A = aConstant;
	m_waveSuppression = waveSuppression;


	InitializeValues();
}


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::~FFTWaveSimulation()
{
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::Simulate()
{
	//m_simulateTimer.StartTimer();
	float elapsedTime = static_cast<float>( m_simulationClock->GetTotalElapsedSeconds() );
	float deltaSeconds = static_cast<float>( m_simulationClock->GetLastDeltaSeconds() );

	//m_pointCalculationTimer.StartTimer();
	for( int positionIndex = 0; positionIndex < m_waveSurfaceVerts.size(); ++positionIndex )
	{
// 		int m = positionIndex / m_numSamples;
// 		int n = positionIndex - ( m * m_numSamples );
// 
// 		GetHeightAtPosition( n, m, elapsedTime );
		m_waveSurfaceVerts[positionIndex].CalculateValuesAtTime( elapsedTime );
	}
	//m_pointCalculationTimer.StopTimer();

	//m_fftTimer.StartTimer();
	for( uint mIndex = 0; mIndex < m_numSamples; ++mIndex )
	{
		//g_theJobSystem->PostJob( new FFTJob( this, m_waveSurfaceVerts, 1, mIndex * m_numSamples ) );

		CalculateFFT( m_waveSurfaceVerts, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde, m_hTilde, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde_dx, m_hTilde_dx, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_hTilde_dy, m_hTilde_dy, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_slopeX, m_slopeX, 1, mIndex * m_numSamples );
// 		CalculateFFT( m_slopeY, m_slopeY, 1, mIndex * m_numSamples );
	}
	for( uint nIndex = 0; nIndex < m_numSamples; ++nIndex )
	{
		//g_theJobSystem->PostJob( new FFTJob( this, m_waveSurfaceVerts, m_numSamples, nIndex ) );
		CalculateFFT( m_waveSurfaceVerts, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde, m_hTilde, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde_dx, m_hTilde_dx, m_numSamples, nIndex );
// 		CalculateFFT( m_hTilde_dy, m_hTilde_dy, m_numSamples, nIndex );
// 		CalculateFFT( m_slopeX, m_slopeX, m_numSamples, nIndex );
// 		CalculateFFT( m_slopeY, m_slopeY, m_numSamples, nIndex );
	}
	//m_fftTimer.StopTimer();
	
// 	g_theJobSystem->WaitForAllJobs();
// 	g_theJobSystem->ClaimAndDeleteAllCompletedJobs();

	for( uint positionIndex = 0; positionIndex < m_waveSurfaceVerts.size(); ++positionIndex )
	{
// 		int m = positionIndex / m_numSamples;
// 		int n = positionIndex - ( m * m_numSamples );
// 
// 		float sign = 1.f - ( 2.f * ( ( n + m ) % 2 ) );
// 
// 		Vec3 translation;
// 		translation.x = -m_hTilde_dx[positionIndex].real() * sign;
// 		translation.y = -m_hTilde_dy[positionIndex].real() * sign;
// 		translation.z = m_hTilde[positionIndex].real() *sign;
// 
// 		Vec3 normal;
// 		normal.x = m_slopeX[positionIndex].real() * sign;
// 		normal.y = m_slopeY[positionIndex].real() * sign;
// 		normal.z = 0.f;
// 
// 		normal = ( Vec3::UNIT_POSITIVE_Z - normal ) / sqrtf( 1.f + DotProduct3D( normal, normal ) );
// 		//normal.Normalize();
// 
// 		Vertex_PCUTBN& currentVert = m_surfaceVerts[positionIndex];
// 		currentVert.m_position = m_initialSurfacePositions[positionIndex] + translation;
// 		currentVert.m_normal = normal;

		//Vertex_PCUTBN& currentVert = m_surfaceVerts[positionIndex];
		//m_waveSurfaceVerts[positionIndex].SetVertexPositionAndNormal( currentVert );
		WaveSurfaceVertex& currentVert = m_waveSurfaceVerts[positionIndex];
		float sign = 1.f - (2.f * PositiveMod(currentVert.m_translatedCoord.x + currentVert.m_translatedCoord.y, 2));

		currentVert.m_height = currentVert.m_hTilde.real() * sign;
	}

	if( m_isIWaveEnabled )
	{
		m_iWave->Update( deltaSeconds );
	}
	for( uint positionIndex = 0; positionIndex < m_surfaceVerts.size(); ++positionIndex )
	{
		int mPlus1 = positionIndex / ( m_numSamples + 1 );
		int nPlus1 = positionIndex - mPlus1 * ( m_numSamples + 1 );


		int numSamplesAsInt = static_cast<int>( m_numSamples );
		Vertex_OCEAN& currentVert = m_surfaceVerts[positionIndex];
		if( mPlus1 != numSamplesAsInt && nPlus1 != numSamplesAsInt )
		{
			int waveIndex = mPlus1 * m_numSamples + nPlus1;
			m_waveSurfaceVerts[waveIndex].SetVertexPositionAndNormal( currentVert );
			currentVert.m_jacobian.x = CalculateJacobianForVertexAtIndex( waveIndex );
		}
		else if( mPlus1 == numSamplesAsInt && nPlus1 == numSamplesAsInt )
		{
			m_waveSurfaceVerts[ 0 ].SetVertexPositionAndNormal( currentVert, true );
			currentVert.m_position += m_initialSurfacePositions[positionIndex];
			currentVert.m_jacobian.x = CalculateJacobianForVertexAtIndex( 0 );
		}
		else if( nPlus1 == numSamplesAsInt )
		{
			int firstIndexInRow = mPlus1 * m_numSamples;
			m_waveSurfaceVerts[ firstIndexInRow ].SetVertexPositionAndNormal( currentVert, true );
			currentVert.m_position += m_initialSurfacePositions[positionIndex];
			currentVert.m_jacobian.x = CalculateJacobianForVertexAtIndex( firstIndexInRow );
		}
		else if( mPlus1 == numSamplesAsInt )
		{
			int firstIndexInCol = nPlus1;
			m_waveSurfaceVerts[firstIndexInCol].SetVertexPositionAndNormal( currentVert, true );
			currentVert.m_position += m_initialSurfacePositions[positionIndex];
			currentVert.m_jacobian.x = CalculateJacobianForVertexAtIndex( firstIndexInCol );
		}
	}

	//MeshGenerateTangents( m_surfaceVerts );
// 	for( int i = 0; i < m_surfaceVerts.size(); ++i )
// 	{
// 		Vertex_PCUTBN const& currentVert = m_surfaceVerts[i];
// 		DebugAddWorldArrow( currentVert.m_position, currentVert.m_position + currentVert.m_tangent, Rgba8::RED, 0.f, DEBUG_RENDER_XRAY );
// 		DebugAddWorldArrow( currentVert.m_position, currentVert.m_position + currentVert.m_bitangent, Rgba8::GREEN, 0.f, DEBUG_RENDER_XRAY );
// 		DebugAddWorldArrow( currentVert.m_position, currentVert.m_position + currentVert.m_normal, Rgba8::BLUE, 0.f, DEBUG_RENDER_XRAY );
// 	}

	if( m_isIWaveEnabled )
	{
		RecalculateNormals();
	}
	m_surfaceMesh->UpdateVerticies( static_cast<uint>( m_surfaceVerts.size() ), &m_surfaceVerts[0] );
	//m_simulateTimer.StopTimer();
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::InitializeValues()
{
	m_iWave = new IWave( this, m_dimensions, m_numSamples, m_numSamples );

	int samplesSquared = m_numSamples * m_numSamples;
	m_hTilde.resize( samplesSquared );
	m_hTilde_dx.resize( samplesSquared );
	m_hTilde_dy.resize( samplesSquared );
	m_slopeX.resize( samplesSquared );
	m_slopeY.resize( samplesSquared );

	m_c.resize(2);
	m_c[0].resize( m_numSamples );
	m_c[1].resize( m_numSamples );

	m_switchArray.resize(2);
	m_switchArray[0].resize( m_numSamples );
	m_switchArray[1].resize( m_numSamples );

	m_log2N = static_cast<uint>( std::log2( m_numSamples ) );
	m_pi2 = 2.f * PI_VALUE;

	CreateBitReversedIndicies();
	CalculateTForIndices();

	m_hTilde0Data.resize( samplesSquared );
	m_waveSurfaceVerts.resize( samplesSquared );
	for( int i = 0; i < samplesSquared; ++i )
	{	
		int m = i / m_numSamples;
		int n = i - ( m * m_numSamples );
		
// 		m_hTilde0Data[i].m_htilde0 = hTilde0(n, m);
// 		m_hTilde0Data[i].m_htilde0Conj = std::conj( hTilde0(n, m, true) );

		m_waveSurfaceVerts[i] = WaveSurfaceVertex( this, n, m, IntVec2( m_numSamples, m_numSamples ), m_dimensions );
	}
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
	int numLoops = m_numSamples >> 1; //n / 2
	int currentIterationSize = 2;
	int lastIterationSize = 1;

	//for( int s = 1; s <= m_log2N; s++ )
	//{
	//		m = currentIterationSize;
	//		m2 = lastIterationSize;
	//		m = 1 << s;
	//		m2 = m >> s;
	//}
	for( uint i = 0; i < m_log2N; ++i )
	{
		which ^= 1;
		for( int j = 0; j < numLoops; ++j )
		{
			//for( int k = j; k < m_numSamples; k+=m )
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
		numLoops				>>= 1;
		currentIterationSize	<<= 1;
		lastIterationSize		<<= 1;
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
	m_simulateTimer.Start();
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = m_bitReversedIndices[sampleIndex] * stride + offset;
		m_switchArray[which][sampleIndex] = data[ dataIndex ];
	}
	m_simulateTimer.Stop();

	int w_ = 0;
	int numLoops = m_numSamples >> 1;
	int currentIterationSize = 2;
	int lastIterationSize = 1;

	m_pointCalculationTimer.Start();
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
	m_pointCalculationTimer.Stop();

	m_fftTimer.Start();
	for( uint sampleIndex = 0; sampleIndex < m_numSamples; ++sampleIndex )
	{
		int dataIndex = sampleIndex * stride + offset;
		WaveSurfaceVertex& vertexDataTo = data[dataIndex];
		WaveSurfaceVertex& vertexDataFrom = m_switchArray[which][sampleIndex];
		vertexDataTo.m_hTilde = vertexDataFrom.m_hTilde;
		vertexDataTo.m_position[0] = vertexDataFrom.m_position[0];
		vertexDataTo.m_position[1] = vertexDataFrom.m_position[1];
		vertexDataTo.m_surfaceSlope[0] = vertexDataFrom.m_surfaceSlope[0];
		vertexDataTo.m_surfaceSlope[1] = vertexDataFrom.m_surfaceSlope[1];
	}
	m_fftTimer.Stop();
}


//---------------------------------------------------------------------------------------------------------
float FFTWaveSimulation::CalculateJacobianForVertexAtIndex( int vertIndex )
{
	int totalVerts = m_numSamples * m_numSamples;
	int northIndex = vertIndex + m_numSamples;
	int southIndex = vertIndex - m_numSamples;
	int eastIndex = vertIndex + 1;
	int westIndex = vertIndex - 1;
	if( southIndex < 0 )
	{
		southIndex = totalVerts - -southIndex;
	}
	if( northIndex > totalVerts - 1)
	{
		northIndex -= totalVerts - 1;
	}
	if( eastIndex > totalVerts - 1)
	{
		eastIndex = 0;
	}
	if( westIndex < 0 )
	{
		westIndex = totalVerts - 1;
	}
	WaveSurfaceVertex& northVert = m_waveSurfaceVerts[northIndex];
	WaveSurfaceVertex& southVert = m_waveSurfaceVerts[southIndex];
	WaveSurfaceVertex& eastVert = m_waveSurfaceVerts[eastIndex];
	WaveSurfaceVertex& westVert = m_waveSurfaceVerts[westIndex];

	Vec2 dDx = ( eastVert.GetHorizontalTranslation() + westVert.GetHorizontalTranslation() ) * 0.5f;
	Vec2 dDy = ( northVert.GetHorizontalTranslation() + southVert.GetHorizontalTranslation() ) * 0.5f;

	return ( ( 1.f + dDx.x ) * ( 1.f + dDy.y ) ) - ( dDx.y * dDy.x );
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::GetHeightAtPosition( int n, int m, float time )
{
	int index = m * m_numSamples + n;

	Vec2 k = GetK( n, m );
	float kLength = k.GetLength();

	m_hTilde[index] = hTilde( n, m, time );
	m_slopeX[index] = m_hTilde[index] * ComplexFloat( 0.f, k.x );
	m_slopeY[index] = m_hTilde[index] * ComplexFloat( 0.f, k.y );
	if( kLength < 0.000001f )
	{
		m_hTilde_dx[index] = ComplexFloat( 0.f, 0.f );
		m_hTilde_dy[index] = ComplexFloat( 0.f, 0.f );
	}
	else
	{
		m_hTilde_dx[index] = m_hTilde[index] * ComplexFloat( 0.f, -k.x / kLength );
		m_hTilde_dy[index] = m_hTilde[index] * ComplexFloat( 0.f, -k.y / kLength );
	}
}


//---------------------------------------------------------------------------------------------------------
WaveSurfaceVertex* FFTWaveSimulation::GetWaveVertAtIndex( int index )
{
	return &m_waveSurfaceVerts[index];
}

