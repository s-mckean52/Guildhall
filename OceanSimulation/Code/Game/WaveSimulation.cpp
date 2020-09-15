#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Transform.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WaveSimulation.hpp"


//---------------------------------------------------------------------------------------------------------
// Wave
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
Wave::Wave( Vec2 const& dir, float waveSize, float setAmplitude, float setPhase )
	: amplitude( setAmplitude )
	, phase( setPhase )
{
	direction = dir * waveSize;
	CalculateMagnitudeAndFrequency( waveSize );
}


//---------------------------------------------------------------------------------------------------------
void Wave::CalculateMagnitudeAndFrequency( float waveSize )
{
	magnitude = ( 2 * PI_VALUE ) / waveSize;
	frequency = sqrtf( 9.81f * magnitude );
}


//---------------------------------------------------------------------------------------------------------
void Wave::RotateDirectionDegrees( float degreesToRotate )
{
	direction.RotateDegrees( degreesToRotate );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddAmplitude( float amplitudeToAdd )
{
	amplitude += amplitudeToAdd;
	Clamp( amplitude, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddMagnitude( float magnitudeToAdd )
{
	float waveLength = direction.GetLength();
	waveLength += magnitudeToAdd;
	Clamp( waveLength, 0.1f, 100.f );
	direction.SetLength( waveLength );
	CalculateMagnitudeAndFrequency( waveLength );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddPhase( float phaseToAdd )
{
	phase += phaseToAdd;
	Clamp( phase, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
// Wave Simulation
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
WaveSimulation::WaveSimulation( Vec2 const& dimensions, uint samples )
	: m_dimensions( dimensions )
	, m_numSamples( samples )
{
	m_transform = new Transform();

	GenerateSurface( Vec3::ZERO, Rgba8::WHITE, dimensions, IntVec2( samples, samples ) );
	m_surfaceMesh = new GPUMesh( g_theRenderer, m_surfaceVerts, m_surfaceIndicies );
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::Render() const
{
	g_theRenderer->SetModelUBO( m_transform->ToMatrix() );
	g_theRenderer->DrawMesh( m_surfaceMesh );
}

//---------------------------------------------------------------------------------------------------------
void WaveSimulation::AddWave( Wave* waveToAdd )
{
	m_waves.push_back( waveToAdd );
}


//---------------------------------------------------------------------------------------------------------
int WaveSimulation::GetNumWaves() const
{
	return static_cast<int>( m_waves.size() );
}


//---------------------------------------------------------------------------------------------------------
Wave* WaveSimulation::GetWaveAtIndex( int index ) const
{
	Clamp( index, 0, static_cast<int>( m_waves.size() - 1 ) );
	return m_waves[index];
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps )
{
	float xStepAmount = dimensions.x / static_cast<float>( steps.x );
	float yStepAmount = dimensions.y / static_cast<float>( steps.y );

	float currentY = 0.f;
	for( int yStep = 0; yStep < steps.y + 1; ++yStep )
	{
		float currentX = 0.f;
		for( int xStep = 0; xStep < steps.x + 1; ++xStep )
		{
			Vec3 currentPosition = origin;
			currentPosition.x += currentX;
			currentPosition.y += currentY;

			float u = RangeMapFloat( 0.f, dimensions.x, 0.f, 1.f, currentX );
			float v = RangeMapFloat( 0.f, dimensions.y, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v );

			m_surfaceVerts.push_back( Vertex_PCUTBN( currentPosition, color, Vec3::UNIT_POSITIVE_X, Vec3::UNIT_POSITIVE_Y, Vec3::UNIT_POSITIVE_Z, uv ) );
			currentX += xStepAmount;
		}
		currentY += yStepAmount;
	}

	unsigned int indexOffset = static_cast<unsigned int>( m_surfaceIndicies.size() );
	for( int yIndex = 0; yIndex < steps.y; ++yIndex )
	{
		for( int xIndex = 0; xIndex < steps.x; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( ( steps.x + 1 ) * yIndex );

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + steps.x + 1;
			unsigned int topRight = currentVertIndex + steps.x + 2;

			m_surfaceIndicies.push_back( indexOffset + bottomLeft );
			m_surfaceIndicies.push_back( indexOffset + bottomRight );
			m_surfaceIndicies.push_back( indexOffset + topRight );

			m_surfaceIndicies.push_back( indexOffset + bottomLeft );
			m_surfaceIndicies.push_back( indexOffset + topRight );
			m_surfaceIndicies.push_back( indexOffset + topLeft );
		}
	}
}
