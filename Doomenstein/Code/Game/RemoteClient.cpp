#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/RemoteClient.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/Server.hpp"
#include "Game/AuthoritativeServer.hpp"
#include "Game/World.hpp"


//---------------------------------------------------------------------------------------------------------
RemoteClient::RemoteClient( Server* owner, UDPSocket* socket )
	: Client( owner )
{
	m_socket = socket;
	owner->AddClient( this );
	m_input = new InputSystem();
}


//---------------------------------------------------------------------------------------------------------
RemoteClient::~RemoteClient()
{
	SendDisconnectMessage();
}

//---------------------------------------------------------------------------------------------------------
void RemoteClient::BeginFrame()
{
	if( m_isDisconnecting )
		return;

	if( m_possessedEntity != nullptr && m_possessedEntity->GetIsDead() )
	{
		m_possessedEntity = nullptr;
	}

	if( g_isDebugDraw )
	{
		DebugAddWorldWireSphere( m_position, 0.2f, Rgba8::BLUE, 0.f );
	}
	ProcessUDPMessages();
	SendWorldData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::EndFrame()
{
	if( m_isDisconnecting )
		return;

	SendCameraData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::Update()
{
	if( m_isDisconnecting )
		return;

	Vec2 relativeMovement = m_input->GetCursorRelativeMovement();
	m_pitchDegrees += -relativeMovement.y;
	m_yawDegrees += relativeMovement.x;
	Clamp( m_pitchDegrees, -89.9f, 89.9f );

	if( m_possessedEntity != nullptr )
	{
		m_possessedEntity->SetYaw( m_yawDegrees );

		if( m_input->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
		{
			m_possessedEntity->Shoot();
		}
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


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessUDPMessages()
{
	std::deque<UDPMessage> udpMessages;
	g_theNetworkSystem->GetUDPMessages( m_socket, udpMessages );
	for( int messageIndex = 0; messageIndex < udpMessages.size(); ++messageIndex )
	{
		UDPMessage newMessage = udpMessages[messageIndex];
		ProcessUDPMessage( newMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessUDPMessage( UDPMessage const& message )
{
	UDPMessageHeader header = message.m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_DISCONNECT: { ProcessDisconnect(); break; }
	case MESSAGE_ID_INPUT_DATA: { ProcessInputData( message ); break; }
	case MESSAGE_ID_ENTITY_DATA: { ProcessEntityData( message ); break; }
	case MESSAGE_ID_CONNECTION_DATA: { ProcessConnectionData( message ); break; }
	case MESSAGE_ID_CAMERA_DATA: { ProcessCameraData( message ); break; }
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessDisconnect()
{
	m_isDisconnecting = true;
	g_theConsole->PrintString( Rgba8::ORANGE, "Client on port: %i is disconnecting", m_socket->GetReceivePort() );
	g_theNetworkSystem->CloseUDPPort( m_socket->GetReceivePort() );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessEntityData( UDPMessage const& message )
{
	if( message.m_header.m_isReliable )
	{
		m_socket->RemoveReliableMessage( message );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessInputData( UDPMessage const& message )
{
	if( !m_owner->IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket inputPacket( m_packets[MESSAGE_ID_INPUT_DATA] );
	InputState newInputState;
	if( inputPacket.IsReadyToRead() )
	{
		memcpy( &newInputState, &inputPacket.m_data[0], sizeof( InputState ) );
		SetInputFromInputState( newInputState );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessConnectionData( UDPMessage const& message )
{
	SendSetupMessage();
	SendWorldData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::ProcessCameraData( UDPMessage const& message )
{
	if( !m_owner->IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket cameraPacket( m_packets[MESSAGE_ID_CAMERA_DATA] );
	CameraData cameraData;
	if( cameraPacket.IsReadyToRead() )
	{
		memcpy( &cameraData, &cameraPacket.m_data[0], sizeof( CameraData ) );
		SetCameraFromCameraData( cameraData );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SendWorldData()
{
	if( m_socket == nullptr )
		return;
	
	if( m_frameNum % 6 == 0 )
	{
		WorldData entityData = g_theGame->GetWorldData();
		m_owner->SendLargeUDPData( m_socket, m_socket->GetHostData(), m_socket->GetSendToPort(), &entityData, sizeof( entityData ), MESSAGE_ID_ENTITY_DATA, m_frameNum );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SendSetupMessage()
{
	if( m_socket == nullptr )
		return;
	
	ConnectionData setupData = g_theGame->GetConnectionData();
	m_owner->SendLargeUDPData( m_socket, m_socket->GetHostData(), m_socket->GetSendToPort(), &setupData, sizeof( setupData ), MESSAGE_ID_CONNECTION_DATA, m_frameNum );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SendCameraData()
{
	if( m_socket == nullptr )
	{
		g_theConsole->ErrorString( "UDP Socket not created yet" );
		return;
	}
	
	CameraData cameraDataToSend = GetCameraData();
	//g_theConsole->PrintString( Rgba8::ORANGE, "Sending Camera Data to port: %i...", m_socket->GetSendToPort() );
	m_owner->SendLargeUDPData( m_socket, m_socket->GetHostData(), m_socket->GetSendToPort(), &cameraDataToSend, sizeof( CameraData ), MESSAGE_ID_CAMERA_DATA, m_frameNum );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SendDisconnectMessage()
{
	if( m_socket == nullptr )
		return;

	m_owner->SendLargeUDPData( m_socket, m_socket->GetHostData(), m_socket->GetSendToPort(), nullptr, 0, MESSAGE_ID_DISCONNECT, m_frameNum );
}


//---------------------------------------------------------------------------------------------------------
void RemoteClient::SendReliableWorldData()
{
	if( m_socket == nullptr )
		return;
	
	WorldData entityData = g_theGame->GetWorldData();
	m_owner->SendLargeUDPData( m_socket, m_socket->GetHostData(), m_socket->GetSendToPort(), &entityData, sizeof( entityData ), MESSAGE_ID_ENTITY_DATA, m_frameNum, true );
}
