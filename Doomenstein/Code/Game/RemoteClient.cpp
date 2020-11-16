#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/RemoteClient.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/Server.hpp"
#include "Game/AuthoritativeServer.hpp"

//---------------------------------------------------------------------------------------------------------
RemoteClient::RemoteClient( Server* owner )
	: Client( owner )
{
	owner->AddClient( this );
	m_input = new InputSystem();
}


//---------------------------------------------------------------------------------------------------------
RemoteClient::~RemoteClient()
{
}

//---------------------------------------------------------------------------------------------------------
void RemoteClient::BeginFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::EndFrame()
{
	static_cast<AuthoritativeServer*>(m_owner)->SendCameraData( this );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::Update()
{
	Vec2 relativeMovement = m_input->GetCursorRelativeMovement();
	m_pitchDegrees += -relativeMovement.y;
	m_yawDegrees += relativeMovement.x;
	Clamp( m_pitchDegrees, -89.9f, 89.9f );

	if( m_possessedEntity != nullptr )
	{
		m_possessedEntity->SetYaw( m_yawDegrees );
	}

	if( m_input->WasKeyJustPressed( KEY_CODE_F3 ) )
	{
		Mat44 rotationMatrix;
		rotationMatrix.RotateZDegrees( m_yawDegrees );
		rotationMatrix.RotateYDegrees( m_pitchDegrees );
		rotationMatrix.RotateXDegrees( m_rollDegrees );
		Vec3 forward = rotationMatrix.TransformVector3D( Vec3::UNIT_POSITIVE_X );

		g_theGame->TryTogglePossessEntity( &m_possessedEntity, m_position, forward );

		if( m_possessedEntity != nullptr )
		{
			m_yawDegrees = m_possessedEntity->GetYaw();
		}
	}

	if( m_possessedEntity != nullptr )
	{
		m_position = g_theGame->MoveEntity( m_possessedEntity, m_yawDegrees, m_input );
		g_theGame->MoveCameraToEntityEye( m_possessedEntity, m_position, m_yawDegrees );
	}
	else
	{
		m_position += g_theGame->MoveWorldCamera( g_theGame->GetDeltaSeconds(), m_yawDegrees, m_input );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::Render()
{

}


//---------------------------------------------------------------------------------------------------------
CameraData RemoteClient::GetCameraData() const
{
	CameraData cameraData;
	cameraData.m_position = m_position;
	cameraData.m_yawDegrees = m_yawDegrees;
	cameraData.m_pitchDegrees = m_pitchDegrees;
	cameraData.m_rollDegrees = m_rollDegrees;

	return cameraData;
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SetInputFromInputState( InputState const& inputState )
{
	m_input->SetFromInputState( inputState );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SetCameraFromCameraData( CameraData const& cameraData )
{
	m_position		= cameraData.m_position;
	m_yawDegrees	= cameraData.m_yawDegrees;
	m_pitchDegrees	= cameraData.m_pitchDegrees;
	m_rollDegrees	= cameraData.m_rollDegrees;
}

