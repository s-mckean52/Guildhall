#pragma once
#include "Engine/Math/Vec3.hpp"

class Server;

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

protected:
	Server* m_owner;
};