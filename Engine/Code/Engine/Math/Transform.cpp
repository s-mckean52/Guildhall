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

