#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"


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