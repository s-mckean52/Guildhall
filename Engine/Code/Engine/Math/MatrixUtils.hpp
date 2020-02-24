#pragma once

struct Vec3;
struct Mat44;

Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ );
Mat44 MakePerspectiveProjectionMatrixD3D( float fieldOfViewDegrees, float aspectRatio, float nearZ, float farZ );

void RotateMatrixPitchYawRollDegrees( Mat44& matrixToRotate, float pitchDegrees, float rollDegrees, float yawDegrees );
void RotateMatrixPitchYawRollDegrees( Mat44& matrixToRotate, Vec3 const& pitchYawRollDegrees  );

void MatrixTranspose( Mat44& matrix );
void MatrixInvertOrthoNormal( Mat44& matrix );
void MatrixInvert( Mat44& matrix );