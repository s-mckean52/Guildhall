#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RawNoise.hpp"
#include "Engine/Math/Vec2.hpp"
#include <stdlib.h>


//---------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntLessThan( int maxExclusive )
{
	unsigned int randomBits = Get1dNoiseUint( m_position++, m_seed );
	return randomBits % maxExclusive;
}


//---------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	int range = maxInclusive - minInclusive + 1;
	return minInclusive + RollRandomIntLessThan( range );
}


//---------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToOneInclusive()
{
	constexpr double scale = 1.0 / static_cast<double>( 0xFFFFFFFF );
	double randomBits = static_cast<double>( Get1dNoiseUint( m_position++, m_seed ) );
	return static_cast<float>( randomBits * scale );
}


//---------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToAlmostOne()
{
	constexpr double scale = 1.0 / ( static_cast<double>( 0xFFFFFFFF ) + 1.0 );
	double randomBits = static_cast<double>( Get1dNoiseUint( m_position++, m_seed ) );
	return static_cast<float>( randomBits * scale );
}


//---------------------------------------------------------------------------------------------------------
bool RandomNumberGenerator::RollPercentChance( float probabilityOfReturningTrue )
{
	float randomPercent = RollRandomFloatZeroToOneInclusive();
	if( randomPercent < probabilityOfReturningTrue )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
Vec2 RandomNumberGenerator::RollRandomDirection2D()
{
	float randomDirectionX = RollRandomFloatZeroToOneInclusive();
	float randomDirectionY = RollRandomFloatZeroToOneInclusive();

	Vec2 randomDirection = Vec2( randomDirectionX, randomDirectionY );
	return randomDirection.GetNormalized();
}


//---------------------------------------------------------------------------------------------------------
void RandomNumberGenerator::Reset( unsigned int seed )
{
	m_position	= 0;
	m_seed		= seed;
}


//---------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatLessThan( float maxExclusive )
{
	constexpr double scale = 1.0 / ( static_cast<double>( 0xFFFFFFFF ) + 1.0 );
	double randomBits = static_cast<double>( Get1dNoiseUint( m_position++, m_seed ) );
	return static_cast<float>( maxExclusive * randomBits * scale );
}


//---------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatInRange( float minInclusive, float maxInclusive )
{
	constexpr double scale = 1.0 / static_cast<double>( 0xFFFFFFFF );
	double range = static_cast<double>( maxInclusive ) - static_cast<double>( minInclusive );
	double randomBits = static_cast<double>( Get1dNoiseUint( m_position++, m_seed ) );
	return static_cast<float>( minInclusive + ( range * randomBits * scale ) );
}
