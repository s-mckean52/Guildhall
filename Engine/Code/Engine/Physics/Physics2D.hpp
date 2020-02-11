#pragma once
#include <vector>

class	Rigidbody2D;
class	Collider2D;
class	DiscCollider2D;
class	PolygonCollider2D;
struct	Vec2;

class Physics2D
{
public:
	Physics2D();
	~Physics2D();
	
	void BeginFrame();
	void Update();
	void EndFrame();

	Rigidbody2D* CreateRigidbody2D();
	DiscCollider2D* CreateDiscCollider2D( Vec2 localPosition, float radius );
	PolygonCollider2D* CreatePolygonCollider2D( std::vector<Vec2> polygonVerts, Vec2 localPosition );

	void DestroyRigidbody2D( Rigidbody2D* rigidbody );
	void DestroyCollider2D( Collider2D* collider );

	Collider2D* AddColliderToVector( Collider2D* newCollider );

public:
	std::vector< Rigidbody2D* > m_rigidbodies2D;
	std::vector< Rigidbody2D* > m_rigidbodies2DToBeDestroyed;
	std::vector< Collider2D* > m_colliders2D;
	std::vector< Collider2D* > m_colliders2DToBeDestroyed;
};