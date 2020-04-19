#pragma once
#include "Game/GameObject.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"

enum EnvironmentObjectType
{
	ENVIROMENT_OBJECT_AABB,
	ENVIROMENT_OBJECT_OBB,
	ENVIROMENT_OBJECT_SPHERE,
};

class EnvironmentObject : public GameObject
{
public:
	EnvironmentObject( EnvironmentObjectType type, Vec3 const& position = Vec3::ZERO, Vec3 const& dimensions = Vec3::UNIT, float orientationDegrees = 0.f, Rgba8 const& tint = Rgba8::WHITE );
	~EnvironmentObject();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	Vec3 PushDiscOut( Vec2 const& discPosition, float discRadius );

private:
	void CreateMesh( Vec3 const& dimensions );
	void Set2DBounds( Vec3 const& dimensions, float orientationDegrees );

private:
	EnvironmentObjectType m_type;
	OBB2 m_asOBB2;
	AABB2 m_asAABB2;
	float m_asDiscRadius = 0.f;
};