#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Timer.hpp"
#include <vector>

class	Rigidbody2D;
class	Collider2D;
class	DiscCollider2D;
class	PolygonCollider2D;
class	Clock;
struct	Collision2D;

class Physics2D
{
public:
	Physics2D( Clock* gameClock );
	~Physics2D();
	
	void BeginFrame();
	void Update();
	void EndFrame();

	void AdvanceSimulation( float deltaSeconds );
	void ApplyEffectors( float deltaSeconds );
	void MoveRigidbodies( float deltaSeconds );
	void UpdateFrameStartPositions();
	void UpdateVerletVelocities();
	void DetectCollisions();
	void ResolveCollisions();
	void ResolveCollision( Collision2D const& collision );
	void EulerStep( float deltaSeconds, Rigidbody2D* rb );
	void ApplyImpulseOnCollision( Collision2D const& collision );

	void ClearFrameData();
	void AddGravityInDownDirection( float gravityToAdd );
	
	void SetSceneGravity();
	void ApplyDragOnRigidbodies();
	void SetClock( Clock* clock );
	void SetFixedDeltaTime( double newFixedDeltaTime );
	void ToggleClockPause();
	void SetClockScale( double clockScale );

	float	GetGravityAmount() const	{ return m_gravityAcceleration.y; }
	double	GetFixedDeltaTime() const	{ return m_fixedDeltaTime; }

	//---------------------------------------------------------------------------------------------------------
	// Create and Destroy Physics objects
	Rigidbody2D* CreateRigidbody2D();
	DiscCollider2D* CreateDiscCollider2D( Vec2 localPosition, float radius );
	PolygonCollider2D* CreatePolygonCollider2D( std::vector<Vec2> polygonVerts, Vec2 localPosition );

	void DestroyRigidbody2D( Rigidbody2D* rigidbody );
	void DestroyCollider2D( Collider2D* collider );

	Collider2D* AddColliderToVector( Collider2D* newCollider );

public:
	Clock* m_clock = nullptr;
	Timer m_stepTimer;
	double m_fixedDeltaTime = 1.0 / 120.0;

	Vec2 m_gravityAcceleration = Vec2( 0.0f, -9.81f );

	std::vector< Collision2D* > m_frameCollisions;
	std::vector< Rigidbody2D* > m_rigidbodies2D;
	std::vector< Collider2D* > m_colliders2D;
};