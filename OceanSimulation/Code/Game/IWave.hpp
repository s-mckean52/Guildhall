#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

class FFTWaveSimulation;
class WaterObject;

//---------------------------------------------------------------------------------------------------------
struct IWaveData
{
	float m_height				= 0.f;
	float m_previousHeight		= 0.f;
	float m_verticalDerivative	= 0.f;
};


//---------------------------------------------------------------------------------------------------------
class IWave
{
public:
	IWave( FFTWaveSimulation* owner, Vec2 const& dimensions, int xSamples, int ySamples );
	~IWave();

	void	Update( float deltaSeconds );
	void	GetHeightFromSimulation( int index );
	void	ApplyHeightsToSimulation();

	void	AddSource( int index, float value );
	void	ClearSources();

	void	CreateKernel( int kernelSize = 6 );
	float	GetKernelValue( int xPos, int yPos );
	void	CalculateNewHeights( float deltaSeconds );

	void	ConvolveHeights();
	float	GetConvolutionValue( int index );
	float	GetHeightAtIndex( int index );

	void	AddWaterObject( WaterObject* waterObjectToAdd );
	
	void ResetHeightsToZero();

private:
	FFTWaveSimulation* m_owner = nullptr;

	Vec2	m_deltas;
	IntVec2 m_gridDimensions;
	int		m_kernelSize = 0;
	int		m_kernelDimension = 0;

	std::vector<float>		m_kernelValuesLookUp;
	std::vector<float>		m_sources;
	std::vector<float>		m_obstructions;
	std::vector<IWaveData>	m_samplePoints;
};