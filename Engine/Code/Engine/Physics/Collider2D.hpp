#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>

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
class	PhysicsMaterial;
struct	Vec2;
struct	Rgba8;
struct	Manifold2;


typedef bool( *collision_check_cb )( Collider2D const*, Collider2D const* );
typedef bool( *manifold_check_cb )( Collider2D const*, Collider2D const*, Manifold2* );


//---------------------------------------------------------------------------------------------------------
class Collider2D
{
	friend class Physics2D;

public:
	virtual void	Destroy()										= 0;
	virtual void	UpdateWorldShape()								= 0;
	virtual void	Move( Vec2 const& movement )					= 0;

	virtual Vec2	GetClosestPoint(Vec2 const& position ) const	= 0;
	virtual bool	Contains( Vec2 const& position ) const			= 0;
	virtual void	SetWorldBounds()								= 0;

	virtual float	CalculateMoment( float mass )					= 0;

	virtual void	DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor ) = 0;


	//---------------------------------------------------------------------------------------------------------
	Collider2DType	GetType() const				{ return m_type; }
	AABB2			GetWorldBounds() const		{ return m_worldBounds; }
	bool			isMarkedForDestroy() const	{ return m_isMarkedForDestroy; }
	float			GetMass() const;
	Vec2			GetVelocity() const;

	void			MarkForDestroy( bool isMarkedForDestroy );

	bool			Intersects( Collider2D const* other ) const;
	bool			GetManifold( Collider2D const* other, Manifold2* manifold );
	bool			WorldBoundsIntersect( Collider2D const* other ) const;
	float			GetBounceWith( Collider2D const* other ) const;
	float			GetFrictionWith( Collider2D const* other ) const;
	float			GetPhysicsMaterialBounciness() const;
	float			GetPhysicsMaterialFriction() const;

protected:
	virtual ~Collider2D();

public:
	bool				m_isMarkedForDestroy	= false;
	Collider2DType		m_type;
	Physics2D*			m_physicsSystem			= nullptr;
	Rigidbody2D*		m_rigidbody				= nullptr;
	PhysicsMaterial*	m_physicsMaterial		= nullptr;
	AABB2				m_worldBounds;
// 	float				m_worldBoundsRadius		= 0.f;
// 	Vec2				m_worldBoundsCenter;
};