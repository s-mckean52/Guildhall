#pragma once
#include "Game/GameObject.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Light.hpp"

struct Vec2;
struct Vec3;
class GPUMesh;
class Transform;

class Lamp : public GameObject
{
public:
	Lamp() {};
	Lamp( Vec3 const& position );

// 	virtual void Update( float deltaSeconds ) override;
// 	virtual void Render() const override;

	void EnableLight( unsigned int indexToUse );
	bool DiscIsInLight( Vec2 const& discCenter2D, float discRadius );

protected:
	float m_collisionDiscRadius = 0.f;
	Light m_light = Light::SPOTLIGHT;
};