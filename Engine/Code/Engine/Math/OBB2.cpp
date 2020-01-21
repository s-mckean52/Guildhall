#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
OBB2::OBB2( const OBB2& copyFrom )
{
	m_center			= copyFrom.m_center;
	m_halfDimensions	= copyFrom.m_halfDimensions;
	m_iBasis			= copyFrom.m_iBasis;
}


//---------------------------------------------------------------------------------------------------------
OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, const Vec2& iBasisNormal )
{
	m_center			= center;
	m_halfDimensions	= fullDimensions * 0.5f;
	m_iBasis			= iBasisNormal;
}


//---------------------------------------------------------------------------------------------------------
OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, float orientationDegrees )
{
	m_center	= center;
	m_halfDimensions = fullDimensions * 0.5f;
	m_iBasis = Vec2::MakeFromPolarDegrees( orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
OBB2::OBB2( const AABB2& asAxisAlignedBox, float orientationDegrees )
{
	m_center = asAxisAlignedBox.GetCenter();
	m_halfDimensions = asAxisAlignedBox.GetDimensions() * 0.5f;
	m_iBasis = Vec2::MakeFromPolarDegrees( orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
bool OBB2::IsPointInside( const Vec2& point ) const
{
	Vec2 iBasisNormal = GetIBasisNormal();
	Vec2 jBasisNormal = GetJBasisNormal();

	Vec2 displacementToPoint = point - m_center;
	Vec2 iComponent = GetProjectedOnto2D( displacementToPoint, iBasisNormal );
	Vec2 jComponent = GetProjectedOnto2D( displacementToPoint, jBasisNormal );

	if( iComponent.GetLength() < m_halfDimensions.x && jComponent.GetLength() < m_halfDimensions.y )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetCenter() const
{
	return m_center;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetDimensions() const
{
	return m_halfDimensions * 2.f;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetIBasisNormal() const
{
	return m_iBasis.GetNormalized();
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetJBasisNormal() const
{
	Vec2 iBasisNormal = m_iBasis.GetNormalized();
	return iBasisNormal.GetRotated90Degrees();
}


//---------------------------------------------------------------------------------------------------------
float OBB2::GetOrientationDegrees() const
{
	return m_iBasis.GetAngleDegrees();
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetNearestPoint( const Vec2& referentPos ) const
{
	Vec2 iBasisNormal = GetIBasisNormal();
	Vec2 jBasisNormal = GetJBasisNormal();

	Vec2 displacementToPoint = referentPos - m_center;
	float iComponent = DotProduct2D( iBasisNormal, displacementToPoint );
	float jComponent = DotProduct2D( jBasisNormal, displacementToPoint );

	Clamp( iComponent, -m_halfDimensions.x, m_halfDimensions.x );
	Clamp( jComponent, -m_halfDimensions.y, m_halfDimensions.y );

	Vec2 nearestPointWorld = m_center + ( iComponent * iBasisNormal ) + ( jComponent * jBasisNormal );

	return nearestPointWorld;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const
{
	Vec2 mins	= m_center - m_halfDimensions;
	Vec2 maxes	= m_center + m_halfDimensions;

	float localX = RangeMapFloat( 0.f, 1.f, mins.x, maxes.x, uvCoordsZeroToOne.x );
	float localY = RangeMapFloat( 0.f, 1.f, mins.y, maxes.y, uvCoordsZeroToOne.y );

	Vec2 worldPoint = m_center + ( localX * GetIBasisNormal() ) + ( localY * GetJBasisNormal() );

	return worldPoint;
}


//---------------------------------------------------------------------------------------------------------
const Vec2 OBB2::GetUVForPoint( const Vec2& point ) const
{
	Vec2 mins	= m_center - m_halfDimensions;
	Vec2 maxes	= m_center + m_halfDimensions;

	Vec2 iBasisNormal = GetIBasisNormal();
	Vec2 jBasisNormal = GetJBasisNormal();

	Vec2 displacementToPoint = point - m_center;
	float iComponent = DotProduct2D( iBasisNormal, displacementToPoint );
	float jComponent = DotProduct2D( jBasisNormal, displacementToPoint );

	float convertedU = RangeMapFloat( mins.x, maxes.x, 0.f, 1.f, iComponent );
	float convertedV = RangeMapFloat( mins.y, maxes.y, 0.f, 1.f, jComponent );

	return Vec2( convertedU, convertedV );
}


//---------------------------------------------------------------------------------------------------------
float OBB2::GetOuterRadius() const
{
	return m_halfDimensions.x + m_halfDimensions.y;
}


//---------------------------------------------------------------------------------------------------------
float OBB2::GetInnerRadius() const
{
	if( m_halfDimensions.x > m_halfDimensions.y )
	{
		return m_halfDimensions.y;
	}
	return m_halfDimensions.x;
}


//---------------------------------------------------------------------------------------------------------
void OBB2::GetCornerPositions( Vec2* out_fourPoints ) const
{
	Vec2 distanceFromCenterToRightSide	= GetIBasisNormal() * m_halfDimensions.x;
	Vec2 distanceFromCenterToTopSide	= GetJBasisNormal() * m_halfDimensions.y;

	out_fourPoints[ 0 ] = m_center - distanceFromCenterToRightSide - distanceFromCenterToTopSide;
	out_fourPoints[ 1 ] = m_center + distanceFromCenterToRightSide - distanceFromCenterToTopSide;
	out_fourPoints[ 2 ] = m_center + distanceFromCenterToRightSide + distanceFromCenterToTopSide;
	out_fourPoints[ 3 ] = m_center - distanceFromCenterToRightSide + distanceFromCenterToTopSide;
}


//---------------------------------------------------------------------------------------------------------
void OBB2::Translate( const Vec2& translation )
{
	m_center += translation;
}


//---------------------------------------------------------------------------------------------------------
void OBB2::SetCenter( const Vec2& newCenter )
{
	m_center = newCenter;
}


//---------------------------------------------------------------------------------------------------------
void OBB2::SetDimensions( const Vec2& newFullDimensions )
{
	m_halfDimensions = newFullDimensions * 0.5f;
}


//---------------------------------------------------------------------------------------------------------
void OBB2::SetOrientationDegrees( float newOrientaiton )
{
	m_iBasis.SetAngleDegrees( newOrientaiton );
}


//---------------------------------------------------------------------------------------------------------
void OBB2::RotateByDegrees( float relativeRotationDegrees )
{
	m_iBasis.RotateDegrees( relativeRotationDegrees );
}


//---------------------------------------------------------------------------------------------------------
void OBB2::StretchToIncludePoint( const Vec2& point )
{
	Vec2 displacementToPoint = point - m_center;
	float iComponent = DotProduct2D( GetIBasisNormal(), point );
	float jComponent = DotProduct2D( GetJBasisNormal(), point );

	if( iComponent > m_halfDimensions.x )
	{
		float distanceToStretch = iComponent - m_halfDimensions.x;
		m_halfDimensions.x += distanceToStretch * 0.5f;
		m_center.x += distanceToStretch * 0.5f;
	}
	else if( iComponent < -m_halfDimensions.x )
	{
		float distanceToStretch = -m_halfDimensions.x - iComponent;
		m_halfDimensions.x += distanceToStretch * 0.5f;
		m_center.x -= distanceToStretch * 0.5f;
	}

	if( jComponent > m_halfDimensions.y )
	{
		float distanceToStretch = jComponent - m_halfDimensions.y;
		m_halfDimensions.y += distanceToStretch * 0.5f;
		m_center.y += distanceToStretch * 0.5f;
	}
	else if( jComponent < -m_halfDimensions.y )
	{
		float distanceToStretch = -m_halfDimensions.y - jComponent;
		m_halfDimensions.y += distanceToStretch * 0.5f;
		m_center.y -= distanceToStretch * 0.5f;
	}
}


//---------------------------------------------------------------------------------------------------------
void OBB2::Fix()
{
	m_halfDimensions.x = abs( m_halfDimensions.x );
	m_halfDimensions.y = abs( m_halfDimensions.y );

	if( m_iBasis == Vec2( 0.f, 0.f ) )
	{
		m_iBasis = Vec2( 1.f, 0.f );
	}
	m_iBasis.Normalize();
}


//---------------------------------------------------------------------------------------------------------
void OBB2::operator=( const OBB2& assignForm )
{
	m_center			= assignForm.m_center;
	m_halfDimensions	= assignForm.m_halfDimensions;
	m_iBasis			= assignForm.m_iBasis;
}
