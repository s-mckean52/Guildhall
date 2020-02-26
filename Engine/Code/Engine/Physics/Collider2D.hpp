#pragma once
#include "Engine/Math/AABB2.hpp"

enum Collider2DType
{
	COLLIDER_TYPE_DISC2D,
	COLLIDER_TYPE_POLYGON2D,

	NUM_COLLIDER_TYPE
};


class	RenderContext;
class	Physics2D;
class	Rigidbody2D;
class	Collider2D;
struct	Vec2;
struct	Rgba8;
struct	AABB2;


//---------------------------------------------------------------------------------------------------------
// Collision Matrix
typedef bool(*collision_check_cb)(Collider2D const*, Collider2D const*);

// static bool DiscVDiscCollisionCheck( Collider2D const* col0, Collider2D const* col1 );
// static bool DiscVPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 );
// static bool PolygonVPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 );
// 
// static collision_check_cb s_collisionCheck[ NUM_COLLIDER_TYPE * NUM_COLLIDER_TYPE ] = {
// 	/*				disc,							polygon		*/
// 	/*	   disc	*/	DiscVDiscCollisionCheck,		nullptr,
// 	/*	polygon	*/	DiscVPolygonCollisionCheck,		PolygonVPolygonCollisionCheck,
// };


//---------------------------------------------------------------------------------------------------------
class Collider2D
{
	friend class Physics2D;

public:
	virtual void Destroy()										= 0;
	virtual void UpdateWorldShape()								= 0;

	virtual Vec2	GetClosestPoint(Vec2 const& position ) const	= 0;
	virtual bool	Contains( Vec2 const& position ) const			= 0;
	virtual void	SetWorldBounds()								= 0;

	virtual void DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor ) = 0;

	bool Intersects( Collider2D const* other ) const;
	Collider2DType GetType() const { return m_type; }

protected:
	virtual ~Collider2D();

public:
	Collider2DType m_type;
	Physics2D* m_physicsSystem	= nullptr;
	Rigidbody2D* m_rigidbody	= nullptr;
	AABB2 m_worldBounds;
};