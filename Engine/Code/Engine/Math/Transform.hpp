#pragma once
#include "Engine/Math/Vec3.hpp"

struct Mat44;


class Transform
{
public:
	Vec3 GetPosition() const						{ return m_position; }
	Vec3 GetScale()	const							{ return m_scale; }
	Vec3 GetRotationPitchYawRollDegrees() const		{ return m_rotationPitchYawRollDegrees; }

	void SetPosition( Vec3 const& position );
	void SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw );

	void Translate( Vec3 const& translation );

	Mat44 ToMatrix() const;

private:
	Vec3 m_position						= Vec3( 0.0f );
	Vec3 m_rotationPitchYawRollDegrees	= Vec3( 0.0f );
	Vec3 m_scale						= Vec3( 1.0f );
};