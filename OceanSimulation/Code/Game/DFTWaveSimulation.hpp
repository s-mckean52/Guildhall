#pragma once
#include "Game/WaveSimulation.hpp"
#include <complex>

struct WavePoint
{
	std::complex<float>	m_height;
	Vec2				m_position	= Vec2::ZERO;
};

struct HTilde0Data
{
	std::complex<float> m_htilde0;
	std::complex<float> m_htilde0Conj;
};


class DFTWaveSimulation : public WaveSimulation
{
public:
	DFTWaveSimulation( Vec2 const& dimensions, uint samples );
	~DFTWaveSimulation();

	void Simulate() override;

	float				GetDeepDispersion( Vec2 const& k );
	float				PhillipsEquation( Vec2 const& k );
	std::complex<float>	hTilde0( Vec2 const& k );
	std::complex<float>	hTilde( int index, Vec2 const& k, float time );
	WavePoint			GetHeightAtPosition( Vec2 const& initialPosition, float time );

private:
	//Phillips Spectrum Variables
	float	m_A				= 1.f;			//Phillips Spectrum Constant
	Vec2	m_windDirection	= Vec2::RIGHT;
	float	m_windSpeed		= 1.f;

	std::vector<HTilde0Data> m_hTildeData;
};