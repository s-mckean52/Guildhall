#include "Game/GameCommon.hpp"
#include "Game/EnvironmentObject.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------------
EnvironmentObject::EnvironmentObject( EnvironmentObjectType type, Vec3 const& position, Vec3 const& dimensions, float orientationDegrees, Rgba8 const& tint )
{

	m_type = type;
	m_transform = new Transform( position, Vec3( 0.f, orientationDegrees, 0.f ) );
	m_tint = tint;
	CreateMesh( dimensions );
	Set2DBounds( dimensions, orientationDegrees );
}


//---------------------------------------------------------------------------------------------------------
EnvironmentObject::~EnvironmentObject()
{
}


//---------------------------------------------------------------------------------------------------------
void EnvironmentObject::Update(float deltaSeconds)
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void EnvironmentObject::Render() const
{
	g_theRenderer->SetCullMode(CULL_MODE_BACK);
	g_theRenderer->SetDepthTest(COMPARE_FUNC_LEQUAL, true);

	g_theRenderer->BindSampler(nullptr);
	g_theRenderer->BindShader( g_theRenderer->GetOrCreateShader("Data/Shaders/Lit.hlsl") );
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindNormalTexture(g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Brick/brick_normal.png" ) );
	g_theRenderer->SetModelUBO(m_transform->ToMatrix(), Rgba8::WHITE, m_specFactor, m_specPower);
	g_theRenderer->DrawMesh(m_mesh);
}


//---------------------------------------------------------------------------------------------------------
Vec3 EnvironmentObject::PushDiscOut( Vec2 const& discPosition, float discRadius )
{
	Vec2 returnPosition = discPosition;
	switch (m_type)
	{
	case ENVIROMENT_OBJECT_AABB:
		PushDiscOutOfAABB2( returnPosition, discRadius, m_asAABB2 );
		break;
	case ENVIROMENT_OBJECT_OBB:
		PushDiscOutOfOBB2( returnPosition, discRadius, m_asOBB2 );
		break;
	case ENVIROMENT_OBJECT_SPHERE:
	{
		PushDiscOutOfDisc2D( returnPosition, discRadius, GetPosition2D(), m_asDiscRadius );
		break;
	}
	default:
		ERROR_AND_DIE( "Invalid environment type on push out" );
		break;
	}

	return Vec3( returnPosition.x, 0.f, returnPosition.y );
}


//---------------------------------------------------------------------------------------------------------
void EnvironmentObject::CreateMesh( Vec3 const& dimensions )
{
	std::vector< Vertex_PCUTBN > meshVerticies;
	std::vector< unsigned int > meshIndicies;
	switch( m_type )
	{
	case ENVIROMENT_OBJECT_AABB:
	case ENVIROMENT_OBJECT_OBB:
	{
		Vec3 maxes = dimensions * 0.5f;
		maxes.z *= -1.f;
		Vec3 mins = -maxes;
		AddBoxToIndexedVertexArray( meshVerticies, meshIndicies, AABB3( mins, maxes ), Rgba8::WHITE );
		break;
	}
	case ENVIROMENT_OBJECT_SPHERE:
	{
		m_asDiscRadius = dimensions.x * 0.5f;
		AddUVSphereToIndexedVertexArray( meshVerticies, meshIndicies, Vec3::ZERO, m_asDiscRadius, 32, 64, Rgba8::WHITE );
		break;
	}
	default:
		ERROR_AND_DIE("Invalid Environment Object Type in Create Mesh");
		break;
	}

	m_mesh = new GPUMesh(g_theRenderer, meshVerticies, meshIndicies);
}


//---------------------------------------------------------------------------------------------------------
void EnvironmentObject::Set2DBounds( Vec3 const& dimensions, float orientationDegrees )
{
	switch (m_type)
	{
	case ENVIROMENT_OBJECT_AABB:
	{
		Vec3 halfDimensions = dimensions * 0.5f;
		Vec2 mins = GetPosition2D() - Vec2( halfDimensions.x, halfDimensions.z );
		Vec2 maxes = GetPosition2D() + Vec2( halfDimensions.x, halfDimensions.z );
		m_asAABB2 = AABB2( mins, maxes );
		break;
	}
	case ENVIROMENT_OBJECT_OBB:
	{
		m_asOBB2 = OBB2( GetPosition2D(), Vec2( dimensions.x, dimensions.z ), -orientationDegrees );
		break;
	}
	case ENVIROMENT_OBJECT_SPHERE:
	{
		m_asDiscRadius = dimensions.x * 0.5f;
		break;
	}
	default:
		ERROR_AND_DIE("Invalid Environment Object Type in Create Mesh");
		break;
	}
}
