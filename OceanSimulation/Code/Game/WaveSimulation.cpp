#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/WaveSimulation.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/WaterObject.hpp"


//---------------------------------------------------------------------------------------------------------
// Wave
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
Wave::Wave( Vec2 const& dir, float wavelength, float setAmplitude, float setPhase )
	: m_amplitude( setAmplitude )
	, m_phase( setPhase )
{
	m_directionNormal = dir;
	if( !ApproximatelyEqual( m_directionNormal.GetLength(), 0.f ) )
	{
		m_directionNormal.Normalize();
	}

	CalculateMagnitudeAndFrequency( wavelength );
}


//---------------------------------------------------------------------------------------------------------
void Wave::CalculateMagnitudeAndFrequency( float waveSize )
{
	m_magnitude = ( 2 * PI_VALUE ) / waveSize;
	m_frequency = sqrtf( 9.81f * m_magnitude );
}


//---------------------------------------------------------------------------------------------------------
void Wave::RotateDirectionDegrees( float degreesToRotate )
{
	m_directionNormal.RotateDegrees( degreesToRotate );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddAmplitude( float amplitudeToAdd )
{
	m_amplitude += amplitudeToAdd;
	Clamp( m_amplitude, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddMagnitude( float magnitudeToAdd )
{
	float waveLength = m_directionNormal.GetLength();
	waveLength += magnitudeToAdd;
	Clamp( waveLength, 0.1f, 100.f );
	m_directionNormal.SetLength( waveLength );
	CalculateMagnitudeAndFrequency( waveLength );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddPhase( float phaseToAdd )
{
	m_phase += phaseToAdd;
	Clamp( m_phase, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
// Wave Simulation
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
WaveSimulation::WaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed )
	: m_dimensions( dimensions )
	, m_numSamples( samples )
	, m_windSpeed( windSpeed )
{
	m_simulationClock = new Clock( g_theGame->GetGameClock() );
	m_transform = new Transform();

	GenerateSurface( Vec3::ZERO, Rgba8::WHITE, dimensions, IntVec2( samples, samples ) );
	m_surfaceMesh = new GPUMesh( g_theRenderer, m_surfaceVerts, m_surfaceIndicies );

	CreateQuadTree();
}


//---------------------------------------------------------------------------------------------------------
WaveSimulation::WaveSimulation( XmlElement const& element )
{
	m_simulationClock = new Clock(g_theGame->GetGameClock());
	m_transform = new Transform();

	std::string simulationModeAsString = ParseXmlAttribute( element, "type", "Default" );
	m_waveSimulationMode = GetWaveSimulationModeFromString( simulationModeAsString );
	m_numSamples = ParseXmlAttribute( element, "samples", -1 );
	m_dimensions = ParseXmlAttribute( element, "dimensions", Vec2( -1.f, -1.f ) );

	XmlElement const& phillipsElement = *element.FirstChildElement( "PhillipsSpectrum" );
	SetPhillipsSpectrumValues( phillipsElement ); 

	XmlElement const& defaultsElement = *element.FirstChildElement( "RuntimeDefaults" );
	SetRuntimeDefaults( defaultsElement ); 

	GenerateSurface( Vec3::ZERO, Rgba8::WHITE, m_dimensions, IntVec2( m_numSamples, m_numSamples ) );
	m_surfaceMesh = new GPUMesh( g_theRenderer, m_surfaceVerts, m_surfaceIndicies );

	CreateQuadTree();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::Simulate()
{
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::Render() const
{
	if( m_isWireFrame ) 
	{ 
		g_theRenderer->BindMaterialByPath( "Data/Shaders/Water_Wireframe.material" );
	}
	else
	{
		g_theRenderer->BindMaterialByPath( "Data/Shaders/Water.material" );
	}
	g_theRenderer->BindShaderByPath( "Data/Shaders/Water_Test.hlsl" );
	DrawMesh();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::RenderRefractionMap() const
{
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_FRONT );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LEQUAL, true );
	g_theRenderer->SetModelTint( Rgba8( 0, 0, 0, 0 ) );
	DrawMesh();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::RenderBackFaces() const
{
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetCullMode( CULL_MODE_FRONT );
	g_theRenderer->SetDepthTest( COMPARE_FUNC_LEQUAL, true );
	g_theRenderer->SetModelTint( Rgba8( 255, 0, 0, 255 ) );
	DrawMesh();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::DrawMesh() const
{
	Vec3 startPosition = -Vec3( m_dimensions, 0.f ) * ( ( static_cast<float>( m_tilingDimensions ) * 0.5f ) - 0.5f );
	uint tilingDimSquared = m_tilingDimensions * m_tilingDimensions;
	for( uint i = 0; i < tilingDimSquared; ++i )
	{
		Transform newTransform = *m_transform;
		Vec3 newPosition = startPosition;
		newPosition.x += m_dimensions.x * ( i / m_tilingDimensions );
		newPosition.y += m_dimensions.y * ( i % m_tilingDimensions );
		newTransform.SetPosition( newPosition );

		g_theRenderer->SetModelMatrix( newTransform.ToMatrix() );
		g_theRenderer->DrawMesh( m_surfaceMesh );
	}

	if( g_isDebugDraw )
	{
		DrawQuadTreeDebug();
	}
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
	if( m_waves.size() == 0 )
		return nullptr;

	Clamp( index, 0, static_cast<int>( m_waves.size() - 1 ) );
	return m_waves[index];
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::ToggleWireFrameMode()
{
	m_isWireFrame = !m_isWireFrame;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetIsWireFrame( bool isWireFrame )
{
	m_isWireFrame = isWireFrame;
}


//---------------------------------------------------------------------------------------------------------
bool WaveSimulation::IsValidNumSamples( uint numSamples )
{
	if( numSamples < 2 || numSamples > 2048 ) return false;

	if( numSamples & ( numSamples - 1 ) ) return false;

	return true;
}


//---------------------------------------------------------------------------------------------------------
Vec2 WaveSimulation::GetK( int n, int m )
{
	Vec2 k;
	k.x = ( PI_VALUE * ( ( 2.f * static_cast<float>( n ) ) - static_cast<float>( m_numSamples ) ) ) / m_dimensions.x;
	k.y = ( PI_VALUE * ( ( 2.f * static_cast<float>( m ) ) - static_cast<float>( m_numSamples ) ) ) / m_dimensions.y;

	return k;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetPosition( Vec3 const& newPosition )
{
	m_transform->SetPosition( newPosition );
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetTilingDimensions( uint tilingDimenisions )
{
	m_tilingDimensions = tilingDimenisions;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::ToggleSimulationClockPause()
{
	m_simulationClock->TogglePause();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::TransformByAverageWater( WaterObject* waterObjectToModify )
{
	AABB3 objectBounds = waterObjectToModify->GetBounds();
	Vec3 objectMinPos = objectBounds.mins + waterObjectToModify->GetPosition();
	Vec2 objectMinPosXY = Vec2( objectMinPos.x, objectMinPos.y );
	AABB2 containingWaterBounds;
	if( GetContainingWaterBoundsForPoint( objectMinPosXY, containingWaterBounds ) )
	{
		Vec2 faceitSize = m_dimensions / static_cast<float>( m_numSamples );
		AABB2 objectBoundsXY = AABB2( objectBounds.mins.x, objectBounds.mins.y, objectBounds.maxes.x, objectBounds.maxes.y );
		Vec2 objectBoundsXYDim = objectBoundsXY.GetDimensions();
		Vec2 localStartPos = objectMinPosXY - containingWaterBounds.mins;
		IntVec2 gridStartPos = IntVec2( RoundUpToInt( localStartPos.x / faceitSize.x ), RoundUpToInt( localStartPos.y / faceitSize.y ) );
		IntVec2 objectGridXYSteps = IntVec2( (int)( objectBoundsXYDim.x / faceitSize.x ), (int)( objectBoundsXYDim.y / faceitSize.y ) );

		Mat44 waterTransform = GetAverageWaterTransformOnGrid( gridStartPos, objectGridXYSteps, containingWaterBounds.GetCenter() );
		waterObjectToModify->SetWorldTransform( waterTransform );

		if( g_isDebugDraw )
		{
			Vec3 p0 = Vec3( containingWaterBounds.mins, 3.f );
			Vec3 p1 = Vec3( containingWaterBounds.maxes.x, containingWaterBounds.mins.y, 3.f );
			Vec3 p2 = Vec3( containingWaterBounds.maxes, 3.f );
			Vec3 p3 = Vec3( containingWaterBounds.mins.x, containingWaterBounds.maxes.y, 3.f );
			DebugAddWorldQuad( p0, p1, p2, p3, AABB2(), Rgba8::GREEN, Rgba8::GREEN, 0.f, DEBUG_RENDER_ALWAYS );
		}
	}
	else
	{
		waterObjectToModify->SetWorldTransform( Mat44::IDENTITY );
	}
}


//---------------------------------------------------------------------------------------------------------
bool WaveSimulation::GetContainingWaterBoundsForPoint( Vec2 const& positionToCheck, AABB2& out_foundBounds )
{
	for( uint waterBoundsIndex = 0; waterBoundsIndex < m_waveGridBounds.size(); ++waterBoundsIndex )
	{
		AABB2 const& currentBounds = m_waveGridBounds[waterBoundsIndex];
		if( IsPointInsideAABB2D( positionToCheck, currentBounds ) )
		{
			out_foundBounds = currentBounds;
			return true;
		}
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
Mat44 WaveSimulation::GetAverageWaterTransformOnGrid( IntVec2 const& gridStartPos, IntVec2 const& gridDimToUse, Vec2 const& waterBoundCenter )
{
	Vec3 positionTotal	= Vec3::ZERO;
	Vec3 tangentTotal	= Vec3::ZERO;
	Vec3 bitangentTotal = Vec3::ZERO;
	Vec3 normalTotal	= Vec3::ZERO;
	for( int yStep = 0; yStep < gridDimToUse.y; ++yStep )
	{
		for( int xStep = 0; xStep < gridDimToUse.x; ++xStep )
		{
			int samplesPlus1 = static_cast<int>( m_numSamples + 1 );
			IntVec2 gridPosition = gridStartPos + IntVec2( xStep, yStep );
			if( gridPosition.x >= samplesPlus1 )
			{
				gridPosition.x -= samplesPlus1;
			}
			if( gridPosition.y >= samplesPlus1 )
			{
				gridPosition.y -= samplesPlus1;
			}

			int gridPositionIndex = gridPosition.x + ( gridPosition.y * samplesPlus1 );
			Vertex_OCEAN& vertexToAdd = m_surfaceVerts[gridPositionIndex];
			positionTotal	+= vertexToAdd.m_position;
			tangentTotal	+= vertexToAdd.m_tangent;
			bitangentTotal	+= vertexToAdd.m_bitangent;
			normalTotal		+= vertexToAdd.m_normal;

			if( g_isDebugDraw )
			{
				DebugAddWorldPoint( vertexToAdd.m_position + Vec3( waterBoundCenter, 0.f ), 0.1f, Rgba8::YELLOW, 0.f, DEBUG_RENDER_ALWAYS );
			}
		}
	}

	int totalPointsHit = gridDimToUse.x * gridDimToUse.y;
	if( totalPointsHit == 0 )
	{
		return Mat44::IDENTITY;
	}

	float inversePointsHit = 1.f / static_cast<float>( totalPointsHit );

	Vec3 positionAverage	= positionTotal * inversePointsHit;
	Vec3 tangentAverage		= tangentTotal * inversePointsHit;
	Vec3 bitangentAverage	= bitangentTotal * inversePointsHit;
	Vec3 normalAverage		= normalTotal * inversePointsHit;

	Mat44 waveVertOrientation = Mat44( tangentAverage, bitangentAverage, normalAverage, Vec3( 0.f, 0.f, positionAverage.z ) );
	return waveVertOrientation;
}


//---------------------------------------------------------------------------------------------------------
STATIC WaveSimulation* WaveSimulation::CreateWaveSimulation( std::string filePath )
{
	XmlDocument waveSimulationFile = new XmlDocument();
	waveSimulationFile.LoadFile( filePath.c_str() );

	if( waveSimulationFile.ErrorID() != 0 )
	{
		g_theConsole->ErrorString( "%s does not exist in Run/Data", filePath.c_str() );
		return nullptr;
	}
	return CreateWaveSimulationFromXML( waveSimulationFile.RootElement() );	
}


//---------------------------------------------------------------------------------------------------------
STATIC WaveSimulation* WaveSimulation::CreateWaveSimulationFromXML( XmlElement* element )
{
	std::string waveSimulationType = ParseXmlAttribute( *element, "type", "Default" );
	WaveSimulationMode waveMode = GetWaveSimulationModeFromString( waveSimulationType );
	switch( waveMode )
	{
	case FFT_WAVE_SIMULATION: { return new FFTWaveSimulation( *element ); };
	case DFT_WAVE_SIMULATION: //{ //return new DFTWaveSimulation( element );
	case GERSTNER_WAVE_SIMULATION:
	default:
		{ return nullptr; }
	}
}


//---------------------------------------------------------------------------------------------------------
WaveSimulationMode WaveSimulation::GetWaveSimulationModeFromString( std::string waveSimulationMode )
{
	if( waveSimulationMode == "FFT" )			{ return FFT_WAVE_SIMULATION; }
	else if( waveSimulationMode == "DFT" )		{ return DFT_WAVE_SIMULATION; }
	else if( waveSimulationMode == "GERSTNER" )	{ return GERSTNER_WAVE_SIMULATION; }
	else										{ return FFT_WAVE_SIMULATION; }
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetPhillipsSpectrumValues( XmlElement const& element )
{
	m_A					= ParseXmlAttribute( element, "aConstant", -1.f );
	m_windSpeed			= ParseXmlAttribute( element, "windSpeed", -1.f );
	m_waveSuppression	= ParseXmlAttribute( element, "waveSuppression", -1.f );

	Vec2 defaultWindDir = Vec2( 0.f, 0.f );
	m_windDirection	= ParseXmlAttribute( element, "windDirection", defaultWindDir );
	if( m_windDirection == defaultWindDir )
	{
		g_theConsole->ErrorString( "Wind Direction defaulted to (1.f, 0.f)" );
		m_windDirection = Vec2( 1.f, 0.f );
	}
	m_windDirection.Normalize();
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetRuntimeDefaults( XmlElement const& element )
{
	m_choppyWaterValue	= ParseXmlAttribute( element, "choppiness", 0.f );
	m_isIWaveEnabled	= ParseXmlAttribute( element, "iWaveEnabled", false );
	m_isWireFrame		= ParseXmlAttribute( element, "wireFrameEnabled", false );
	m_tilingDimensions	= ParseXmlAttribute( element, "tilingSize", 1 );
	//m_dispersionRelation = ParseXmlAttribute( element, "dispersionRelation", "Default" );
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps )
{
	Vec2 halfDimensions = dimensions * 0.5f;

	float xStepAmount = dimensions.x / static_cast<float>( steps.x );
	float yStepAmount = dimensions.y / static_cast<float>( steps.y );

	float currentY = -halfDimensions.y;
	for( int yStep = 0; yStep < steps.y + 1; ++yStep )
	{
		float currentX = -halfDimensions.x;
		for( int xStep = 0; xStep < steps.x + 1; ++xStep )
		{
			Vec3 currentPosition = origin;
			currentPosition.x += currentX;
			currentPosition.y += currentY;

			float u = RangeMapFloat( -halfDimensions.x, halfDimensions.x, 0.f, 1.f, currentX );
			float v = RangeMapFloat( -halfDimensions.y, halfDimensions.y, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v );

			m_initialSurfacePositions.push_back( currentPosition );
			m_surfaceVerts.push_back( Vertex_OCEAN( currentPosition, color, Vec3::UNIT_POSITIVE_X, Vec3::UNIT_POSITIVE_Y, Vec3::UNIT_POSITIVE_Z, uv ) );
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


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::CreateQuadTree()
{
	m_waveGridBounds.clear();
	Vec2 startPosition = -m_dimensions * ( static_cast<float>( m_tilingDimensions ) * 0.5f );
	uint tilingDimSquared = m_tilingDimensions * m_tilingDimensions;
	for( uint i = 0; i < tilingDimSquared; ++i )
	{
		int yPos = i / m_tilingDimensions;
		int xPos = i - ( yPos * m_tilingDimensions );

		AABB2 newBounds;
		newBounds.mins = startPosition + Vec2( m_dimensions.x * static_cast<float>( xPos ), m_dimensions.y * static_cast<float>( yPos ) );
		newBounds.maxes = newBounds.mins + m_dimensions;
		m_waveGridBounds.push_back( newBounds );
	}
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::DrawQuadTreeDebug() const
{
	std::vector<Vertex_PCU> verts;
	uint tilingDimSquared = m_tilingDimensions * m_tilingDimensions;
	for( uint i = 0; i < tilingDimSquared; ++i )
	{
		AABB2 const& boundsToDraw = m_waveGridBounds[i];
		AppendVertsForAABB2OutlineAtPoint( verts, boundsToDraw, Rgba8::MAGENTA, 0.1f );
	}

	TranslateVertexArray( verts, Vec3( 0.f, 0.f, 3.f ) );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelUBO( Mat44::IDENTITY );
	g_theRenderer->DrawVertexArray( verts );
}


//---------------------------------------------------------------------------------------------------------
float WaveSimulation::GetDeepDispersion( float kLength )
{
	constexpr float w0 = 2.f * 3.14159f / 200.f;
	float wk = sqrtf( GRAVITY * kLength );
	return RoundDownToInt( wk ) * w0;
}


//---------------------------------------------------------------------------------------------------------
float WaveSimulation::PhillipsEquation( Vec2 const& k, float lengthK )
{
	// A * e^( -1 / ( k * L )^2 ) / k^4 * ( Dot( k, w )^2 )

	if( lengthK <= m_waveSuppression )
		return 0.f;

	float L = ( m_windSpeed * m_windSpeed ) * INVERSE_GRAVITY;

	float kQuad = lengthK * lengthK * lengthK * lengthK;
	float exponentOfE = -1 / ( ( lengthK * L ) * ( lengthK * L) );
	float eComponent = std::exp( exponentOfE );

	float kDotW = DotProduct2D( k.GetNormalized(), m_windDirection );
	if( kDotW <= 0.f )
		kDotW = 0.f;

	float kDotWSquared = kDotW * kDotW;

	float damperSquared = m_waveSuppression * m_waveSuppression;
	float lengthKSquared = lengthK * lengthK;
	float supressionValue = std::exp( -lengthKSquared * damperSquared );

	return m_A * ( eComponent / kQuad ) * kDotWSquared * supressionValue;
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat WaveSimulation::hTilde( int n, int m, float time )
{
	//ERROR_AND_DIE( "WaveSimulation::hTilde deprecated - Use WaveSurfaceVertex::hTilde" );
	// 	int halfSamples = static_cast<int>( m_numSamples * 0.5 );
	// 	int index = ( ( m + halfSamples ) * m_numSamples ) + ( n + halfSamples );
	int index = ( m * m_numSamples ) + n;

	Vec2 k = GetK( n, m );

	ComplexFloat htilde0 = m_hTilde0Data[index].m_htilde0;
	ComplexFloat htilde0Conj = m_hTilde0Data[index].m_htilde0Conj;

	float dispersionRelation = GetDeepDispersion( k.GetLength() );
	float dispersionTime = dispersionRelation * time * 10.f;

	float cosDispersionTime = cos( dispersionTime );
	float sinDispersionTime = sin( dispersionTime );

	ComplexFloat eulers( cosDispersionTime, sinDispersionTime );
	ComplexFloat eulersConj( cosDispersionTime, -sinDispersionTime );
	
	return ( htilde0 * eulers ) + ( htilde0Conj * eulersConj );
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat WaveSimulation::hTilde0( int n, int m, bool doesNegateK )
{
	//ERROR_AND_DIE( "WaveSimulation::hTilde0 deprecated - Use WaveSurfaceVertex::hTilde0" );
	Vec2 k = GetK( n, m );
	if( doesNegateK )
	{
		k = -k;
	}

	const float inverse_sqrt_2 = 1.f / sqrtf(2.f);

	//Gaussian;
	float random1 = g_RNG->RollRandomFloatZeroToOneInclusive();
	float random2 = g_RNG->RollRandomFloatZeroToOneInclusive();

	float v = sqrtf( -2 * std::log( random1 ) );
	float f = 2.f * PI_VALUE * random2;

	float gRand1 = v * cos(f);
	float gRand2 = v * sin(f);

	std::complex<float> guassianComplex( gRand1, gRand2 );
	
	return inverse_sqrt_2 * guassianComplex * sqrt( PhillipsEquation( k, k.GetLength() ) ); 
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetIWaveEnabled( bool isEnabled)
{
	m_isIWaveEnabled = isEnabled;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetChoppyWaterValue( float choppyWaterValue )
{
	m_choppyWaterValue = choppyWaterValue;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::AddChoppyWaterValue( float choppyWaterValueToAdd )
{
	m_choppyWaterValue += choppyWaterValueToAdd;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::AddTimeFactor( float timeFactorToAdd )
{
	m_timeFactor += timeFactorToAdd;
	m_simulationClock->SetScale( static_cast<double>( m_timeFactor ) );
}

