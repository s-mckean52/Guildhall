#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>
#include <complex>

typedef std::complex<float>			ComplexFloat;
typedef std::vector<ComplexFloat>	ComplexFloatVector;

struct	Vec3;
struct	Rgba8;
struct	IntVec2;
class	Transform;
class	GPUMesh;


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
	virtual ~WaveSimulation() = default;
	WaveSimulation( Vec2 const& dimensions, uint samples );

	virtual void Simulate() = 0;

	virtual void Render() const;
	virtual void AddWave( Wave* waveToAdd );

	int			GetNumWaves() const;
	Wave*		GetWaveAtIndex( int index ) const;
	float		GetDeepDispersion( Vec2 const& k );
	float		PhillipsEquation( Vec2 const& k );
	
	ComplexFloat hTilde( int n, int m, float time );
	ComplexFloat hTilde0( int n, int m, bool doesNegateK = false );
	void		ToggleWireFrameMode();

	bool		IsValidNumSamples( uint numSamples );
	Vec2		GetK( int n, int m );

	void		SetPosition( Vec3 const& newPosition );
	void		SetTilingDimensions( uint tilingDimenisions );

private:
	void	GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps );

protected:
	uint				m_tilingDimensions = 1;
	bool				m_isWireFrame	= true;
	Vec2				m_dimensions	= Vec2( 1.f, 1.f );
	Transform*			m_transform		= nullptr;
	uint				m_numSamples	= 16;
	std::vector<Wave*>	m_waves;

		//Phillips Spectrum Variables
	float	m_A				= 0.01f;//05f;			//Phillips Spectrum Constant
	Vec2	m_windDirection	= Vec2::RIGHT;
	float	m_windSpeed		= 37.f;

	GPUMesh*					m_surfaceMesh		= nullptr; 
	std::vector<uint>			m_surfaceIndicies;
	std::vector<Vertex_PCUTBN>	m_surfaceVerts;
	std::vector<Vec3>			m_initialSurfacePositions;

	std::vector<HTilde0Data> m_hTilde0Data;
};