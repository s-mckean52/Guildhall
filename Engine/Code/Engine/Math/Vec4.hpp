#pragma once

struct Vec2;
struct Vec3;

struct Vec4
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:
	~Vec4() {};
	Vec4( const Vec4& toCopy );
	Vec4( const Vec3& toCopy, float initialW );
	Vec4( const Vec2& toCopyFirst, const Vec2& toCopySecond );
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );

	bool		operator==( const Vec4& toCompare ) const;
	bool		operator!=( const Vec4& toCompare ) const;
	const Vec4	operator+( const Vec4& vecToAdd ) const;
	const Vec4	operator-( const Vec4& vecToSubtract ) const;
	const Vec4	operator-() const;
	const Vec4	operator*( const Vec4& vecToMultiply ) const;
	const Vec4	operator*( float uniformScale ) const;
	const Vec4	operator/( float inverseScale ) const;

	void		operator+=( const Vec4& vecToAdd );
	void		operator-=( const Vec4& vecToSubtract );
	void		operator*=( const float uniformScale );
	void		operator/=( const float inverseScale );
	void		operator=( const Vec4& vecToCopy );

	friend const Vec4 operator*( float uniformScale, const Vec4& vecToScale );
};