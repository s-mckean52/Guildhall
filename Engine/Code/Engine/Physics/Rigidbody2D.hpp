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
	void SetDrag( float drag );
	void MarkForDestroy( bool isMarkedForDestroy );

	void ApplyImpulseAt( Vec2 const& worldPos, Vec2 const& impulse );
	void AddForceFromAcceleration( const Vec2& acceleration );
	void AddDrag( float dragToAdd );

	void DebugRender( RenderContext* context ) const;

	bool	IsSimulated() const;
	bool	DoesTakeForces() const;
	bool	IsEnabled() const				{ return m_enabled; }
	bool	IsMarkedForDestroy() const		{ return m_isMarkedForDestroy; }
	float 	GetDrag() const					{ return m_drag; }

public:
	bool			m_isMarkedForDestroy	= false;
	bool			m_enabled				= true;
	SimulationMode	m_simulationMode		= SIMULATION_MODE_DYNAMIC;

	Vec2			m_worldPosition;
	Vec2			m_velocity;
	Vec2			m_frameForces;
	float			m_mass					= 1.0f;
	float			m_drag					= 0.0f;

	Vec2			m_positionLastFrame;

	Physics2D*		m_physicsSystem	= nullptr;
	Collider2D*		m_collider		= nullptr;

private:
	~Rigidbody2D();
};