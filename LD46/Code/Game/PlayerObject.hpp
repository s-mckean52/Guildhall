#pragma once
#include "Game/GameObject.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Light.hpp"

struct Vec3;
class Camera;

class PlayerObject : public GameObject
{
public:
	PlayerObject( Vec3 const& position, float radius );
	~PlayerObject();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	

	float GetRadius() const		{ return m_radius; }

	void EnableLight();
	void MoveAlongCamera( Vec3 const& directionToMove );
	void TranslatePosition( Vec3 const& translation );
	void SetCamera( Camera* camera );
	void SetPosition( Vec3 const& newPosition );
	void DimLight( float fractionOfStart );
	void IncreseLightIntensity( float fractionOfStart, float deltaSeconds );
	bool IsDead() const;

private:
	void SetLight();
	void UpdatePosition( float deltaSeconds );
	void UpdateView( float deltaSeconds );
	void UpdateCameraView();


private:
	float m_maxIntensity = 1.f;
	float m_lightIntensity = 1.f;
	float m_fractionLightLostPerSecond = 0.05f;
	Light m_light = Light::POINT;
	Camera* m_camera = nullptr;
	Vec2 m_cameraAnglesDegreesAroundPlayer = Vec2( 0.f, -20.f );
	float m_cameraDistanceFromPlayer = 1.4f;
	float m_cameraSpeed = 1.f;

	float m_movementSpeed = 2.5f;
	float m_radius = 0.f;
};