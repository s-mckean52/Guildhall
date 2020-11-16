#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Game/Client.hpp"

class Server;
class Entity;
class InputSystem;


class RemoteClient : public Client
{
public:
	RemoteClient( Server* owner );
	~RemoteClient();

	void BeginFrame()	override;
	void EndFrame()		override;
	void Update()		override;
	void Render()		override;

	CameraData GetCameraData() const;

	void SetInputFromInputState( InputState const& inputState );
	void SetCameraFromCameraData( CameraData const& cameraData );

protected:
	Vec3			m_position;
	float			m_yawDegrees				= 0.f;
	float			m_pitchDegrees				= 0.f;
	float			m_rollDegrees				= 0.f;
	InputSystem*	m_input				= nullptr;
	Entity*			m_possessedEntity	= nullptr;
};