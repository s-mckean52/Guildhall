#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"

Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ )
{
	Vec3 min = Vec3( minX, minY, minZ );
	Vec3 max = Vec3( maxX, maxY, maxZ );

	return Mat44::CreateOrthographicProjection( min, max );
}

