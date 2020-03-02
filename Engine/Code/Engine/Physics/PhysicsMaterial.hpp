#pragma once


class PhysicsMaterial
{
public:
	PhysicsMaterial() = default;
	~PhysicsMaterial() {}

	float GetBounciness() const		{ return m_bounciness; }
	float GetFriction() const		{ return m_friction; }

	void AddBounciness( float bouncinessToAdd );
	void AddFriction( float frictionToAdd );

public:
	float m_bounciness = 0.5f;
	float m_friction = 0.0f;
};