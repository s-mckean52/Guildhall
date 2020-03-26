#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"


//---------------------------------------------------------------------------------------------------------
OBB3::OBB3( const OBB3& copyFrom )
{
	m_transformMatrix	= copyFrom.m_transformMatrix;
	m_halfDimensions	= copyFrom.m_halfDimensions;
}


//---------------------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& pitchYawRollDegrees )
{
	RotateMatrixPitchYawRollDegrees( m_transformMatrix, pitchYawRollDegrees );
	m_transformMatrix.SetTranslation3D( center );

	m_halfDimensions = fullDimensions * 0.5f;
}


//---------------------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& fullDimensions, const Mat44& transformMatrix )
{
	m_halfDimensions = fullDimensions * 0.5f;
	m_transformMatrix = transformMatrix;
}


//---------------------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal, const Vec3& jBasisNormal, const Vec3& kBasisNormal )
{
	m_halfDimensions = fullDimensions * 0.5f;
	m_transformMatrix = Mat44( iBasisNormal, jBasisNormal, kBasisNormal, center );
}


//---------------------------------------------------------------------------------------------------------
OBB3::OBB3( const AABB3& asAxisAlignedBox, const Vec3& pitchYawRollDegrees )
{
	RotateMatrixPitchYawRollDegrees( m_transformMatrix, pitchYawRollDegrees );
	m_transformMatrix.SetTranslation3D( asAxisAlignedBox.GetCenter() );

	m_halfDimensions = asAxisAlignedBox.GetDimensions() * 0.5f;
}


//---------------------------------------------------------------------------------------------------------
const Vec3 OBB3::GetCenter() const
{
	return m_transformMatrix.GetTranslation3D();
}


//---------------------------------------------------------------------------------------------------------
const Vec3 OBB3::GetDimensions() const
{
	return m_halfDimensions * 2.f;
}


//---------------------------------------------------------------------------------------------------------
const Vec3 OBB3::GetIBasisNormal() const
{
	Vec3 iBasis = m_transformMatrix.GetIBasis3D();
	if( iBasis == Vec3::ZERO )
	{
		return Vec3::RIGHT;
	}
	return iBasis.GetNormalize();
}


//---------------------------------------------------------------------------------------------------------
const Vec3 OBB3::GetJBasisNormal() const
{
	Vec3 jBasis = m_transformMatrix.GetJBasis3D();
	if( jBasis == Vec3::ZERO )
	{
		return Vec3::UP;
	}
	return jBasis.GetNormalize();
}


//---------------------------------------------------------------------------------------------------------
const Vec3 OBB3::GetKBasisNormal() const
{
	Vec3 kBasis = m_transformMatrix.GetKBasis3D();
	if( kBasis == Vec3::ZERO )
	{
		return -Vec3::FORWARD;
	}
	return kBasis.GetNormalize();
}


//---------------------------------------------------------------------------------------------------------
void OBB3::GetCornerPositions( Vec3* out_eightPoints ) const
{
	Vec3 distanceFromCenterToRight	= GetIBasisNormal() * m_halfDimensions.x;
	Vec3 distanceFromCenterToTop	= GetJBasisNormal() * m_halfDimensions.y;
	Vec3 distanceFromCenterToFront	= GetKBasisNormal() * m_halfDimensions.z;

	Vec3 centerPosition = GetCenter();

	out_eightPoints[ 0 ] = centerPosition - distanceFromCenterToRight - distanceFromCenterToTop + distanceFromCenterToFront;
	out_eightPoints[ 1 ] = centerPosition + distanceFromCenterToRight - distanceFromCenterToTop + distanceFromCenterToFront;
	out_eightPoints[ 2 ] = centerPosition + distanceFromCenterToRight + distanceFromCenterToTop + distanceFromCenterToFront;
	out_eightPoints[ 3 ] = centerPosition - distanceFromCenterToRight + distanceFromCenterToTop + distanceFromCenterToFront;

	out_eightPoints[ 4 ] = centerPosition - distanceFromCenterToRight - distanceFromCenterToTop - distanceFromCenterToFront;
	out_eightPoints[ 5 ] = centerPosition + distanceFromCenterToRight - distanceFromCenterToTop - distanceFromCenterToFront;
	out_eightPoints[ 6 ] = centerPosition + distanceFromCenterToRight + distanceFromCenterToTop - distanceFromCenterToFront;
	out_eightPoints[ 7 ] = centerPosition - distanceFromCenterToRight + distanceFromCenterToTop - distanceFromCenterToFront;
}


//---------------------------------------------------------------------------------------------------------
void OBB3::Translate( const Vec3& translation )
{
	m_transformMatrix.Translate3D( translation );
}


//---------------------------------------------------------------------------------------------------------
void OBB3::SetCenter( const Vec3& newCenter )
{
	m_transformMatrix.SetTranslation3D( newCenter );
}


//---------------------------------------------------------------------------------------------------------
void OBB3::SetDimensions( const Vec3& newFullDimensions )
{
	m_halfDimensions = newFullDimensions * 0.5f;
}


//---------------------------------------------------------------------------------------------------------
void OBB3::SetOrientationDegrees( const Vec3& newPitchYawRollDegrees )
{
	Mat44 newTransform = Mat44::IDENTITY;
	RotateMatrixPitchYawRollDegrees( newTransform, newPitchYawRollDegrees );
	newTransform.SetTranslation3D( m_transformMatrix.GetTranslation3D() );

	m_transformMatrix = newTransform;
}


//---------------------------------------------------------------------------------------------------------
void OBB3::RotatePitchYawRollDegrees(const Vec3& relativePitchYawRollDegrees)
{
	RotateMatrixPitchYawRollDegrees( m_transformMatrix, relativePitchYawRollDegrees );
}


//---------------------------------------------------------------------------------------------------------
void OBB3::Fix()
{
	m_halfDimensions.x = abs( m_halfDimensions.x );
	m_halfDimensions.y = abs( m_halfDimensions.y );
	m_halfDimensions.z = abs( m_halfDimensions.z );

	Vec3 iBasisNomal = GetIBasisNormal();
	Vec3 jBasisNormal = GetJBasisNormal();
	Vec3 kBasisNormal = GetKBasisNormal();

	m_transformMatrix.SetBasisVectors3D( iBasisNomal, jBasisNormal, kBasisNormal );
}


//---------------------------------------------------------------------------------------------------------
void OBB3::operator=( const OBB3& assignForm )
{
	m_transformMatrix = assignForm.m_transformMatrix;
	m_halfDimensions = assignForm.m_halfDimensions;
}
