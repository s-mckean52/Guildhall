#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WaterObject.hpp"
#include "Game/Game.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
WaterObject::WaterObject( Vec3 const& m_halfDimensions, Vec3 const& initialPosition )
{
	m_movementSpeed = BOX_MOVEMENT_SPEED;
	m_bounds = AABB3( -m_halfDimensions, m_halfDimensions );
	m_transform.SetPosition( initialPosition );
	//CreateMesh();

	std::vector<Vertex_PCUTBN> verts;
	std::vector<uint> vertOffsets;
	ReadAndParseObjFile( "Data/Models/woodenCrate.obj", verts );
	m_mesh = new GPUMesh( g_theRenderer );
	m_mesh->UpdateVerticies( static_cast<uint>( verts.size() ), &verts[0] );
	m_transform.SetRotationFromPitchYawRollDegrees( -90.f, 0.f, 0.f );
	m_transform.SetUniformScale( 1.f );
	m_transform.Translate( Vec3::ZERO );
}


//---------------------------------------------------------------------------------------------------------
void WaterObject::Update( Camera* camera )
{
	float deltaSeconds = static_cast<float>( g_theGame->GetGameClock()->GetLastDeltaSeconds() );


	float leftAmount	= 0.f;
	float forwardAmount = 0.f;

	if( g_theInput->IsKeyPressed( BOX_RIGHT_KEY ) )
	{
		leftAmount -= m_movementSpeed * deltaSeconds;
	}
	if( g_theInput->IsKeyPressed( BOX_LEFT_KEY ) )
	{
		leftAmount += m_movementSpeed * deltaSeconds;
	}

	if( g_theInput->IsKeyPressed( BOX_FORWARD_KEY ) )
	{
		forwardAmount += m_movementSpeed * deltaSeconds;
	}
	if( g_theInput->IsKeyPressed( BOX_BACKWARD_KEY ) )
	{
		forwardAmount -= m_movementSpeed * deltaSeconds;
	}

	float cameraYawDegrees = camera->GetYawDegrees();
	float cosCameraYawDegrees = CosDegrees( cameraYawDegrees );
	float sinCameraYawDegrees = SinDegrees( cameraYawDegrees );
	Vec2 cameraForwardXY = Vec2( cosCameraYawDegrees, sinCameraYawDegrees );
	Vec2 cameraLeftXY = Vec2( -sinCameraYawDegrees, cosCameraYawDegrees );

	Vec2 translationXY = ( cameraForwardXY * forwardAmount ) + ( cameraLeftXY * leftAmount );
	m_transform.Translate( Vec3( translationXY, 0.f ) );
}


//---------------------------------------------------------------------------------------------------------
void WaterObject::Render() const
{
// 	std::vector<Material*> materials;
// 	materials.push_back( g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Shark_body.material" ) );
// 	materials.push_back( g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Shark_teeth.material" ) );
// 	materials.push_back( g_theRenderer->GetOrCreateMaterialFromFile( "Data/Shaders/Shark_eye.material" ) );
	Mat44 modelMatrix = m_worldOrientation;
	modelMatrix.SetTranslation3D( Vec3::ZERO );
	Mat44 transformOrientation = m_transform.ToMatrix();
	transformOrientation.SetTranslation3D( Vec3::ZERO );

	modelMatrix.TransformBy( transformOrientation );
	modelMatrix.SetTranslation3D( m_worldOrientation.GetTranslation3D() + GetPosition() );
 	
	g_theRenderer->SetModelUBO( modelMatrix );
// 	g_theRenderer->DrawMeshWithMaterials( m_mesh, &materials[0] );
	g_theRenderer->BindMaterialByPath( "Data/Shaders/WoodenCrate.material" );
	g_theRenderer->DrawMesh( m_mesh );

	if( g_isDebugDraw )
	{
		std::vector<Vertex_PCU> verts;
		std::vector<uint> indices;
		AddBoxToIndexedVertexArray( verts, indices, m_bounds, Rgba8::GREEN );

		g_theRenderer->SetModelUBO( modelMatrix );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawIndexedVertexArray( verts, indices );


		Mat44 debugBasis = m_worldOrientation;
		debugBasis.Tx = m_transform.GetPosition().x;
		debugBasis.Ty = m_transform.GetPosition().y;
		DebugAddWorldBasis( debugBasis, 0.f, DEBUG_RENDER_ALWAYS );
	}
}


//---------------------------------------------------------------------------------------------------------
void WaterObject::SetWorldTransform( Mat44 const& transform )
{
	m_worldOrientation = transform;
}


//---------------------------------------------------------------------------------------------------------
void WaterObject::CreateMesh()
{
	std::vector<Vertex_PCUTBN>	vertexArray;
	std::vector<uint>			indexArray;

	Vec3 frontBottomLeft =	m_bounds.mins;
	Vec3 frontBottomRight =	Vec3( m_bounds.maxes.x,	m_bounds.mins.y,		m_bounds.mins.z );
	Vec3 frontTopLeft =		Vec3( m_bounds.mins.x,	m_bounds.mins.y,		m_bounds.maxes.z );
	Vec3 frontTopRight =	Vec3( m_bounds.maxes.x,	m_bounds.mins.y,		m_bounds.maxes.z );

	//looking At Back
	Vec3 backBottomLeft =	Vec3( m_bounds.maxes.x,	m_bounds.maxes.y,	m_bounds.mins.z );
	Vec3 backBottomRight =	Vec3( m_bounds.mins.x,	m_bounds.maxes.y,	m_bounds.mins.z );
	Vec3 backTopLeft =		Vec3( m_bounds.maxes.x,	m_bounds.maxes.y,	m_bounds.maxes.z );
	Vec3 backTopRight =		Vec3( m_bounds.mins.x,	m_bounds.maxes.y,	m_bounds.maxes.z );

	Vec3 frontTangent = ( frontBottomRight - frontBottomLeft ).GetNormalize();
	Vec3 frontBitangent = ( frontTopLeft - frontBottomLeft ).GetNormalize();
	Vec3 frontNormal = CrossProduct3D( frontTangent, frontBitangent );

	std::vector<Vertex_PCUTBN> boxVerticies = {
		//Front
		Vertex_PCUTBN( frontBottomLeft,		m_tint,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomRight,	m_tint,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontTopLeft,		m_tint,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontTopRight,		m_tint,	frontTangent,	frontBitangent,		frontNormal,		Vec2( 1.f, 1.f ) ),

		//Right
		Vertex_PCUTBN( frontBottomRight,	m_tint,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomLeft,		m_tint,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontTopRight,		m_tint,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopLeft,			m_tint,	-frontNormal,	frontBitangent,		frontTangent,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Back																									 		    
		Vertex_PCUTBN( backBottomLeft,		m_tint,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomRight,		m_tint,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopLeft,			m_tint,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopRight,		m_tint,	-frontTangent,	frontBitangent,		-frontNormal,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Left																									 		    
		Vertex_PCUTBN( backBottomRight,		m_tint,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomLeft,		m_tint,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopRight,		m_tint,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontTopLeft,		m_tint,	frontNormal,	frontBitangent,		-frontTangent,		Vec2( 1.f, 1.f ) ),
																												 		    
		//Top																									 		    
		Vertex_PCUTBN( frontTopLeft,		m_tint,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( frontTopRight,		m_tint,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( backTopRight,		m_tint,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( backTopLeft, 		m_tint,	frontTangent,	-frontNormal,		frontBitangent,		Vec2( 1.f, 1.f ) ),

		//Bottom
		Vertex_PCUTBN( backBottomRight,		m_tint,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 0.f ) ),
		Vertex_PCUTBN( backBottomLeft,		m_tint,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 0.f ) ),
		Vertex_PCUTBN( frontBottomLeft,		m_tint,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 0.f, 1.f ) ),
		Vertex_PCUTBN( frontBottomRight,	m_tint,	frontTangent,	frontNormal,		-frontBitangent,	Vec2( 1.f, 1.f ) ),
	};

	std::vector<unsigned int> boxIndicies = {
		//Front
		0, 1, 3,
		0, 3, 2,

		//Right
		4, 5, 7,
		4, 7, 6,

		//Back
		8, 9, 11,
		8, 11, 10,

		//Right
		12, 13, 15,
		12, 15, 14,

		//Top
		16, 17, 19,
		16, 19, 18,

		//Bottom
		20, 21, 23,
		20, 23, 22,
	};

	unsigned int indexOffset = static_cast<unsigned int>( vertexArray.size() );
	for( int vertexIndex = 0; vertexIndex < boxVerticies.size(); ++vertexIndex )
	{
		vertexArray.push_back( boxVerticies[ vertexIndex ] );
	}

	for( int indexIndex = 0; indexIndex < boxIndicies.size(); ++indexIndex )
	{
		indexArray.push_back( indexOffset + boxIndicies[ indexIndex ] );
	}

	m_mesh = new GPUMesh( g_theRenderer, vertexArray, indexArray );
}

