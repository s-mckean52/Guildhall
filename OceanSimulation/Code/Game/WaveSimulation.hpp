#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/GameCommon.hpp"
#include <vector>


struct	Vec3;
struct	Rgba8;
struct	IntVec2;
class	Transform;
class	GPUMesh;
class	Clock;
struct Texture;

//---------------------------------------------------------------------------------------------------------
enum WaveSimulationMode
{
	FFT_WAVE_SIMULATION,
	DFT_WAVE_SIMULATION,
	GERSTNER_WAVE_SIMULATION,
};

//---------------------------------------------------------------------------------------------------------
struct HTilde0Data
{
	ComplexFloat m_htilde0;
	ComplexFloat m_htilde0Conj;
};


//---------------------------------------------------------------------------------------------------------
struct Wave
{
public:
	Vec2	m_directionNormal	= Vec2::RIGHT;
	float	m_directionLength	= 1.f;
	float	m_amplitude			= 1.f;
	float	m_phase				= 0.f;
	float	m_frequency			= 100.f;
	float	m_magnitude			= 100.f;

public:
	~Wave() = default;
	explicit Wave( Vec2 const& dir, float length, float setAmplitude, float setPhase = 0.f );

	void CalculateMagnitudeAndFrequency( float waveSize );

	void RotateDirectionDegrees( float degreesToRotate );
	void AddAmplitude( float amplitudeToAdd );
	void AddMagnitude( float magnitudeToAdd );
	void AddPhase( float phaseToAdd );
};


//---------------------------------------------------------------------------------------------------------
class WaveSimulation
{
public:
	virtual ~WaveSimulation() {};
	WaveSimulation( XmlElement const& element );
	WaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed );

	virtual void Simulate();

	virtual void Render() const;
	virtual void AddWave( Wave* waveToAdd );

	int			GetNumWaves() const;
	uint		GetNumTiles() const			{ return m_tilingDimensions; }
	int			GetNumSamples() const		{ return m_numSamples; }
	Vec2 const& GetGridDimensions() const	{ return m_dimensions; }
	float		GetWindSpeed() const		{ return m_windSpeed; }
	float		GetAConstant() const		{ return m_A; }
	float		GetWaveSuppression() const	{ return m_waveSuppression; }
	Vec2 const&	GetWindDirection() const	{ return m_windDirection; }

	Wave*		GetWaveAtIndex( int index ) const;
	float		GetDeepDispersion( float kLength );
	float		PhillipsEquation( Vec2 const& k, float lengthK );

	bool	IsWireFrameEnabled() const				{ return m_isWireFrame; }
	bool	IsIWaveEnabled() const					{ return m_isIWaveEnabled; }
	float	GetChoppyWaterValue() const				{ return m_choppyWaterValue; }
	void	SetIWaveEnabled( bool isEnabled );
	void	SetChoppyWaterValue( float choppyWaterValue );
	void	AddChoppyWaterValue( float choppyWaterValueToAdd );
	
	ComplexFloat hTilde( int n, int m, float time );
	ComplexFloat hTilde0( int n, int m, bool doesNegateK = false );
	void		ToggleWireFrameMode();
	void		SetIsWireFrame( bool isWireFrame );

	bool		IsValidNumSamples( uint numSamples );
	Vec2		GetK( int n, int m );

	void		SetPosition( Vec3 const& newPosition );
	void		SetTilingDimensions( uint tilingDimenisions );
	void		ToggleSimulationClockPause();

	Texture*	GenerateNormalMap();

public:
	static	WaveSimulation*		CreateWaveSimulation( std::string filePath );
	static	WaveSimulation*		CreateWaveSimulationFromXML( XmlElement* element );
	static	WaveSimulationMode	GetWaveSimulationModeFromString( std::string waveSimulationMode );
			void				SetPhillipsSpectrumValues( XmlElement const& element );
			void				SetRuntimeDefaults( XmlElement const& element );

private:
	void	GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps );

protected:
	Clock*				m_simulationClock		= nullptr;
	WaveSimulationMode	m_waveSimulationMode	= FFT_WAVE_SIMULATION;
	bool				m_isIWaveEnabled		= false;
	float				m_choppyWaterValue		= 0.f;

	uint				m_tilingDimensions = 1;
	bool				m_isWireFrame	= true;
	Vec2				m_dimensions	= Vec2( 1.f, 1.f );
	Transform*			m_transform		= nullptr;
	uint				m_numSamples	= 16;
	std::vector<Wave*>	m_waves;

		//Phillips Spectrum Variables
	float	m_A					= 0.0005f;			//Phillips Spectrum Constant 0.001 is slightly above max value for this
	float	m_waveSuppression	= 1.f;
	Vec2	m_windDirection		= Vec2::RIGHT;
	float	m_windSpeed			= 37.f;

	GPUMesh*					m_surfaceMesh		= nullptr; 
	std::vector<uint>			m_surfaceIndicies;
	std::vector<Vertex_PCUTBN>	m_surfaceVerts;
	std::vector<Vec3>			m_initialSurfacePositions;

	std::vector<HTilde0Data> m_hTilde0Data;
};