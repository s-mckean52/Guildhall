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
	Vec2 GetFrameAcceleration( float gravity );

	void SetPosition( Vec2 position );
	void SetEnabled( bool enable );
	void SetSimulationMode( SimulationMode simulationMode );

	void DebugRender( RenderContext* context ) const;

	bool DoesMove() const;
	bool IsEnabled() const { return m_enabled; }

public:
	bool			m_enabled			= true;
	SimulationMode	m_simulationMode	= SIMULATION_MODE_DYNAMIC;
	Vec2			m_worldPosition;
	Vec2			m_velocity;

	Physics2D* m_physicsSystem	= nullptr;
	Collider2D* m_collider		= nullptr;

private:
	~Rigidbody2D();
};