#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/IWave.hpp"
#include "Game/GameCommon.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/WaterObject.hpp"
#include <complex>
#include <corecrt_math.h>

//---------------------------------------------------------------------------------------------------------
IWave::IWave( FFTWaveSimulation* owner, Vec2 const& dimensions, int xSamples, int ySamples )
	:m_owner( owner )
{
	m_deltas.x = dimensions.x / xSamples;
	m_deltas.y = dimensions.y / ySamples;

	m_gridDimensions = IntVec2( xSamples, ySamples );
	int numPoints = xSamples * ySamples;
	m_samplePoints.resize( numPoints );

	m_sources.resize( numPoints );
	m_obstructions.resize( numPoints );
	for( int i = 0; i < m_obstructions.size(); ++i )
	{
		m_obstructions[i] = 1.f;
	}

	CreateKernel( 6 );
}


//---------------------------------------------------------------------------------------------------------
IWave::~IWave()
{
}


//---------------------------------------------------------------------------------------------------------
void IWave::Update( float deltaSeconds )
{
	//UpdateObstructions()

	//Apply Source, obstruction
	for( int index = 0; index < m_samplePoints.size(); ++index )
	{
		IWaveData& currentPoint = m_samplePoints[index];
		currentPoint.m_height += m_sources[index];
		currentPoint.m_height *= m_obstructions[index];
		GetHeightFromSimulation( index );
	}

	CalculateNewHeights( deltaSeconds );
	ApplyHeightsToSimulation();
	ClearSources();
}


//---------------------------------------------------------------------------------------------------------
void IWave::GetHeightFromSimulation( int index )
{
	WaveSurfaceVertex& currentWaveVert = m_owner->GetWaveVertAtIndex( index );
	m_samplePoints[index].m_height -= currentWaveVert.m_height * ( 1.f - m_obstructions[index] );
}


//---------------------------------------------------------------------------------------------------------
void IWave::ApplyHeightsToSimulation()
{
	for( int index = 0; index < m_samplePoints.size(); ++index )
	{
		WaveSurfaceVertex& currentWaveVert = m_owner->GetWaveVertAtIndex( index );
		currentWaveVert.m_height += m_samplePoints[index].m_height;
	}
}


//---------------------------------------------------------------------------------------------------------
void IWave::AddSource( int index, float value )
{
	m_sources[index] = value;
}


//---------------------------------------------------------------------------------------------------------
void IWave::ClearSources()
{
	for( int index = 0; index < m_sources.size(); ++index )
	{
		m_sources[index] = 0.f;
		m_obstructions[index] = 1.f;
	}
}

//---------------------------------------------------------------------------------------------------------
void IWave::CreateKernel( int kernelSize )
{
	m_kernelSize = kernelSize;
	m_kernelDimension = ( 2 * kernelSize ) + 1;
	int numKernelPoints = m_kernelDimension * m_kernelDimension;
	m_kernelValuesLookUp.resize( numKernelPoints );

	float g0 = 0.f;
	float deltaQ = 0.001f;
	for( int n = 0; n < 10000; ++n )
	{
		float q = n * deltaQ;
		float q2 = q * q;
		g0 += q2 * std::exp( -q2 );
	}

	for( int index = 0; index < numKernelPoints; ++index )
	{
		int yPos = index / m_kernelDimension;
		int xPos = index - ( yPos * m_kernelDimension );
		yPos -= m_kernelSize;
		xPos -= m_kernelSize;


		float g = 0.f;
		float r = sqrtf( ( xPos * xPos ) + ( yPos * yPos ) );
		for( int n = 0; n < 10000; ++n )
		{
			float q = n * deltaQ;
			float q2 = q * q;
			float eQ = q2 * std::exp(-q2);
			g += ::_j0( q * r ) * eQ;
		}
		m_kernelValuesLookUp[index] = g / g0;
	}
}


//---------------------------------------------------------------------------------------------------------
float IWave::GetKernelValue( int xPos, int yPos )
{
	int offsetXPos = xPos + m_kernelSize;
	int offsetYPos = yPos + m_kernelSize;
	int kernelIndex = offsetXPos + ( offsetYPos * m_kernelDimension );
	return m_kernelValuesLookUp[kernelIndex];
}


//---------------------------------------------------------------------------------------------------------
void IWave::CalculateNewHeights( float deltaSeconds )
{
	constexpr float a = 0.3f;

	ConvolveHeights();

	float aDeltaT = a * deltaSeconds;
	float curr = ( 2.f - aDeltaT ) / ( 1.f + aDeltaT );
	float prev = 1.f / ( 1.f + aDeltaT );
	float g = ( GRAVITY * deltaSeconds * deltaSeconds ) / ( 1.f + aDeltaT );

	float tempHeight;
	for( int index = 0; index < m_samplePoints.size(); ++index )
	{
		IWaveData& currentPoint = m_samplePoints[index];
		tempHeight = currentPoint.m_height;


		currentPoint.m_height = ( currentPoint.m_height * curr ) - ( currentPoint.m_previousHeight * prev ) - ( currentPoint.m_verticalDerivative * g );
		if( ApproximatelyEqual( currentPoint.m_height, 0.f, 0.0001f ) )
		{
			currentPoint.m_height = 0.f;
		}
		
		currentPoint.m_previousHeight = tempHeight;
	}
}


//---------------------------------------------------------------------------------------------------------
void IWave::ConvolveHeights()
{
	for( int index = 0; index < m_samplePoints.size(); ++index )
	{
		m_samplePoints[index].m_verticalDerivative = GetConvolutionValue( index );
	}
}


//---------------------------------------------------------------------------------------------------------
float IWave::GetConvolutionValue( int index )
{
	float convolveValue = 0.f;
	for( int k = -m_kernelSize; k < m_kernelSize + 1; ++k )
	{
		for( int l = -m_kernelSize; l < m_kernelSize + 1; ++l )
		{
// 			float NEHeight = GetHeightAtIndex( index + ( m_gridDimensions.x * l ) + k );
// 			float NWHeight = GetHeightAtIndex( index + ( m_gridDimensions.x * l ) - k );
// 			float SEHeight = GetHeightAtIndex( index - ( m_gridDimensions.x * l ) + k );
// 			float SWHeight = GetHeightAtIndex( index - ( m_gridDimensions.x * l ) - k );
// 
// 			convolveValue += GetKernelValue( k, l ) * ( NEHeight + SEHeight + NWHeight + SWHeight );

 			float height = GetHeightAtIndex( index + ( m_gridDimensions.x * l ) + k );
			convolveValue += GetKernelValue( k, l ) * height;
		}
	}
	//Clamp( convolveValue, -1.f, 1.f );
	return convolveValue;// +m_samplePoints[index].m_height;
}



//---------------------------------------------------------------------------------------------------------
float IWave::GetHeightAtIndex( int index )
{
	if( index < 0 )
	{
		int tiledIndex = index + m_samplePoints.size() - 1;
		return m_samplePoints[tiledIndex].m_height;
	}
	else if( index > m_samplePoints.size() - 1 )
	{
		int tiledIndex = index - ( m_samplePoints.size() - 1 );
		return m_samplePoints[tiledIndex].m_height;
	}
	else
	{
		return m_samplePoints[index].m_height;
	}
}


//---------------------------------------------------------------------------------------------------------
void IWave::AddWaterObject( WaterObject* waterObjectToAdd )
{
	Vec3 position = waterObjectToAdd->GetPosition();
	AABB3 objectBounds = waterObjectToAdd->GetBounds();
	AABB2 object2DInnerBounds = AABB2( objectBounds.mins.x, objectBounds.mins.y, objectBounds.maxes.x, objectBounds.maxes.y );


	Vec2 minOutterBoundsOnGrid;
	minOutterBoundsOnGrid.x = RoundDownToInt( object2DInnerBounds.mins.x / m_deltas.x ) * m_deltas.x;
	minOutterBoundsOnGrid.y = RoundDownToInt( object2DInnerBounds.mins.y / m_deltas.y ) * m_deltas.y; 
	
	Vec2 maxOutterBoundsOnGrid;
	maxOutterBoundsOnGrid.x = ( RoundDownToInt( object2DInnerBounds.maxes.x / m_deltas.x ) + 1 ) * m_deltas.x;
	maxOutterBoundsOnGrid.y = ( RoundDownToInt( object2DInnerBounds.maxes.y / m_deltas.y ) + 1 ) * m_deltas.y; 

	AABB2 object2DOutterBounds = AABB2( minOutterBoundsOnGrid, maxOutterBoundsOnGrid );

	object2DInnerBounds.Translate( Vec2( position.x, position.y ) );
	object2DOutterBounds.Translate( Vec2( position.x, position.y ) );


	for( int i = 0; i < m_obstructions.size(); ++i )
	{
		Vec2 samplePointPosition;
		samplePointPosition.y = static_cast<float>( ( i / m_gridDimensions.x ) - ( m_gridDimensions.x / 2 ) ) * m_deltas.y; 
		samplePointPosition.x = static_cast<float>( ( i % m_gridDimensions.x ) - ( m_gridDimensions.y / 2 ) ) * m_deltas.x;

		if( IsPointInsideAABB2D( samplePointPosition, object2DInnerBounds ) )
		{
			m_obstructions[i] = 0.f;
		}
		else if( IsPointInsideAABB2D( samplePointPosition, object2DOutterBounds ) )
		{
			Vec2 nearestPointOnInnerBounds = GetNearestPointOnAABB2D( samplePointPosition, object2DInnerBounds );
			Vec2 displacementToPoint = samplePointPosition - nearestPointOnInnerBounds;
			float percent = ( ( displacementToPoint.x / m_deltas.x ) + ( displacementToPoint.y / m_deltas.y ) ) * 0.5f;
			m_obstructions[i] = 1.f - abs( percent );
		}
	}
}

