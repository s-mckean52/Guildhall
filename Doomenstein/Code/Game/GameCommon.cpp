#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <string>


//---------------------------------------------------------------------------------------------------------
Mat44 GetBillboardTransformMatrix( Camera const& cameraToLookAt, Vec3 const& positionToLookFrom, BillboardType billboardType )
{
	switch( billboardType )
	{
	case BillboardType::CAMERA_FACING_XY:
		return BillboardFacingXY( cameraToLookAt, positionToLookFrom );
		break;
	case BillboardType::CAMERA_OPPOSING_XY:
		return BillboardOpposingXY( cameraToLookAt, positionToLookFrom );
		break;
	case BillboardType::CAMERA_FACING_XYZ:
		return BillboardFacingXYZ( cameraToLookAt, positionToLookFrom );
		break;
	case BillboardType::CAMERA_OPPOSING_XYZ:
		return BillboardOpposingXYZ( cameraToLookAt, positionToLookFrom );
		break;
	default:
		ERROR_AND_DIE( "Invalid Billboardtype" );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
Mat44 BillboardFacingXY( Camera const& cameraToLookAt, Vec3 const& positionToLookFrom )
{
	Vec3 cameraPosition = cameraToLookAt.GetPosition();
	Vec2 cameraPositionXY = Vec2( cameraPosition.x, cameraPosition.y );
	Vec2 lookFromPositionXY = Vec2( positionToLookFrom.x, positionToLookFrom.y );
	Vec2 lookDirection = cameraPositionXY - lookFromPositionXY;
	lookDirection.Normalize();

	//GUARANTEE_OR_DIE( ApproximatelyEqual( lookDirection.GetLength(), 1.f ), "BILLBOARD ERROR" );

	float mat[] = { lookDirection.x,	-lookDirection.y,	0.f,	positionToLookFrom.x,
					lookDirection.y,	lookDirection.x,	0.f,	positionToLookFrom.y,
					0.f,				0.f,				1.f,	positionToLookFrom.z,
					0.f,				0.f,				0.f,	1.f };

	return Mat44( mat );
}


//---------------------------------------------------------------------------------------------------------
Mat44 BillboardOpposingXY( Camera const& cameraToLookAt, Vec3 const& positionToLookFrom )
{
	Vec3 cameraPosition = cameraToLookAt.GetPosition();
	Vec2 cameraPositionXY = Vec2( cameraPosition.x, cameraPosition.y );
	Vec2 lookFromPositionXY = Vec2( positionToLookFrom.x, positionToLookFrom.y );
	Vec2 lookDirection = lookFromPositionXY - cameraPositionXY;
	lookDirection.Normalize();

	//GUARANTEE_OR_DIE( ApproximatelyEqual( lookDirection.GetLength(), 1.f ), "BILLBOARD ERROR" );

	float mat[] = { lookDirection.x,	-lookDirection.y,	0.f,	positionToLookFrom.x,
					lookDirection.y,	lookDirection.x,	0.f,	positionToLookFrom.y,
					0.f,				0.f,				1.f,	positionToLookFrom.z,
					0.f,				0.f,				0.f,	1.f };

	return Mat44( mat );
}


//---------------------------------------------------------------------------------------------------------
Mat44 BillboardFacingXYZ( Camera const& cameraToLookAt, Vec3 const& positionToLookFrom )
{	
	Vec3 worldUp = Vec3::UNIT_POSITIVE_Z;

	Vec3 forwardNormal = cameraToLookAt.GetPosition() - positionToLookFrom;
	forwardNormal.Normalize();

	Vec3 leftNormal = CrossProduct3D( worldUp, forwardNormal );
	if( ApproximatelyEqual( leftNormal.GetLength(), 0.f ) )
	{
		leftNormal = CrossProduct3D( Vec3::UNIT_POSITIVE_Z, forwardNormal );
		if( ApproximatelyEqual( leftNormal.GetLength(), 0.f ) )
		{
			leftNormal = Vec3::UNIT_POSITIVE_Y;
		}
	}
	leftNormal.Normalize();

	Vec3 upNormal = CrossProduct3D( forwardNormal, leftNormal );
	return Mat44( forwardNormal, leftNormal, upNormal, positionToLookFrom );
}


//---------------------------------------------------------------------------------------------------------
Mat44 BillboardOpposingXYZ( Camera const& cameraToLookAt, Vec3 const& positionToLookFrom )
{
	Vec3 worldUp = Vec3::UNIT_POSITIVE_Z;

	Vec3 forwardNormal = cameraToLookAt.GetPosition() - positionToLookFrom;
	forwardNormal.Normalize();

	Vec3 rightNormal = CrossProduct3D( forwardNormal, worldUp );
	if( ApproximatelyEqual( rightNormal.GetLength(), 0.f ) )
	{
		rightNormal = CrossProduct3D( forwardNormal, Vec3::UNIT_POSITIVE_Z );
		if( ApproximatelyEqual( rightNormal.GetLength(), 0.f ) )
		{
			rightNormal = -Vec3::UNIT_POSITIVE_Y;
		}
	}
	rightNormal.Normalize();

	Vec3 upNormal = CrossProduct3D( rightNormal, forwardNormal );
	return Mat44( forwardNormal, rightNormal, upNormal, positionToLookFrom );
}


//---------------------------------------------------------------------------------------------------------
BillboardType GetBillboardTypeFromString( char const* billboardTypeAsString )
{
	if( IsStringEqual( billboardTypeAsString, "CameraFacingXY" ) )			{ return BillboardType::CAMERA_FACING_XY; }
	else if( IsStringEqual( billboardTypeAsString, "CameraOpposingXY" ) )	{ return BillboardType::CAMERA_OPPOSING_XY; }
	else if( IsStringEqual( billboardTypeAsString, "CameraFacingXYZ" ) )	{ return BillboardType::CAMERA_FACING_XYZ; }
	else if( IsStringEqual( billboardTypeAsString, "CameraOpposingXYZ" ) )	{ return BillboardType::CAMERA_OPPOSING_XYZ; }
	else
		return BillboardType::INVALID_BILLBOARD_TYPE;
}


//---------------------------------------------------------------------------------------------------------
void DrawLineBetweenPoints( const Vec2& startPosition, const Vec2& endPosition, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> lineVerts;
	
	AppendVertsForLineBetweenPoints( lineVerts, startPosition, endPosition, color, thickness );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( lineVerts );
}


//---------------------------------------------------------------------------------------------------------
void DrawCircleAtPoint( const Vec2& position, float radius, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> circleVerts;

	AppendVertsForCircleAtPoint( circleVerts, position, radius, color, thickness );

	//TransformVertexArray( circleVerts, 1.f, 0.f, position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( circleVerts );
}


//---------------------------------------------------------------------------------------------------------
void DrawAABB2AtPoint( const Vec2& position, const AABB2& box, const Rgba8& color, float thickness )
{
	std::vector<Vertex_PCU> aabb2Verts;
	
	AppendVertsForAABB2OutlineAtPoint( aabb2Verts, box, color, thickness );

	TransformVertexArray( aabb2Verts, 1.f, 0.f, position );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( aabb2Verts );
}