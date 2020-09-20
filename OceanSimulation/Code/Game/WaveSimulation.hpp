#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

struct	Vec3;
struct	Rgba8;
struct	IntVec2;
class	Transform;
class	GPUMesh;

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
	
	void		ToggleWireFrameMode();

private:
	void GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps );

protected:
	bool				m_isWireFrame	= false;
	Transform*			m_transform		= nullptr;
	Vec2				m_dimensions	= Vec2( 10.f, 10.f );
	uint				m_numSamples	= 128;
	std::vector<Wave*>	m_waves;

	GPUMesh*					m_surfaceMesh		= nullptr; 
	std::vector<uint>			m_surfaceIndicies;
	std::vector<Vertex_PCUTBN>	m_surfaceVerts;
	std::vector<Vec3>			m_initialSurfacePositions;
};