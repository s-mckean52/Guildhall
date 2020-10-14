#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/GameCommon.hpp"

struct WaveSurfaceVertex
{
public:
	IntVec2			m_translatedCoord	= IntVec2( 0, 0 );
	ComplexFloat	m_hTilde0			= ComplexFloat( 0.f, 0.f );
	ComplexFloat	m_hTilde0Conj		= ComplexFloat( 0.f, 0.f );

	Vec2			m_k					= Vec2::ZERO;
	ComplexFloat	m_hTilde			= ComplexFloat( 0.f, 0.f );
	ComplexFloat	m_position[2];
	Vec2			m_initialPosition	= Vec2::ZERO;

	Vec3			m_surfaceNormal		= Vec3::UNIT_POSITIVE_Z;
	ComplexFloat	m_surfaceSlope[2];

public:
	WaveSurfaceVertex( int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions, Vec2 const& dimensions );
	~WaveSurfaceVertex() = default;

	Vec2 GetK()	{ return m_k; }

	void CalculateHTildeAtTime( float time );
	ComplexFloat CalculateHTilde0( bool doesNegateK = false );

	void CalculateValuesAtTime( float time );

private:
	void TranslateSurfaceCoord( int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions );
	void CalculateK( IntVec2 const& sampleDimensions, Vec2 const& dimensions );
};