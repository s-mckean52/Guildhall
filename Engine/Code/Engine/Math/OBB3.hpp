#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"

struct AABB3;

struct OBB3
{
	Mat44	m_transformMatrix;
	Vec3	m_halfDimensions;

	// Construction / destructor
	OBB3() = default;
	~OBB3() = default;
	OBB3( const OBB3& copyFrom );
	explicit OBB3( const Vec3& fullDimensions, const Mat44& transformMatrix = Mat44::IDENTITY );
	explicit OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal = Vec3::UNIT_POSITIVE_X, const Vec3& jBasisNormal = Vec3::UNIT_POSITIVE_Y, const Vec3& kBasisNormal = Vec3::UNIT_POSITIVE_Z );
	explicit OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& pitchYawRollDegrees );
	explicit OBB3( const AABB3& asAxisAlignedBox, const Vec3& pitchYawRollDegrees = Vec3::ZERO );

	// Accessors (const methods)
	const Vec3	GetCenter() const;
	const Vec3	GetDimensions() const;
	const Vec3	GetIBasisNormal() const;
	const Vec3	GetJBasisNormal() const;
	const Vec3	GetKBasisNormal() const;
	void		GetCornerPositions( Vec3* out_eightPoints ) const;

	// Mutators
	void	Translate( const Vec3& translation );
	void	SetCenter( const Vec3& newCenter );
	void	SetDimensions( const Vec3& newFullDimensions );
	void	SetOrientationDegrees( const Vec3& newPitchYawRollDegrees );
	void	RotatePitchYawRollDegrees( const Vec3& relativePitchYawRollDegrees );
	void	Fix();

	// Operators
	void	operator=( const OBB3& assignForm );
};