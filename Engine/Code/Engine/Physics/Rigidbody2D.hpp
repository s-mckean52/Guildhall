#pragma once
#include "Engine/Math/Vec2.hpp"

class	Collider2D;
class	RenderContext;

enum SimulationMode
{
	SIMULATION_MODE_STATIC,
	SIMULATION_MODE_KINEMATIC,
	SIMULATION_MODE_DYNAMIC,
};

class Rigidbody2D
{
	friend class Physics2D;

public:
	void Destroy();
	void TakeCollider( Collider2D* collider );
	Vec2 GetFrameAcceleration();
	Vec2 GetVelocity() const;

	void SetPosition( Vec2 position );
	void SetEnabled( bool enable );
	void SetSimulationMode( SimulationMode simulationMode );
	void SetVelocity( Vec2 const& newVelocity );
	void SetMass( float mass );

	void AddForceFromAcceleration( const Vec2& acceleration );

	void DebugRender( RenderContext* context ) const;

	bool DoesTakeForces() const;
	bool IsEnabled() const { return m_enabled; }
	bool IsSimulated() const;

public:
	bool			m_enabled			= true;
	SimulationMode	m_simulationMode	= SIMULATION_MODE_DYNAMIC;
	Vec2			m_worldPosition;
	Vec2			m_velocity;
	Vec2			m_positionLastFrame;
	float			m_mass;
	Vec2			m_frameForces;

	Physics2D* m_physicsSystem	= nullptr;
	Collider2D* m_collider		= nullptr;

private:
	~Rigidbody2D();
};