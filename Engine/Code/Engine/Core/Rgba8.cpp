#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"


//---------------------------------------------------------------------------------------------------------
Rgba8::Rgba8( const Rgba8& copyFrom )
{
	r = copyFrom.r;
	g = copyFrom.g;
	b = copyFrom.b;
	a = copyFrom.a;
}


//---------------------------------------------------------------------------------------------------------
Rgba8::Rgba8( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha )
{
	r = red;
	g = green;
	b = blue;
	a = alpha;
}


//---------------------------------------------------------------------------------------------------------
Rgba8::Rgba8( Vec4 const& fractions )
{
	r = static_cast<unsigned char>( 255.f * fractions.x );
	g = static_cast<unsigned char>( 255.f * fractions.y );
	b = static_cast<unsigned char>( 255.f * fractions.z );
	a = static_cast<unsigned char>( 255.f * fractions.w );
}


//---------------------------------------------------------------------------------------------------------
STATIC Rgba8 Rgba8::MakeFromFloats( float red, float green, float blue, float alpha )
{
	Rgba8 fromFloats;
	fromFloats.r = static_cast<unsigned char>( 255.f * red );
	fromFloats.g = static_cast<unsigned char>( 255.f * green );
	fromFloats.b = static_cast<unsigned char>( 255.f * blue );
	fromFloats.a = static_cast<unsigned char>( 255.f * alpha );
	return fromFloats;
}


//---------------------------------------------------------------------------------------------------------
Vec4 Rgba8::GetValuesAsFractions() const
{
	Vec4 colorAsFloats;
	colorAsFloats.x = static_cast<float>( r ) / 255.f;
	colorAsFloats.y = static_cast<float>( g ) / 255.f;
	colorAsFloats.z = static_cast<float>( b ) / 255.f;
	colorAsFloats.w = static_cast<float>( a ) / 255.f;

	return colorAsFloats;
}


//---------------------------------------------------------------------------------------------------------
void Rgba8::SetFromText( const char* text )
{
	Strings splitText = SplitStringOnDelimiter( text, ',' );

	GUARANTEE_OR_DIE( splitText.size() >= 3 && splitText.size() <= 4, Stringf( "Rgba8 SetFromText invalid number of values, %i values were passed", splitText.size() ) );

	r = static_cast<unsigned char>( atoi( splitText[ 0 ].c_str() ) );
	g = static_cast<unsigned char>( atoi( splitText[ 1 ].c_str() ) );
	b = static_cast<unsigned char>( atoi( splitText[ 2 ].c_str() ) );

	if( splitText.size() == 4 )
	{
		a = static_cast<unsigned char>( atoi( splitText[ 3 ].c_str() ) );
	}
	else
	{
		a = 255;
	}
}


//---------------------------------------------------------------------------------------------------------
void Rgba8::operator=( const Rgba8& copyFrom )
{
	r = copyFrom.r;
	g = copyFrom.g;
	b = copyFrom.b;
	a = copyFrom.a;
}


// ---------------------------------------------------------------------------------------------------------
// const Rgba8 Rgba8::operator-( const Rgba8& colorToSubtract ) const
// {
// 	return Rgba8( r - colorToSubtract.r, g - colorToSubtract.g, b - colorToSubtract.b, a - colorToSubtract.a );
// }
// 
// 
// ---------------------------------------------------------------------------------------------------------
// const Rgba8 Rgba8::operator+( const Rgba8& colorToAdd ) const
// {
// 	return Rgba8( r + colorToAdd.r, g + colorToAdd.g, b + colorToAdd.b, a + colorToAdd.a );
// }
// 
// 
// ---------------------------------------------------------------------------------------------------------
// void Rgba8::operator*=( float fraction )
// {
// 	r = static_cast<unsigned char>( static_cast<float>( r ) * fraction );
// 	g = static_cast<unsigned char>( static_cast<float>( g ) * fraction );
// 	b = static_cast<unsigned char>( static_cast<float>( b ) * fraction );
// 	a = static_cast<unsigned char>( static_cast<float>( a ) * fraction );
// }
// 
// 
// ---------------------------------------------------------------------------------------------------------
// const Rgba8 Rgba8::operator*( float fraction ) const
// {
// 	Rgba8 returnColor;
// 	returnColor.r = static_cast<unsigned char>( static_cast<float>( r ) * fraction );
// 	returnColor.g = static_cast<unsigned char>( static_cast<float>( g ) * fraction );
// 	returnColor.b = static_cast<unsigned char>( static_cast<float>( b ) * fraction );
// 	returnColor.a = static_cast<unsigned char>( static_cast<float>( a ) * fraction );
// 	return returnColor;
// }


// Static Colors
const Rgba8 Rgba8::BLACK( 0, 0, 0 );
const Rgba8 Rgba8::WHITE( 255, 255, 255 );

const Rgba8 Rgba8::RED( 255, 0, 0 );
const Rgba8 Rgba8::GREEN( 0, 255, 0 );
const Rgba8 Rgba8::BLUE( 0, 0, 255 );

const Rgba8 Rgba8::YELLOW( 255, 255, 0 );
const Rgba8 Rgba8::MAGENTA( 255, 0, 255 );
const Rgba8 Rgba8::CYAN( 0, 255, 255 );

const Rgba8 Rgba8::GRAY( 50, 50, 50 );
const Rgba8 Rgba8::ORANGE( 241, 90, 34 );