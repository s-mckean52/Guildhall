#pragma once

struct Vec2;

struct Vec3
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

public:
	~Vec3() {}
	Vec3() {}
	Vec3( const Vec3& copyFrom );
	Vec3( const Vec2& copyFrom, float z );
	Vec3( float initialX, float initialY, float initialZ);
	Vec3( float initialXYZ );

	//Accessors
	float		GetLength() const;
	float		GetLengthXY() const;
	float		GetLengthSquared() const;
	float		GetLengthXYSquared() const;
	float		GetAngleAboutZRadians() const;
	float		GetAngleAboutZDegrees() const;
	const Vec3	GetRotatedAboutZRadians( float deltaRadians ) const;
	const Vec3	GetRotatedAboutZDegrees( float deltaDegrees ) const;
	const Vec3	GetClamped( float maxLength );
	const Vec3	GetNormalize() const;

	//Operators
	bool		operator==( const Vec3& compare ) const;
	bool		operator!=( const Vec3& compare ) const;
	const Vec3	operator+( const Vec3& vecToAdd ) const;
	const Vec3	operator-( const Vec3& vecToSubtract ) const;
	const Vec3	operator-() const;
	const Vec3	operator/( float inverseScale ) const;
	const Vec3	operator*( float uniformScale ) const;
	const Vec3	operator*( const Vec3& vecToMultiply ) const;

	void		operator+=( const Vec3& vecToAdd );
	void		operator-=( const Vec3& vecToSubtract );
	void		operator/=( const float inverseScale );
	void 		operator*=( const float uniformScale );
	void 		operator=( const Vec3& copyFrom );

	//Standalone friend functions
	friend const Vec3 operator*( float uniformScale, const Vec3& vecToScale );
};