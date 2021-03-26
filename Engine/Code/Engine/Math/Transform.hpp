#pragma once
#include "Engine/Math/Vec3.hpp"

struct Mat44;


class Transform
{
public:
	//explicit Transform();
	explicit Transform( Vec3 const& position = Vec3::ZERO, Vec3 const& rotationPitchYawRollDegrees = Vec3::ZERO, Vec3 const& scale = Vec3::ONES );

	Vec3 GetPosition() const						{ return m_position; }
	Vec3 GetScale()	const							{ return m_scale; }
	Vec3 GetRotationPitchYawRollDegrees() const		{ return m_rotationPitchYawRollDegrees; }

	void SetPosition( Vec3 const& position );
	void SetYawDegrees( float yawDegrees );
	void SetRotationFromPitchYawRollDegrees( float pitchDegrees, float yawDegrees, float rollDegrees );
	void SetUniformScale( float uniformScale );

	void AddRotationPitchYawRollDegrees( float pitchDegrees, float yawDegrees, float rollDegrees );

	void Translate( Vec3 const& translation );

	Mat44 ToMatrix() const;

private:
	Vec3 m_position						= Vec3( 0.0f );
	Vec3 m_rotationPitchYawRollDegrees	= Vec3( 0.0f );
	Vec3 m_scale						= Vec3( 1.0f );
};