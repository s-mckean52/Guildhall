#pragma once
#include "Game/WaveSimulation.hpp"

struct WavePoint;
struct HTilde0Data;

class FFTWaveSimulation : public WaveSimulation
{
public:
	~FFTWaveSimulation();
	FFTWaveSimulation( Vec2 const& dimensions, uint samples );

	void Simulate() override;

	void GetHeightAtPosition( int n, int m, float time );

	uint			ReverseBits( uint value );
	void			CreateBitReversedIndicies();
	void			CalculateTForIndices();
	ComplexFloat 	GetTCalculation( uint x, uint samplesAtDimension );

	void CalculateFFT( std::vector<ComplexFloat>& data_in, std::vector<ComplexFloat>& data_out, int stride, int offset );


protected:
	uint m_log2N = 0;
	float m_pi2 = 0.f;
	uint which = 0;

	ComplexFloatVector m_hTilde;
	ComplexFloatVector m_hTilde_dx;
	ComplexFloatVector m_hTilde_dy;
	ComplexFloatVector m_slopeX;
	ComplexFloatVector m_slopeY;

	std::vector<ComplexFloatVector> m_c;
	std::vector<ComplexFloatVector> m_Ts;

	std::vector<uint> m_bitReversedIndices;
};