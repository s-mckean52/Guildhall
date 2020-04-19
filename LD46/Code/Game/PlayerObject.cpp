#include "Game/GameCommon.hpp"
#include "Game/PlayerObject.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
PlayerObject::PlayerObject( Vec3 const& position, float radius )
{
	std::vector<Vertex_PCUTBN> meshVerts;
	std::vector<unsigned int> meshIndicies;
	AddUVSphereToIndexedVertexArray( meshVerts, meshIndicies, Vec3::ZERO, 0.02f, 32, 64, Rgba8::WHITE );
	m_mesh = new GPUMesh( g_theRenderer, meshVerts, meshIndicies );
	m_transform = new Transform( position );
	m_tint = Rgba8::YELLOW;
	m_cameraDistanceFromPlayer += 0.02;
	m_radius = radius;
	SetLight();
}


//---------------------------------------------------------------------------------------------------------
PlayerObject::~PlayerObject()
{
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::Update( float deltaSeconds )
{
	if( IsDead() ) return;

	DimLight( m_fractionLightLostPerSecond * deltaSeconds );
	UpdatePosition( deltaSeconds );
	UpdateView( deltaSeconds );

	UpdateCameraView();
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::Render() const
{
	GameObject::Render();
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::EnableLight()
{
	g_theRenderer->EnableLight( 0, m_light );
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::MoveAlongCamera( Vec3 const& directionToMove )
{
	if( g_isDebugCamera )
	{
		TranslatePosition( directionToMove );
		return;
	}

	Mat44 cameraView = m_camera->GetTransform().ToMatrix();
	Vec3 translation = cameraView.TransformVector3D(directionToMove);
	translation.y = 0.f;
	TranslatePosition(translation);
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::TranslatePosition( Vec3 const& translation )
{
	m_transform->Translate( translation );
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::SetCamera( Camera* camera )
{
	m_camera = camera;
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::SetPosition( Vec3 const& newPosition )
{
	m_transform->SetPosition( Vec3( newPosition.x, GetPosition3D().y, newPosition.z ) );	
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::DimLight(float fractionOfStart)
{
	m_lightIntensity -= m_maxIntensity * fractionOfStart;
	Clamp(m_lightIntensity, 0.f, m_maxIntensity);
	SetLight();
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::IncreseLightIntensity(float fractionOfStart, float deltaSeconds )
{
	m_lightIntensity += m_maxIntensity * ( ( fractionOfStart + m_fractionLightLostPerSecond ) * deltaSeconds );
	Clamp( m_lightIntensity, 0.f, m_maxIntensity );
	SetLight();
}


//---------------------------------------------------------------------------------------------------------
bool PlayerObject::IsDead() const
{
	return( m_lightIntensity <= 0.f );
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::SetLight()
{
	float lightIntensityFraction = m_lightIntensity / m_maxIntensity;
	m_tint.a = static_cast<unsigned char>(255.f * lightIntensityFraction);
	m_light.position = GetPosition3D();
	m_light.intensity = m_lightIntensity;
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::UpdatePosition(float deltaSeconds)
{
	float amountToMove = m_movementSpeed * deltaSeconds;
	if (g_theInput->IsKeyPressed('W'))
	{
		MoveAlongCamera(Vec3(0.0f, 0.0f, -amountToMove));
	}
	if (g_theInput->IsKeyPressed('A'))
	{
		MoveAlongCamera(Vec3(-amountToMove, 0.0f, 0.0f));
	}
	if (g_theInput->IsKeyPressed('S'))
	{
		MoveAlongCamera(Vec3(0.0f, 0.0f, amountToMove));
	}
	if (g_theInput->IsKeyPressed('D'))
	{
		MoveAlongCamera(Vec3(amountToMove, 0.0f, 0.0f));
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::UpdateView( float deltaSeconds )
{
	if (g_isDebugCamera) return;

	float amountToMove = m_cameraSpeed * deltaSeconds;
	Vec2 relativeMovement = g_theInput->GetCursorRelativeMovement();
	m_cameraAnglesDegreesAroundPlayer.x += relativeMovement.x * amountToMove;
	m_cameraAnglesDegreesAroundPlayer.y += relativeMovement.y * amountToMove;
	Clamp( m_cameraAnglesDegreesAroundPlayer.y, -30.f, 5.f );

	if( m_cameraAnglesDegreesAroundPlayer.x >= 180.f )
	{
		m_cameraAnglesDegreesAroundPlayer.x -= 360.f;
	}
	else if( m_cameraAnglesDegreesAroundPlayer.x <= -180.f )
	{
		m_cameraAnglesDegreesAroundPlayer.x += 360.f;
	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerObject::UpdateCameraView()
{
	if (g_isDebugCamera)
	{
		Vec2 position2D = GetPosition2D();
		m_camera->SetPosition( Vec3( position2D.x, 15.f, position2D.y ) );
		m_camera->SetPitchYawRollRotationDegrees(-90.f, 0.f, 0.f);
		return;
	}

	m_camera->SetPitchYawRollRotationDegrees( m_cameraAnglesDegreesAroundPlayer.y, m_cameraAnglesDegreesAroundPlayer.x, 0.f );
	Mat44 rotationMatrix = Mat44::IDENTITY;
	RotateMatrixPitchYawRollDegrees( rotationMatrix, Vec3( m_cameraAnglesDegreesAroundPlayer.y, m_cameraAnglesDegreesAroundPlayer.x, 0.f ) );
 	Vec3 newCameraPosition = GetPosition3D() + rotationMatrix.TransformVector3D( -Vec3::FORWARD * m_cameraDistanceFromPlayer );
	m_camera->SetPosition( newCameraPosition );
}
