#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Game/Server.hpp"

struct UDPMessage;

struct CameraData
{
	Vec3 m_position;
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};

class Client
{
public:
	Client( Server* owner );
	virtual ~Client();

	virtual void BeginFrame()	= 0;
	virtual void EndFrame()		= 0;
	virtual void Update()		= 0;
	virtual void Render()		= 0;
	virtual void SendReliableWorldData() = 0;

	bool IsDisconnecting() const { return m_isDisconnecting; }

	void SetFrameNum( int frameNum ) { m_frameNum = frameNum; }

	void UnpackUDPMessage( UDPMessage const& message );

	

protected:
	bool m_isDisconnecting = false;

	Server*		m_owner;
	int			m_frameNum = 0;
	UDPPacket	m_packets[NUM_MESSAGE_ID] = {};
};