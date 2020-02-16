#pragma once

struct Mat44;

Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ );