#pragma once

enum Collider2DType
{
	COLLIDER_TYPE_DISC2D,

	NUM_COLLIDER_TYPE
};


class	RenderContext;
class	Physics2D;
class	Rigidbody2D;
struct	Vec2;
struct	Rgba8;


class Collider2D
{
	friend class Physics2D;

public:
	virtual void Destroy()										= 0;
	virtual void UpdateWorldShape()								= 0;

	virtual Vec2 GetClosestPoint(Vec2 const& position ) const	= 0;
	virtual bool Contains( Vec2 const& position ) const			= 0;
	virtual bool Intersects( Collider2D const* other ) const	= 0;

	virtual void DebugRender( RenderContext* context, Rgba8 const& borderColor, Rgba8 const& fillColor ) = 0;

protected:
	virtual ~Collider2D();

public:
	Collider2DType m_type;
	Physics2D* m_physicsSystem	= nullptr;
	Rigidbody2D* m_rigidbody	= nullptr;
};