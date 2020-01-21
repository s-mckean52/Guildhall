#pragma once


struct IntVec2
{
public:
	int x = 0;
	int y = 0;

public:
	// Construction/Destruction
	~IntVec2() {}
	IntVec2() {}
	IntVec2( const IntVec2& copyFrom );
	explicit IntVec2( int initialX, int initialY );

	//Accessors
	float			GetLength() const;
	int				GetLengthSquared() const;
	int				GetTaxiCabLength() const;

	float			GetOrientationDegrees() const;
	float			GetOrientationRadians() const;
	const IntVec2	GetRotated90Degrees() const;
	const IntVec2	GetRotatedMinus90Degrees() const;

	//Mutators
	void	SetFromText( const char* text );
	void	Rotate90Degrees();
	void	RotateMinus90Degrees();

	// Operators (const)
	bool			operator==	( const IntVec2& compare ) const;			// IntVec2 == IntVec2
	bool			operator!=	( const IntVec2& compare ) const;			// IntVec2 != IntVec2
	const IntVec2	operator+	( const IntVec2& intVecToAdd ) const;		// IntVec2 + IntVec2
	const IntVec2	operator-	( const IntVec2& intVecToSubtract ) const;	// IntVec2 - IntVec2
	const IntVec2	operator-	() const;									// -IntVec2, i.e. "unary negation"
	const IntVec2	operator*	( int uniformScale ) const;					// IntVec2 * int
	const IntVec2	operator*	( const IntVec2& intVecToMultiply ) const;	// IntVec2 * IntVec2

	// Operators (self-mutating / non-const)
	void		operator+=	( const IntVec2& intVecToAdd );			// IntVec2 += IntVec2
	void		operator-=	( const IntVec2& vecToSubtract );		// IntVec2 -= IntVec2
	void		operator*=	( int uniformScale );					// IntVec2 *= int
	void		operator=	( const IntVec2& copyFrom );			// IntVec2 = IntVec2

	// Standalone "friend" functions that are conceptually, but not actually, part of IntVec2::
	friend const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale );	// int * IntVec2
};
