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

	void	TakeCollider( Collider2D* collider );
	Vec2	GetFrameAcceleration();
	float	GetFrameAngularAcceleration();
	Vec2	GetVelocity() const;
	Vec2	GetImpactVelocityAtPoint( Vec2 const& point ) const;
	void	UpdateVerletVelocity( float frameTime );
	float	GetRotationDegrees() const;
	float	GetRotationRadians() const								{ return m_rotationRadians; }
	float	GetAngularVelocity() const								{ return m_angularVelocity; }
	float	GetFrameTorque() const									{ return m_frameTorque; }
	float	GetMoment() const										{ return m_moment; }

	void SetPosition( Vec2 position );
	void SetEnabled( bool enable );
	void SetSimulationMode( SimulationMode simulationMode );
	void SetVelocity( Vec2 const& newVelocity );
	void SetMass( float mass );
	void SetDrag( float drag );
	void SetRotationRadians( float rotationRadians );
	void SetRotationDegrees( float rotationDegrees );
	void AddRotationRadians( float rotationRadiansToAdd );
	void AddRotationDegrees( float rotationDegreesToAdd );
	void SetAngularVelocity( float angularVelocity );
	void AddAngularVelocity( float angularVelocityToAdd );
	void SetMoment( float moment );
	void MarkForDestroy( bool isMarkedForDestroy );

	void ApplyImpulseAt( Vec2 const& point, Vec2 const& impulse );
	void ApplyFrictionAt( Vec2 const& worldPos, float frictionCoefficient, Vec2 const& collisionNormal, float normalImpulse, Vec2 const& collisionTangent, float tangentImpulse );
	void ApplyDragForce();
	void CalculateMoment( float oldMass );

	void AddForce( Vec2 const& forceToAdd );
	void AddFrameTorque( float torqueToAdd );
	void AddForceFromAcceleration( const Vec2& acceleration );
	void AddDrag( float dragToAdd );

	void DebugRender( RenderContext* context ) const;

	bool	IsSimulated() const;
	bool	DoesTakeForces() const;
	bool	IsEnabled() const				{ return m_enabled; }
	bool	IsMarkedForDestroy() const		{ return m_isMarkedForDestroy; }
	float 	GetDrag() const					{ return m_drag; }
	Vec2	GetVerletVelocity() const		{ return m_verletVelocity; }

public:
	bool			m_isMarkedForDestroy	= false;
	bool			m_enabled				= true;
	SimulationMode	m_simulationMode		= SIMULATION_MODE_DYNAMIC;

	float			m_mass					= 1.0f;
	Vec2			m_worldPosition;
	Vec2			m_velocity;
	Vec2			m_verletVelocity;
	Vec2			m_frameForces;
	float			m_drag					= 0.0f;

	float			m_rotationRadians		= 0.f;
	float			m_angularVelocity		= 0.f;
	float			m_frameTorque			= 0.f;
	float			m_moment				= 0.f;

	Vec2			m_frameStartPosition;
	Vec2			m_positionLastFrame;

	Physics2D*		m_physicsSystem	= nullptr;
	Collider2D*		m_collider		= nullptr;

private:
	~Rigidbody2D();
};