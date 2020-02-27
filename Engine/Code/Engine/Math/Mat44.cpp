#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"


const Mat44 Mat44::IDENTITY = Mat44();


//---------------------------------------------------------------------------------------------------------
Mat44::Mat44( float* sixteenValuesBasisMajor )
{
	Ix = sixteenValuesBasisMajor[ 0 ];
	Jx = sixteenValuesBasisMajor[ 1 ];
	Kx = sixteenValuesBasisMajor[ 2 ];
	Tx = sixteenValuesBasisMajor[ 3 ];

	Iy = sixteenValuesBasisMajor[ 4 ];
	Jy = sixteenValuesBasisMajor[ 5 ];
	Ky = sixteenValuesBasisMajor[ 6 ];
	Ty = sixteenValuesBasisMajor[ 7 ];
	
	Iz = sixteenValuesBasisMajor[ 8 ];
	Jz = sixteenValuesBasisMajor[ 9 ];
	Kz = sixteenValuesBasisMajor[ 10 ];
	Tz = sixteenValuesBasisMajor[ 11 ];
	
	Iw = sixteenValuesBasisMajor[ 12 ];
	Jw = sixteenValuesBasisMajor[ 13 ];
	Kw = sixteenValuesBasisMajor[ 14 ];
	Tw = sixteenValuesBasisMajor[ 15 ];
}


//---------------------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation2D )
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = 0.f;
	Iw = 0.f;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = 0.f;
	Jw = 0.f;

	Kx = 0.f;
	Ky = 0.f;
	Kz = 1.f;
	Kw = 0.f;

	Tx = translation2D.x;
	Ty = translation2D.y;
	Tz = 0.f;
	Tw = 1.f;
}


//---------------------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis, const Vec3& translation3D )
{
	Ix = iBasis.x;
	Iy = iBasis.y;
	Iz = iBasis.z;
	Iw = 0.f;

	Jx = jBasis.x;
	Jy = jBasis.y;
	Jz = jBasis.z;
	Jw = 0.f;

	Kx = kBasis.x;
	Ky = kBasis.y;
	Kz = kBasis.z;
	Kw = 0.f;

	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
	Tw = 1.f;
}


//---------------------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec4& iBasisHomogeneous, const Vec4& jBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous )
{
	Ix = iBasisHomogeneous.x;
	Iy = iBasisHomogeneous.y;
	Iz = iBasisHomogeneous.z;
	Iw = iBasisHomogeneous.w;

	Jx = jBasisHomogeneous.x;
	Jy = jBasisHomogeneous.y;
	Jz = jBasisHomogeneous.z;
	Jw = jBasisHomogeneous.w;

	Kx = kBasisHomogeneous.x;
	Ky = kBasisHomogeneous.y;
	Kz = kBasisHomogeneous.z;
	Kw = kBasisHomogeneous.w;

	Tx = translationHomogeneous.x;
	Ty = translationHomogeneous.y;
	Tz = translationHomogeneous.z;
	Tw = translationHomogeneous.w;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Mat44::TransformVector2D( const Vec2& vector ) const
{
	float xResult = ( Ix * vector.x ) + ( Jx * vector.y );
	float yResult = ( Iy * vector.x ) + ( Jy * vector.y );

	return Vec2( xResult, yResult );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::TransformVector3D( const Vec3& vector ) const
{
	float xResult = ( Ix * vector.x ) + ( Jx * vector.y ) + ( Kx * vector.z );
	float yResult = ( Iy * vector.x ) + ( Jy * vector.y ) + ( Ky * vector.z );
	float zResult = ( Iz * vector.x ) + ( Jz * vector.y ) + ( Kz * vector.z );

	return Vec3( xResult, yResult, zResult );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Mat44::TransformPosition2D( const Vec2& position ) const
{
	float xResult = ( Ix * position.x ) + ( Jx * position.y ) + Tx;
	float yResult = ( Iy * position.x ) + ( Jy * position.y ) + Ty;

	return Vec2( xResult, yResult );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::TransformPosition3D( const Vec3& position ) const
{
	float xResult = ( Ix * position.x ) + ( Jx * position.y ) + ( Kx * position.z ) + Tx;
	float yResult = ( Iy * position.x ) + ( Jy * position.y ) + ( Ky * position.z ) + Ty;
	float zResult = ( Iz * position.x ) + ( Jz * position.y ) + ( Kz * position.z ) + Tz;

	return Vec3( xResult, yResult, zResult );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Mat44::TransformHomogeneousPoint3D( const Vec4& point ) const
{
	float xResult = ( Ix * point.x ) + ( Jx * point.y ) + ( Kx * point.z ) + ( Tx * point.w );
	float yResult = ( Iy * point.x ) + ( Jy * point.y ) + ( Ky * point.z ) + ( Ty * point.w );
	float zResult = ( Iz * point.x ) + ( Jz * point.y ) + ( Kz * point.z ) + ( Tz * point.w );
	float wResult = ( Iw * point.x ) + ( Jw * point.y ) + ( Kw * point.z ) + ( Tw * point.w );

	return Vec4( xResult, yResult, zResult, wResult );
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::GetTransformMatrixBy( const Mat44& transformationMatrix ) const
{
	Mat44 returnMatrix = Mat44();

	returnMatrix.Ix = (Ix * transformationMatrix.Ix) + (Jx * transformationMatrix.Iy) + (Kx * transformationMatrix.Iz) + (Tx * transformationMatrix.Iw);
	returnMatrix.Jx = (Ix * transformationMatrix.Jx) + (Jx * transformationMatrix.Jy) + (Kx * transformationMatrix.Jz) + (Tx * transformationMatrix.Jw);
	returnMatrix.Kx = (Ix * transformationMatrix.Kx) + (Jx * transformationMatrix.Ky) + (Kx * transformationMatrix.Kz) + (Tx * transformationMatrix.Kw);
	returnMatrix.Tx = (Ix * transformationMatrix.Tx) + (Jx * transformationMatrix.Ty) + (Kx * transformationMatrix.Tz) + (Tx * transformationMatrix.Tw);

	returnMatrix.Iy = (Iy * transformationMatrix.Ix) + (Jy * transformationMatrix.Iy) + (Ky * transformationMatrix.Iz) + (Ty * transformationMatrix.Iw);
	returnMatrix.Jy = (Iy * transformationMatrix.Jx) + (Jy * transformationMatrix.Jy) + (Ky * transformationMatrix.Jz) + (Ty * transformationMatrix.Jw);
	returnMatrix.Ky = (Iy * transformationMatrix.Kx) + (Jy * transformationMatrix.Ky) + (Ky * transformationMatrix.Kz) + (Ty * transformationMatrix.Kw);
	returnMatrix.Ty = (Iy * transformationMatrix.Tx) + (Jy * transformationMatrix.Ty) + (Ky * transformationMatrix.Tz) + (Ty * transformationMatrix.Tw);

	returnMatrix.Iz = (Iz * transformationMatrix.Ix) + (Jz * transformationMatrix.Iy) + (Kz * transformationMatrix.Iz) + (Tz * transformationMatrix.Iw);
	returnMatrix.Jz = (Iz * transformationMatrix.Jx) + (Jz * transformationMatrix.Jy) + (Kz * transformationMatrix.Jz) + (Tz * transformationMatrix.Jw);
	returnMatrix.Kz = (Iz * transformationMatrix.Kx) + (Jz * transformationMatrix.Ky) + (Kz * transformationMatrix.Kz) + (Tz * transformationMatrix.Kw);
	returnMatrix.Tz = (Iz * transformationMatrix.Tx) + (Jz * transformationMatrix.Ty) + (Kz * transformationMatrix.Tz) + (Tz * transformationMatrix.Tw);

	returnMatrix.Iw = (Iw * transformationMatrix.Ix) + (Jw * transformationMatrix.Iy) + (Kw * transformationMatrix.Iz) + (Tw * transformationMatrix.Iw);
	returnMatrix.Jw = (Iw * transformationMatrix.Jx) + (Jw * transformationMatrix.Jy) + (Kw * transformationMatrix.Jz) + (Tw * transformationMatrix.Jw);
	returnMatrix.Kw = (Iw * transformationMatrix.Kx) + (Jw * transformationMatrix.Ky) + (Kw * transformationMatrix.Kz) + (Tw * transformationMatrix.Kw);
	returnMatrix.Tw = (Iw * transformationMatrix.Tx) + (Jw * transformationMatrix.Ty) + (Kw * transformationMatrix.Tz) + (Tw * transformationMatrix.Tw);

	return returnMatrix;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Mat44::GetIBasis2D() const
{
	return Vec2( Ix, Iy );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Mat44::GetJBasis2D() const
{
	return Vec2( Jx, Jy );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 Mat44::GetTranslation2D() const
{
	return Vec2( Tx, Ty );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::GetIBasis3D() const
{
	return Vec3( Ix, Iy, Iz );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::GetJBasis3D() const
{
	return Vec3( Jx, Jy, Jz );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::GetKBasis3D() const
{
	return Vec3( Kx, Ky, Kz );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 Mat44::GetTranslation3D() const
{
	return Vec3( Tx, Ty, Tz );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Mat44::GetIBasis4D() const
{
	return Vec4( Ix, Iy, Iz, Iw );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Mat44::GetJBasis4D() const
{
	return Vec4( Jx, Jy, Jz, Jw );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Mat44::GetKBasis4D() const
{
	return Vec4( Kx, Ky, Kz, Kw );
}


//---------------------------------------------------------------------------------------------------------
const Vec4 Mat44::GetTranslation4D() const
{
	return Vec4( Tx, Ty, Tz, Tw );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetTranslation2D( const Vec2& translation2D )
{
	Tx = translation2D.x;
	Ty = translation2D.y;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;

	Tx = translation2D.x;
	Ty = translation2D.y;
}

void Mat44::SetTranslation3D( const Vec3& translation3D )
{
	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;
	
	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;
	
	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors4D( const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D )
{
	Ix = iBasis4D.x;
	Iy = iBasis4D.y;
	Iz = iBasis4D.z;
	Iw = iBasis4D.w;

	Jx = jBasis4D.x;
	Jy = jBasis4D.y;
	Jz = jBasis4D.z;
	Jw = jBasis4D.w;

	Kx = kBasis4D.x;
	Ky = kBasis4D.y;
	Kz = kBasis4D.z;
	Kw = kBasis4D.w;

	Tx = translation4D.x;
	Ty = translation4D.y;
	Tz = translation4D.z;
	Tw = translation4D.w;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::RotateXDegrees( float degreesAboutX )
{
	float cosTheta = CosDegrees( degreesAboutX );
	float sinTheta = SinDegrees( degreesAboutX );

	Mat44 tempMat44 = *this;

	Jx = ( tempMat44.Jx * cosTheta ) + ( tempMat44.Kx * sinTheta );
	Jy = ( tempMat44.Jy * cosTheta ) + ( tempMat44.Ky * sinTheta );
	Jz = ( tempMat44.Jz * cosTheta ) + ( tempMat44.Kz * sinTheta );
	Jw = ( tempMat44.Jw * cosTheta ) + ( tempMat44.Kw * sinTheta );

	Kx = ( tempMat44.Jx * -sinTheta ) + ( tempMat44.Kx * cosTheta );
	Ky = ( tempMat44.Jy * -sinTheta ) + ( tempMat44.Ky * cosTheta );
	Kz = ( tempMat44.Jz * -sinTheta ) + ( tempMat44.Kz * cosTheta );
	Kw = ( tempMat44.Jw * -sinTheta ) + ( tempMat44.Kw * cosTheta );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::RotateYDegrees( float degreesAboutY )
{
	float cosTheta = CosDegrees( degreesAboutY );
	float sinTheta = SinDegrees( degreesAboutY );

	Mat44 tempMat44 = *this;

	Ix = ( tempMat44.Ix * cosTheta ) + ( tempMat44.Kx * -sinTheta );
	Iy = ( tempMat44.Iy * cosTheta ) + ( tempMat44.Ky * -sinTheta );
	Iz = ( tempMat44.Iz * cosTheta ) + ( tempMat44.Kz * -sinTheta );
	Iw = ( tempMat44.Iw * cosTheta ) + ( tempMat44.Kw * -sinTheta );

	Kx = ( tempMat44.Ix * sinTheta ) + ( tempMat44.Kx * cosTheta );
	Ky = ( tempMat44.Iy * sinTheta ) + ( tempMat44.Ky * cosTheta );
	Kz = ( tempMat44.Iz * sinTheta ) + ( tempMat44.Kz * cosTheta );
	Kw = ( tempMat44.Iw * sinTheta ) + ( tempMat44.Kw * cosTheta );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::RotateZDegrees( float degreesAboutZ )
{
	float cosTheta = CosDegrees( degreesAboutZ );
	float sinTheta = SinDegrees( degreesAboutZ );

	Mat44 tempMat44 = *this;

	Ix = ( tempMat44.Ix * cosTheta ) + ( tempMat44.Jx * sinTheta );
	Iy = ( tempMat44.Iy * cosTheta ) + ( tempMat44.Jy * sinTheta );
	Iz = ( tempMat44.Iz * cosTheta ) + ( tempMat44.Jz * sinTheta );
	Iw = ( tempMat44.Iw * cosTheta ) + ( tempMat44.Jw * sinTheta );

	Jx = ( tempMat44.Ix * -sinTheta ) + ( tempMat44.Jx * cosTheta );
	Jy = ( tempMat44.Iy * -sinTheta ) + ( tempMat44.Jy * cosTheta );
	Jz = ( tempMat44.Iz * -sinTheta ) + ( tempMat44.Jz * cosTheta );
	Jw = ( tempMat44.Iw * -sinTheta ) + ( tempMat44.Jw * cosTheta );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::Translate2D( const Vec2& translation2D )
{
	Mat44 tempMat44 = *this;

	Tx += ( tempMat44.Ix * translation2D.x ) + ( tempMat44.Jx * translation2D.y );
	Ty += ( tempMat44.Iy * translation2D.x ) + ( tempMat44.Jy * translation2D.y );
	Tz += ( tempMat44.Iz * translation2D.x ) + ( tempMat44.Jz * translation2D.y );
	Tw += ( tempMat44.Iw * translation2D.x ) + ( tempMat44.Jw * translation2D.y );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::ScaleUniform2D( float uniformScaleXY )
{
	Mat44 tempMat44 = *this;

	Ix *= uniformScaleXY;
	Iy *= uniformScaleXY;
	Iz *= uniformScaleXY;
	Iw *= uniformScaleXY;

	Jx *= uniformScaleXY;
	Jy *= uniformScaleXY;
	Jz *= uniformScaleXY;
	Jw *= uniformScaleXY;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::ScaleNonUniform2D( const Vec2& scalesXY )
{
	Ix *= scalesXY.x;
	Iy *= scalesXY.x;
	Iz *= scalesXY.x;
	Iw *= scalesXY.x;

	Jx *= scalesXY.y;
	Jy *= scalesXY.y;
	Jz *= scalesXY.y;
	Jw *= scalesXY.y;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::Translate3D( const Vec3& translation3D )
{
	Mat44 tempMat44 = *this;

	Tx += ( tempMat44.Ix * translation3D.x ) + ( tempMat44.Jx * translation3D.y ) + ( tempMat44.Kx * translation3D.z );
	Ty += ( tempMat44.Iy * translation3D.x ) + ( tempMat44.Jy * translation3D.y ) + ( tempMat44.Ky * translation3D.z );
	Tz += ( tempMat44.Iz * translation3D.x ) + ( tempMat44.Jz * translation3D.y ) + ( tempMat44.Kz * translation3D.z );
	Tw += ( tempMat44.Iw * translation3D.x ) + ( tempMat44.Jw * translation3D.y ) + ( tempMat44.Kw * translation3D.z );
}


//---------------------------------------------------------------------------------------------------------
void Mat44::ScaleUniform3D( float uniformScaleXYZ )
{
	Ix *= uniformScaleXYZ;
	Iy *= uniformScaleXYZ;
	Iz *= uniformScaleXYZ;
	Iw *= uniformScaleXYZ;

	Jx *= uniformScaleXYZ;
	Jy *= uniformScaleXYZ;
	Jz *= uniformScaleXYZ;
	Jw *= uniformScaleXYZ;
	
	Kx *= uniformScaleXYZ;
	Ky *= uniformScaleXYZ;
	Kz *= uniformScaleXYZ;
	Kw *= uniformScaleXYZ;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::ScaleNonUniform3D( const Vec3& scalesXYZ )
{
	Ix *= scalesXYZ.x;
	Iy *= scalesXYZ.x;
	Iz *= scalesXYZ.x;
	Iw *= scalesXYZ.x;

	Jx *= scalesXYZ.y;
	Jy *= scalesXYZ.y;
	Jz *= scalesXYZ.y;
	Jw *= scalesXYZ.y;
	
	Kx *= scalesXYZ.z;
	Ky *= scalesXYZ.z;
	Kz *= scalesXYZ.z;
	Kw *= scalesXYZ.z;
}


//---------------------------------------------------------------------------------------------------------
void Mat44::TransformBy( const Mat44& transformationMatrix )
{
	Mat44 tempMat44 = *this;

	Ix = ( tempMat44.Ix * transformationMatrix.Ix ) + ( tempMat44.Jx * transformationMatrix.Iy ) + ( tempMat44.Kx * transformationMatrix.Iz ) + ( tempMat44.Tx * transformationMatrix.Iw );
	Jx = ( tempMat44.Ix * transformationMatrix.Jx ) + ( tempMat44.Jx * transformationMatrix.Jy ) + ( tempMat44.Kx * transformationMatrix.Jz ) + ( tempMat44.Tx * transformationMatrix.Jw );
	Kx = ( tempMat44.Ix * transformationMatrix.Kx ) + ( tempMat44.Jx * transformationMatrix.Ky ) + ( tempMat44.Kx * transformationMatrix.Kz ) + ( tempMat44.Tx * transformationMatrix.Kw );
	Tx = ( tempMat44.Ix * transformationMatrix.Tx ) + ( tempMat44.Jx * transformationMatrix.Ty ) + ( tempMat44.Kx * transformationMatrix.Tz ) + ( tempMat44.Tx * transformationMatrix.Tw );

	Iy = ( tempMat44.Iy * transformationMatrix.Ix ) + ( tempMat44.Jy * transformationMatrix.Iy ) + ( tempMat44.Ky * transformationMatrix.Iz ) + ( tempMat44.Ty * transformationMatrix.Iw );
	Jy = ( tempMat44.Iy * transformationMatrix.Jx ) + ( tempMat44.Jy * transformationMatrix.Jy ) + ( tempMat44.Ky * transformationMatrix.Jz ) + ( tempMat44.Ty * transformationMatrix.Jw );
	Ky = ( tempMat44.Iy * transformationMatrix.Kx ) + ( tempMat44.Jy * transformationMatrix.Ky ) + ( tempMat44.Ky * transformationMatrix.Kz ) + ( tempMat44.Ty * transformationMatrix.Kw );
	Ty = ( tempMat44.Iy * transformationMatrix.Tx ) + ( tempMat44.Jy * transformationMatrix.Ty ) + ( tempMat44.Ky * transformationMatrix.Tz ) + ( tempMat44.Ty * transformationMatrix.Tw );

	Iz = ( tempMat44.Iz * transformationMatrix.Ix ) + ( tempMat44.Jz * transformationMatrix.Iy ) + ( tempMat44.Kz * transformationMatrix.Iz ) + ( tempMat44.Tz * transformationMatrix.Iw );
	Jz = ( tempMat44.Iz * transformationMatrix.Jx ) + ( tempMat44.Jz * transformationMatrix.Jy ) + ( tempMat44.Kz * transformationMatrix.Jz ) + ( tempMat44.Tz * transformationMatrix.Jw );
	Kz = ( tempMat44.Iz * transformationMatrix.Kx ) + ( tempMat44.Jz * transformationMatrix.Ky ) + ( tempMat44.Kz * transformationMatrix.Kz ) + ( tempMat44.Tz * transformationMatrix.Kw );
	Tz = ( tempMat44.Iz * transformationMatrix.Tx ) + ( tempMat44.Jz * transformationMatrix.Ty ) + ( tempMat44.Kz * transformationMatrix.Tz ) + ( tempMat44.Tz * transformationMatrix.Tw );

	Iw = ( tempMat44.Iw * transformationMatrix.Ix ) + ( tempMat44.Jw * transformationMatrix.Iy ) + ( tempMat44.Kw * transformationMatrix.Iz ) + ( tempMat44.Tw * transformationMatrix.Iw );
	Jw = ( tempMat44.Iw * transformationMatrix.Jx ) + ( tempMat44.Jw * transformationMatrix.Jy ) + ( tempMat44.Kw * transformationMatrix.Jz ) + ( tempMat44.Tw * transformationMatrix.Jw );
	Kw = ( tempMat44.Iw * transformationMatrix.Kx ) + ( tempMat44.Jw * transformationMatrix.Ky ) + ( tempMat44.Kw * transformationMatrix.Kz ) + ( tempMat44.Tw * transformationMatrix.Kw );
	Tw = ( tempMat44.Iw * transformationMatrix.Tx ) + ( tempMat44.Jw * transformationMatrix.Ty ) + ( tempMat44.Kw * transformationMatrix.Tz ) + ( tempMat44.Tw * transformationMatrix.Tw );
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateXRotationDegrees( float degreesAboutX )
{
	Mat44 newMat44;

	float cosTheta = CosDegrees( degreesAboutX );
	float sinTheta = SinDegrees( degreesAboutX );

	newMat44.Jy = cosTheta;
	newMat44.Jz = sinTheta;

	newMat44.Ky = -sinTheta;
	newMat44.Kz = cosTheta;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateYRotationDegrees( float degreesAboutY )
{
	Mat44 newMat44;

	float cosTheta = CosDegrees( degreesAboutY );
	float sinTheta = SinDegrees( degreesAboutY );

	newMat44.Ix = cosTheta;
	newMat44.Iz = -sinTheta;

	newMat44.Kx = sinTheta;
	newMat44.Kz = cosTheta;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateZRotationDegrees( float degreesAboutZ )
{
	Mat44 newMat44;

	float cosTheta = CosDegrees( degreesAboutZ );
	float sinTheta = SinDegrees( degreesAboutZ );

	newMat44.Ix = cosTheta;
	newMat44.Iy = sinTheta;

	newMat44.Jx = -sinTheta;
	newMat44.Jy = cosTheta;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateTranslationXY( const Vec2& translationXY )
{
	Mat44 newMat44;

	newMat44.Tx = translationXY.x;
	newMat44.Ty = translationXY.y;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateUniformScaleXY( float uniformScaleXY )
{
	Mat44 newMat44;

	newMat44.Ix = uniformScaleXY;
	newMat44.Jy = uniformScaleXY;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateNonUniformScaleXY( const Vec2& scalesXY )
{
	Mat44 newMat44;

	newMat44.Ix = scalesXY.x;
	newMat44.Jy = scalesXY.y;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateTranslationXYZ( const Vec3& translationXYZ )
{
	Mat44 newMat44;

	newMat44.Tx = translationXYZ.x;
	newMat44.Ty = translationXYZ.y;
	newMat44.Tz = translationXYZ.z;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateUniformScaleXYZ( float uniformScaleXYZ )
{
	Mat44 newMat44;

	newMat44.Ix = uniformScaleXYZ;
	newMat44.Jy = uniformScaleXYZ;
	newMat44.Kz = uniformScaleXYZ;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateNonUniformScaleXYZ( const Vec3& scalesXYZ )
{
	Mat44 newMat44;

	newMat44.Ix = scalesXYZ.x;
	newMat44.Jy = scalesXYZ.y;
	newMat44.Kz = scalesXYZ.z;

	return newMat44;
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateOrthographicProjection( const Vec3& min, const Vec3& max )
{
	// min.x, max.x -> (-1, 1)
	//ndc.x = x / ( max.x - min.x ) - ( min.x / ( max.x - min.x ) ) * 2.f + -1.f;
	//a = 2.f / ( max.x - min.x )
	//b = ( -2.f * min.x - max.x + min.x ) / ( max.x - min.x )
	//  = -(max.x + min.x) / ( max.x - min.x )

	// min.z, max.z -> (0, 1)
	//ndc.x = x / ( max.x - min.x ) - ( min.x / ( max.x - min.x ) ) * 1.f;
	//a = 2.f / ( max.x - min.x )
	//b = ( -2.f * min.x - max.x + min.x ) / ( max.x - min.x )
	//  = -min.x / ( max.x - min.x )


	Vec3 diff = max - min;
	Vec3 sum = max + min;

	float mat[] = {
		2.0f / diff.x,		0.0f,				0.0f,				0.0f,
		0.0f,				2.0f / diff.y,		0.0f,				0.0f,
		0.0f,				0.0f,				1.0f / diff.z,		0.0f,
		-sum.x / diff.x,	-sum.y / diff.y,	-min.z / diff.z,	1.0f
	};

	return Mat44( mat );
}


//---------------------------------------------------------------------------------------------------------
const Mat44 Mat44::CreatePerspectiveProjection( float fieldOfViewDegrees, float aspectRatio, float nearZ, float farZ )
{
	float height = 1.0f / TanDegrees( fieldOfViewDegrees * 0.5f );
	float zRange = farZ - nearZ;
	float inverseZRange = 1.0f / zRange;

	float mat[] = {
	height / aspectRatio,			0,								0,									0,
						0,		height,								0,									0,
						0,			0,			-farZ * inverseZRange,		nearZ * farZ * inverseZRange,
						0,			0,								-1,									0
	};

	return Mat44( mat );
}


//---------------------------------------------------------------------------------------------------------
// void Mat44::operator=( const Mat44& copyFrom )
// {
// 	Ix = copyFrom.Ix;
// 	Iy = copyFrom.Iy;
// 	Iz = copyFrom.Iz;
// 	Iw = copyFrom.Iw;
// 
// 	Jx = copyFrom.Jx;
// 	Jy = copyFrom.Jy;
// 	Jz = copyFrom.Jz;
// 	Jw = copyFrom.Jw;
// 	
// 	Kx = copyFrom.Kx;
// 	Ky = copyFrom.Ky;
// 	Kz = copyFrom.Kz;
// 	Kw = copyFrom.Kw;
// 
// 	Tx = copyFrom.Tx;
// 	Ty = copyFrom.Ty;
// 	Tz = copyFrom.Tz;
// 	Tw = copyFrom.Tw;
// }
