#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ )
{
	Vec3 min = Vec3( minX, minY, minZ );
	Vec3 max = Vec3( maxX, maxY, maxZ );

	return Mat44::CreateOrthographicProjection( min, max );
}


//---------------------------------------------------------------------------------------------------------
Mat44 MakePerspectiveProjectionMatrixD3D( float fieldOfViewDegrees, float aspectRatio, float nearZ, float farZ )
{
	return Mat44::CreatePerspectiveProjection( fieldOfViewDegrees, aspectRatio, nearZ, farZ );
}


//---------------------------------------------------------------------------------------------------------
void RotateMatrixPitchYawRollDegrees( Mat44& matrixToRotate, float pitchDegrees, float yawDegrees, float rollDegrees )
{
// 	Mat44 xRotation = Mat44::CreateXRotationDegrees( pitchDegrees );
// 	Mat44 yRotation = Mat44::CreateYRotationDegrees( yawDegrees );
// 	Mat44 zRotation = Mat44::CreateZRotationDegrees( rollDegrees );
// 
// 	matrixToRotate.TransformBy( yRotation );
// 	matrixToRotate.TransformBy( xRotation );
// 	matrixToRotate.TransformBy( zRotation );

	matrixToRotate.RotateYDegrees( yawDegrees );
	matrixToRotate.RotateXDegrees( pitchDegrees );
	matrixToRotate.RotateZDegrees( rollDegrees );
}


//---------------------------------------------------------------------------------------------------------
void RotateMatrixPitchYawRollDegrees( Mat44& matrixToRotate, Vec3 const& pitchYawRollDegrees )
{
	RotateMatrixPitchYawRollDegrees( matrixToRotate, pitchYawRollDegrees.x, pitchYawRollDegrees.y, pitchYawRollDegrees.z );
}


//---------------------------------------------------------------------------------------------------------
bool MatrixIsOrthoNormal( Mat44 const& matrix )
{
	Vec3 iBasis = matrix.GetIBasis3D();
	Vec3 jBasis = matrix.GetJBasis3D();
	Vec3 kBasis = matrix.GetKBasis3D();

	float iBasisLength = iBasis.GetLength();
	float jBasisLength = jBasis.GetLength();
	float kBasisLength = kBasis.GetLength();

	if( !ApproximatelyEqual( iBasisLength, 1.f ) || 
		!ApproximatelyEqual( jBasisLength, 1.f ) || 
		!ApproximatelyEqual( kBasisLength, 1.f ) )
	{
		return false;
	}

	float iBasisDotJBasis = DotProduct3D( iBasis, jBasis );
	float iBasisDotKBasis = DotProduct3D( iBasis, kBasis );
	float jBasisDotKBasis = DotProduct3D( jBasis, kBasis );

	if( !ApproximatelyEqual( iBasisDotJBasis, 0.f ) ||
		!ApproximatelyEqual( iBasisDotKBasis, 0.f ) || 
		!ApproximatelyEqual( jBasisDotKBasis, 0.f ) )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
void MatrixTranspose( Mat44& matrix )
{
	float mat[] = {
		matrix.Ix,	matrix.Iy,	matrix.Iz,	matrix.Iw,
		matrix.Jx,	matrix.Jy,	matrix.Jz,	matrix.Jw,
		matrix.Kx,	matrix.Ky,	matrix.Kz,	matrix.Kw,
		matrix.Tx,	matrix.Ty,	matrix.Tz,	matrix.Tw
	};

	matrix = Mat44( mat );
}


//---------------------------------------------------------------------------------------------------------
void MatrixInvertOrthoNormal( Mat44& matrix )
{
	GUARANTEE_OR_DIE( MatrixIsOrthoNormal( matrix ), "Tried to invert non-orthonormal matrix with InvertOrthoNormal");

	Mat44 inverseMatrix = matrix;
	inverseMatrix.SetTranslation3D( Vec3( 0.0f ) );
	MatrixTranspose( inverseMatrix );

	Vec3 translation = matrix.GetTranslation3D();
	Vec3 inverseTranslation = inverseMatrix.TransformPosition3D( -translation );
	inverseMatrix.SetTranslation3D( inverseTranslation );

	matrix = inverseMatrix;
}


//---------------------------------------------------------------------------------------------------------
// Lifted from GLU
// assuming memory is going (ix, jx, kx, tx, iy, jy, ...)
void MatrixInvert( Mat44& matrix )
{
	double inv[16];
	double det;
	double m[16];
	unsigned int i;

	MatrixTranspose( matrix );
	float* matData = matrix.GetAsFloatArray();

	for( i = 0; i < 16; ++i ) {
		m[i] = (double)matData[i];
	}

	inv[0] = m[5]  * m[10] * m[15] -
		m[5]  * m[11] * m[14] -
		m[9]  * m[6]  * m[15] +
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] -
		m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
		m[4]  * m[11] * m[14] +
		m[8]  * m[6]  * m[15] -
		m[8]  * m[7]  * m[14] -
		m[12] * m[6]  * m[11] +
		m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9]  * m[15] -
		m[4]  * m[11] * m[13] -
		m[8]  * m[5]  * m[15] +
		m[8]  * m[7]  * m[13] +
		m[12] * m[5]  * m[11] -
		m[12] * m[7]  * m[9];

	inv[12] = -m[4]  * m[9]  * m[14] +
		m[4]  * m[10] * m[13] +
		m[8]  * m[5]  * m[14] -
		m[8]  * m[6]  * m[13] -
		m[12] * m[5]  * m[10]  +
		m[12] * m[6]  * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
		m[1]  * m[11] * m[14] +
		m[9]  * m[2]  * m[15] -
		m[9]  * m[3]  * m[14] -
		m[13] * m[2]  * m[11] +
		m[13] * m[3]  * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
		m[0]  * m[11] * m[14] -
		m[8]  * m[2]  * m[15] +
		m[8]  * m[3]  * m[14] +
		m[12] * m[2]  * m[11] -
		m[12] * m[3]  * m[10];

	inv[9] = -m[0]  * m[9]  * m[15] +
		m[0]  * m[11] * m[13] +
		m[8]  * m[1]  * m[15] -
		m[8]  * m[3]  * m[13] -
		m[12] * m[1]  * m[11] +
		m[12] * m[3]  * m[9];

	inv[13] = m[0]  * m[9]  * m[14] -
		m[0]  * m[10] * m[13] -
		m[8]  * m[1]  * m[14] +
		m[8]  * m[2]  * m[13] +
		m[12] * m[1]  * m[10] -
		m[12] * m[2]  * m[9];

	inv[2] = m[1]  * m[6] * m[15] -
		m[1]  * m[7] * m[14] -
		m[5]  * m[2] * m[15] +
		m[5]  * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] +
		m[0]  * m[7] * m[14] +
		m[4]  * m[2] * m[15] -
		m[4]  * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] -
		m[0]  * m[7] * m[13] -
		m[4]  * m[1] * m[15] +
		m[4]  * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] +
		m[0]  * m[6] * m[13] +
		m[4]  * m[1] * m[14] -
		m[4]  * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.0 / det;

	float retData[ 16 ];
	for( i = 0; i < 16; i++ ) {
		retData[i] = (float)(inv[i] * det);
	}

	Mat44 ret = Mat44( retData );
	MatrixTranspose( ret );

	matrix = ret;
}

