#pragma once

struct Vec2;
struct Vec3;
struct Vec4;

struct Mat44
{
public:
	float Ix = 1.f;
	float Iy = 0.f;
	float Iz = 0.f;
	float Iw = 0.f;

	float Jx = 0.f;
	float Jy = 1.f;
	float Jz = 0.f;
	float Jw = 0.f;

	float Kx = 0.f;
	float Ky = 0.f;
	float Kz = 1.f;
	float Kw = 0.f;

	float Tx = 0.f;
	float Ty = 0.f;
	float Tz = 0.f;
	float Tw = 1.f;

	const static Mat44 IDENTITY;

public:
	//Constructors
	Mat44() = default;
	explicit Mat44( float* sixteenValuesBasisMajor );
	explicit Mat44( const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation2D );
	explicit Mat44( const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis, const Vec3& translation3D );
	explicit Mat44( const Vec4& iBasisHomogeneous, const Vec4& jBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous );

	//Transform positions and vectors using this Matrix
	const Vec2	TransformVector2D( const Vec2& vector ) const;
	const Vec3	TransformVector3D( const Vec3& vector ) const;
	const Vec2	TransformPosition2D( const Vec2& position ) const;
	const Vec3	TransformPosition3D( const Vec3& position ) const;
	const Vec4	TransformHomogeneousPoint3D( const Vec4& point ) const;
	const Mat44	GetTransformMatrixBy( const Mat44& transformationMatrix ) const;

	//Accessors
	const float*	GetAsFloatArray() const			{ return &Ix; }
	float*			GetAsFloatArray()				{ return &Ix; }

	const Vec2		GetIBasis2D() const;
	const Vec2		GetJBasis2D() const;
	const Vec2		GetTranslation2D() const;

	const Vec3		GetIBasis3D() const;
	const Vec3		GetJBasis3D() const;
	const Vec3		GetKBasis3D() const;
	const Vec3		GetTranslation3D()  const;

	const Vec4		GetIBasis4D() const;
	const Vec4		GetJBasis4D() const;
	const Vec4		GetKBasis4D() const;
	const Vec4		GetTranslation4D() const;

	//Mutators
	void SetTranslation2D( const Vec2& translation2D );
	void SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D );
	void SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D );

	void SetTranslation3D( const Vec3& translation3D );
	void SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D );
	void SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D );

	void SetBasisVectors4D( const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D );

	//Transformation Mutators
	void RotateXDegrees( float degreesAboutX );
	void RotateYDegrees( float degreesAboutY );
	void RotateZDegrees( float degreesAboutZ );

	void Translate2D( const Vec2& translation2D );
	void ScaleUniform2D( float uniformScaleXY );
	void ScaleNonUniform2D( const Vec2& scalesXY );

	void Translate3D( const Vec3& translation3D );
	void ScaleUniform3D( float uniformScaleXYZ );
	void ScaleNonUniform3D( const Vec3& scalesXYZ );

	void TransformBy( const Mat44& transformationMatrix );

	//Static Creation Methods
	static const Mat44 CreateXRotationDegrees( float degreesAboutX );
	static const Mat44 CreateYRotationDegrees( float degreesAboutY );
	static const Mat44 CreateZRotationDegrees( float degreesAboutZ );
	//static const Mat44 CreateXYZRotationDegrees( const Vec3& rotationXYZ );
					   
	static const Mat44 CreateTranslationXY( const Vec2& translationXY );
	static const Mat44 CreateUniformScaleXY( float uniformScaleXY );
	static const Mat44 CreateNonUniformScaleXY( const Vec2& scalesXY );
					   
	static const Mat44 CreateTranslationXYZ( const Vec3& translationXYZ );
	static const Mat44 CreateUniformScaleXYZ( float uniformScaleXYZ );
	static const Mat44 CreateNonUniformScaleXYZ( const Vec3& scalesXYZ );


	//Projection
	static const Mat44 CreateOrthographicProjection( const Vec3& min, const Vec3& max );
	static const Mat44 CreatePerspectiveProjection( float fieldOfView, float aspectRatio, float nearZ, float farZ );

// public:
// 	void operator=( const Mat44& copyFrom );

private:
	const Mat44 operator*( const Mat44& rhs ) const = delete;
};