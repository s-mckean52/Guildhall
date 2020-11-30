#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Game/Client.hpp"

class Server;
class Entity;
class InputSystem;
struct UDPMessage;


class RemoteClient : public Client
{
public:
	RemoteClient( Server* owner, UDPSocket* socket );
	~RemoteClient();

	void BeginFrame()	override;
	void EndFrame()		override;
	void Update()		override;
	void Render()		override;

	CameraData	GetCameraData() const;

	void SetInputFromInputState( InputState const& inputState );
	void SetCameraFromCameraData( CameraData const& cameraData );

	void ProcessUDPMessages();
	void ProcessUDPMessage( UDPMessage const& message );

	void ProcessDisconnect();
	void ProcessEntityData( UDPMessage const& message );
	void ProcessInputData( UDPMessage const& message );
	void ProcessConnectionData( UDPMessage const& message );
	void ProcessCameraData( UDPMessage const& message );

	void SendWorldData();
	void SendSetupMessage();
	void SendCameraData();
	void SendDisconnectMessage();
	void SendReliableWorldData() override;

protected:
	Vec3			m_position;
	UDPSocket*		m_socket			= nullptr;
	float			m_yawDegrees		= 0.f;
	float			m_pitchDegrees		= 0.f;
	float			m_rollDegrees		= 0.f;
	InputSystem*	m_input				= nullptr;
	Entity*			m_possessedEntity	= nullptr;
};