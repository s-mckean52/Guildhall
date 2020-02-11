#include "Engine/Math/Polygon2D.hpp"

Polygon2D::Polygon2D( const Polygon2D& copyFrom )
{

}

Polygon2D::Polygon2D( const std::vector<Vec2> verts, const Vec2& position )
{

}

bool Polygon2D::IsValid() const
{

}

bool Polygon2D::IsPointInside( const Vec2& point ) const
{

}

const Vec2 Polygon2D::GetCenter() const
{

}

const Vec2 Polygon2D::GetNearestPoint( const Vec2& referentPos ) const
{

}

float Polygon2D::GetOuterRadius() const
{

}

float Polygon2D::GetInnerRadius() const
{

}

std::vector<Vec2> Polygon2D::GetLocalVerts() const
{

}

void Polygon2D::AddLocalVert( const Vec2& worldPosition )
{

}

void Polygon2D::SetLocalVerts( std::vector<Vec2> worldVerts )
{

}

void Polygon2D::Translate( const Vec2& translation )
{

}

void Polygon2D::SetCenter( const Vec2& newCenter )
{

}

void Polygon2D::SetOrientationDegrees( float newOrientaiton )
{

}

void Polygon2D::RotateByDegrees( float relativeRotationDegrees )
{

}
