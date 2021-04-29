#pragma once
#include "Game/WaveSimulation.hpp"
#include "Engine/Core/ProfileTimer.hpp"

struct	WavePoint;
struct	HTilde0Data;
struct	WaveSurfaceVertex;
class	IWave;

class FFTWaveSimulation : public WaveSimulation
{
	friend class FFTJob;

public:
	~FFTWaveSimulation();
	FFTWaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed );
	FFTWaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed, Vec2 const& windDir, float aConstant, float waveSuppression );
	FFTWaveSimulation( XmlElement const& element );

	void Simulate() override;
	void InitializeValues();

	void GetHeightAtPosition( int n, int m, float time );
	WaveSurfaceVertex* GetWaveVertAtIndex( int index );

	uint			ReverseBits( uint value );
	void			CreateBitReversedIndicies();
	void			CalculateTForIndices();
	ComplexFloat 	GetTCalculation( uint x, uint samplesAtDimension );

	void CalculateFFT( std::vector<ComplexFloat>& data_in, std::vector<ComplexFloat>& data_out, int stride, int offset );
	void CalculateFFT( std::vector<WaveSurfaceVertex>& data, int stride, int offset );

	float	CalculateJacobianForVertexAtIndex( int vertIndex );

public:
	IWave* m_iWave = nullptr;
	ProfileTimer m_fftTimer					= ProfileTimer( "FFT_FFTTimer" );
	ProfileTimer m_simulateTimer			= ProfileTimer( "FFT_SimulateTimer" );
	ProfileTimer m_pointCalculationTimer	= ProfileTimer( "FFT_PointTimer" );

protected:
	uint m_log2N = 0;
	float m_pi2 = 0.f;
	uint which = 0;

	ComplexFloatVector m_hTilde;
	ComplexFloatVector m_hTilde_dx;
	ComplexFloatVector m_hTilde_dy;
	ComplexFloatVector m_slopeX;
	ComplexFloatVector m_slopeY;

	std::vector<WaveSurfaceVertex>	m_waveSurfaceVerts;

	std::vector<ComplexFloatVector> m_c;
	std::vector<WaveSurfaceVertexVector> m_switchArray;
	std::vector<ComplexFloatVector> m_Ts;

	std::vector<uint> m_bitReversedIndices;
};