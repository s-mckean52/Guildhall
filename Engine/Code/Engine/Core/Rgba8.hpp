#pragma once

struct Vec4;

struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

public:
	~Rgba8() {}
	Rgba8() {}
	Rgba8( const Rgba8& copyFrom);
	explicit Rgba8( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255 );
	explicit Rgba8( Vec4 const& fractions );

	void SetFromText( const char* text );

// 	const Rgba8 operator-( const Rgba8& colorToSubtract ) const;
// 	const Rgba8 operator+( const Rgba8& colorToAdd ) const;
// 	const Rgba8 operator*( float fraction ) const;
// 	void operator*=( float fraction );
	void operator=( const Rgba8& copyFrom );
	
// Static Colors
	static const Rgba8 BLACK;
	static const Rgba8 WHITE;

	static const Rgba8 RED;
	static const Rgba8 GREEN;
	static const Rgba8 BLUE;

	static const Rgba8 YELLOW;
	static const Rgba8 MAGENTA;
	static const Rgba8 CYAN;

	static const Rgba8 GRAY;
	static const Rgba8 ORANGE;
};