#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
void Transform::SetPosition( Vec3 const& position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Transform::SetRotationFromPitchYawRollDegrees( float pitchDegrees, float yawDegrees, float rollDegrees )
{
	float halfPitchRange = 90.f;
	float halfRollRange = 180.f;
	float halfYawRange = 180.f;

	Clamp( pitchDegrees, -halfPitchRange, halfPitchRange );

	while( yawDegrees < -halfYawRange || yawDegrees > halfYawRange )
	{
		if( yawDegrees < -halfYawRange )
		{
			yawDegrees += 360.f;
		}
		else if( yawDegrees > halfYawRange )
		{
			yawDegrees -= 360.f;
		}
	}

	while( rollDegrees < -halfRollRange || rollDegrees > halfRollRange )
	{
		if( rollDegrees < -halfRollRange )
		{
			rollDegrees += 360.f;
		}
		else if( rollDegrees > halfRollRange )
		{
			rollDegrees -= 360.f;
		}
	}
	m_rotationPitchYawRollDegrees = Vec3( pitchDegrees, yawDegrees, rollDegrees );
}


//---------------------------------------------------------------------------------------------------------
void Transform::AddRotationPitchYawRollDegrees( float pitchDegrees, float yawDegrees, float rollDegrees )
{
	m_rotationPitchYawRollDegrees.x += pitchDegrees;
	m_rotationPitchYawRollDegrees.y += yawDegrees;
	m_rotationPitchYawRollDegrees.z += rollDegrees;
}


//---------------------------------------------------------------------------------------------------------
void Transform::Translate( Vec3 const& translation )
{
	m_position += translation;
}


//---------------------------------------------------------------------------------------------------------
Mat44 Transform::ToMatrix() const
{
	Mat44 transformationMatrix = Mat44::CreateTranslationXYZ( m_position );

	RotateMatrixPitchYawRollDegrees( transformationMatrix, m_rotationPitchYawRollDegrees );

	transformationMatrix.ScaleNonUniform3D( m_scale );
	
	return transformationMatrix;
}

