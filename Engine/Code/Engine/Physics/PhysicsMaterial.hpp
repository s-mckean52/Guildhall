#pragma once


class PhysicsMaterial
{
public:
	PhysicsMaterial() = default;
	~PhysicsMaterial() {}

	float GetBounciness() const { return m_bounciness; }

public:
	float m_bounciness = 0.5f;
};