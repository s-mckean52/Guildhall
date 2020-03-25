#pragma once

//-----------------------------------------------------------------------------------------------
struct Vec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

public:
	// Construction/Destruction
	~Vec2() {}												// destructor (do nothing)
	Vec2() {}												// default constructor (do nothing)
	Vec2( const Vec2& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)

	//static methods
	static Vec2 MakeFromPolarDegrees( float directionDegrees, float length = 1.f );
	static Vec2 MakeFromPolarRadians( float directionRadians, float length = 1.f );

	//Accessors
	float		GetLength() const;
	float		GetLengthSquared() const;
	float		GetAngleDegrees() const;
	float		GetAngleRadians() const;
	const Vec2	GetRotated90Degrees() const;
	const Vec2	GetRotatedMinus90Degrees() const;
	const Vec2	GetRotatedDegrees( float deltaDegrees ) const;
	const Vec2	GetRotatedRadians( float deltaRadians ) const;
	const Vec2	GetClamped( float maxLength ) const;
	const Vec2	GetNormalized() const;
	const Vec2	GetReflected( const Vec2& vectorToReflectAround ) const;

	//Mutators
	void		SetFromText		( const char* text );
	void		SetLength		( float newLength );
	void		SetAngleDegrees	( float newOrientationDegrees );
	void		SetAngleRadians	( float newOrientationRadians );
	void		SetPolarDegrees	( float newOrientationDegrees, float newLength );
	void		SetPolarRadians	( float newOrientationRadians, float newLength );
	void		RotateDegrees	( float deltaDegrees );
	void		RotateRadians	( float deltaRadians );
	
	void		Rotate90Degrees();
	void		RotateMinus90Degrees();
	void		ClampLength( float maxLength );
	void		Normalize();
	float		NormalizeAndGetPreviousLength();
	void		Reflect( const Vec2& vectorToReflectAround );


	// Operators (const)
	bool		operator==( const Vec2& compare ) const;		// vec2 == vec2
	bool		operator!=( const Vec2& compare ) const;		// vec2 != vec2
	const Vec2	operator+( const Vec2& vecToAdd ) const;		// vec2 + vec2
	const Vec2	operator-( const Vec2& vecToSubtract ) const;	// vec2 - vec2
	const Vec2	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec2	operator*( float uniformScale ) const;			// vec2 * float
	const Vec2	operator*( const Vec2& vecToMultiply ) const;	// vec2 * vec2
	const Vec2	operator/( float inverseScale ) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec2& vecToAdd );				// vec2 += vec2
	void		operator-=( const Vec2& vecToSubtract );		// vec2 -= vec2
	void		operator*=( const float uniformScale );			// vec2 *= float
	void		operator/=( const float uniformDivisor );		// vec2 /= float
	void		operator=( const Vec2& copyFrom );				// vec2 = vec2

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec2 operator*( float uniformScale, const Vec2& vecToScale );	// float * vec2


	//---------------------------------------------------------------------------------------------------------
	// Static Vec2
	static const Vec2 ZERO;
	static const Vec2 RIGHT;
	static const Vec2 LEFT;
	static const Vec2 UP;
	static const Vec2 DOWN;
	static const Vec2 UNIT;
};


