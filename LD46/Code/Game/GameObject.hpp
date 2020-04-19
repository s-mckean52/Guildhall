#pragma once
#include "Engine/Core/Rgba8.hpp"

struct Vec2;
struct Vec3;
class GPUMesh;
class Transform;

class GameObject
{
public:
	GameObject();
	GameObject( GPUMesh* mesh, Transform* transform, Rgba8 const& tint = Rgba8::WHITE );
	~GameObject();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	Vec2 GetPosition2D() const;
	Vec3 GetPosition3D() const;

protected:
	GPUMesh* m_mesh = nullptr;
	Transform* m_transform = nullptr;
	Rgba8 m_tint = Rgba8::WHITE;

	float m_specFactor = 0.f;
	float m_specPower = 32.f;
};