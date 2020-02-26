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
void Transform::SetRotationFromPitchYawRollDegrees( float pitch, float yaw, float roll )
{
	float halfPitchRange = 90.f;
	float halfRollRange = 180.f;
	float halfYawRange = 180.f;

	Clamp( pitch, -halfPitchRange, halfPitchRange );

	while( yaw < -halfYawRange || yaw > halfYawRange )
	{
		if( yaw < -halfYawRange )
		{
			yaw += 360.f;
		}
		else if( yaw > halfYawRange )
		{
			yaw -= 360.f;
		}
	}

	while( roll < -halfRollRange || roll > halfRollRange )
	{
		if( roll < -halfRollRange )
		{
			roll += 360.f;
		}
		else if( roll > halfRollRange )
		{
			roll -= 360.f;
		}
	}
	m_rotationPitchYawRollDegrees = Vec3( pitch, yaw, roll );
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

