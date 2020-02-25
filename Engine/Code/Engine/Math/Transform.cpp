#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MatrixUtils.hpp"


//---------------------------------------------------------------------------------------------------------
void Transform::SetPosition( Vec3 const& position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Transform::SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	float halfPitchRange = 90.f;
	float halfRollRange = 180.f;
	float halfYawRange = 180.f;

	while( pitch < -halfPitchRange || pitch > halfPitchRange )
	{
		if( pitch < -halfPitchRange )
		{
			pitch += halfPitchRange * 2.f;
		}
		else if( pitch > halfPitchRange )
		{
			pitch -= halfPitchRange * 2.f;
		}
	}

	while( roll < -halfRollRange || roll > halfRollRange )
	{
		if( roll < -halfRollRange )
		{
			roll += halfRollRange * 2.f;
		}
		else if( roll > halfRollRange )
		{
			roll -= halfRollRange * 2.f;
		}
	}

	while( yaw < -halfYawRange || yaw > halfYawRange )
	{
		if( yaw < -halfYawRange )
		{
			yaw += halfYawRange * 2.f;
		}
		else if( yaw > halfYawRange )
		{
			yaw -= halfYawRange * 2.f;
		}
	}

	m_rotationPitchYawRollDegrees = Vec3( pitch, roll, yaw );
}


//---------------------------------------------------------------------------------------------------------
void Transform::Translate( Vec3 const& translation )
{
	m_position += translation;
}


//---------------------------------------------------------------------------------------------------------
Mat44 Transform::ToMatrix() const
{
	Mat44 transformationMatrix = Mat44::CreateNonUniformScaleXYZ( m_scale );
	RotateMatrixPitchYawRollDegrees( transformationMatrix, m_rotationPitchYawRollDegrees );
	transformationMatrix.Translate3D( m_position );
	
	return transformationMatrix;
}

