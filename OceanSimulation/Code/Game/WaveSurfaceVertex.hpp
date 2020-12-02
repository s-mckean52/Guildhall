#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

struct Vertex_PCUTBN;
struct WaveSurfaceVertex;
class WaveSimulation;

typedef std::complex<float>			ComplexFloat;
typedef std::vector<ComplexFloat>	ComplexFloatVector;
typedef std::vector<WaveSurfaceVertex>	WaveSurfaceVertexVector;

struct WaveSurfaceVertex
{
public:
	WaveSimulation* m_owner = nullptr;

	IntVec2			m_translatedCoord	= IntVec2( 0, 0 );
	ComplexFloat	m_hTilde0			= ComplexFloat( 0.f, 0.f );
	ComplexFloat	m_hTilde0Conj		= ComplexFloat( 0.f, 0.f );

	Vec2			m_k					= Vec2::ZERO;
	ComplexFloat	m_hTilde			= ComplexFloat( 0.f, 0.f );
	float			m_height			= 0.f;

	ComplexFloat	m_position[2];
	Vec3			m_initialPosition	= Vec3::ZERO;

	Vec3			m_surfaceNormal		= Vec3::UNIT_POSITIVE_Z;
	ComplexFloat	m_surfaceSlope[2];

public:
	WaveSurfaceVertex() {};
	WaveSurfaceVertex( WaveSimulation* owner, int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions, Vec2 const& dimensions );
	~WaveSurfaceVertex() {};

	void SetVertexPositionAndNormal( Vertex_PCUTBN& vertexToModify, bool isTiledVert = false );

	Vec2 const&	GetK() const	{ return m_k; }

	void			CalculateValuesAtTime( float time );
	void			CalculateHTildeAtTime( float time );
	ComplexFloat	CalculateHTilde0( bool doesNegateK = false );

private:
	void TranslateSurfaceCoord( int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions );
	void CalculateK( IntVec2 const& sampleDimensions, Vec2 const& dimensions );
};