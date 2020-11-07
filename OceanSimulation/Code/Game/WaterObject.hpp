#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Rgba8.hpp"

class GPUMesh;

class WaterObject
{
public:
	~WaterObject()	{};
	WaterObject( Vec3 const& m_halfDimensions, Vec3 const& initialPosition );

	void Update();
	void Render() const;

	Vec3	GetPosition() const		{ return m_transform.GetPosition(); }
	AABB3	GetBounds() const		{ return m_bounds; }

	void CreateMesh();

private:
	float		m_movementSpeed = 1.f;
	GPUMesh*	m_mesh			= nullptr;
	Transform	m_transform		= Transform();
	AABB3		m_bounds		= AABB3();
	Rgba8		m_tint			= Rgba8::WHITE;
};