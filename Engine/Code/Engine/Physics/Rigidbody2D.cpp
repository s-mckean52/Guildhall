#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::Destroy()
{
	m_physicsSystem->DestroyRigidbody2D( this );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::TakeCollider( Collider2D* collider )
{
	if( m_collider != nullptr )
	{
		m_physicsSystem->DestroyCollider2D( m_collider );
	}

	m_collider = collider;
	m_collider->m_rigidbody = this;
	m_collider->UpdateWorldShape();
}


//---------------------------------------------------------------------------------------------------------
Vec2 Rigidbody2D::GetFrameAcceleration()
{
	Vec2 acceleration = m_frameForces / m_mass;
	return acceleration;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Rigidbody2D::GetVelocity() const
{
	return m_velocity;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetPosition( Vec2 position )
{
	m_worldPosition = position;
	if( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetEnabled( bool isEnabled )
{
	m_enabled = isEnabled;
	if( !isEnabled )
	{
		m_velocity = Vec2();
	}
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetSimulationMode( SimulationMode simulationMode )
{
	m_simulationMode = simulationMode;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetVelocity( Vec2 const& newVelocity )
{
	m_velocity = newVelocity;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetMass( float mass )
{
	m_mass = mass;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetDrag( float drag )
{
	m_drag = drag;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::MarkForDestroy( bool isMarkedForDestroy )
{
	m_isMarkedForDestroy = isMarkedForDestroy;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyImpulseAt( const Vec2& worldPos, const Vec2& impulse )
{
	UNUSED( worldPos );
	m_velocity += impulse * ( 1 / m_mass );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddForceFromAcceleration( const Vec2& acceleration )
{
	m_frameForces += m_mass * acceleration;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddDrag( float dragToAdd )
{
	m_drag += dragToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::DebugRender( RenderContext* context ) const
{
	Rgba8 xColor;
	float xSize = 5.f;
	Vec2 xOne = Vec2( xSize, xSize );
	Vec2 xTwo = Vec2( xSize, -xSize );

	Vec2 start1 = m_worldPosition + xOne;
	Vec2 end1 = m_worldPosition - xOne;
	Vec2 start2 = m_worldPosition + xTwo;
	Vec2 end2 = m_worldPosition - xTwo;

	std::vector<Vertex_PCU> vertexArray;
	if( IsEnabled() )
	{
		xColor = Rgba8::BLUE;
	}
	else
	{
		xColor = Rgba8::RED;
	}
	AppendVertsForLineBetweenPoints( vertexArray, start1, end1, xColor, 3.f );
	AppendVertsForLineBetweenPoints( vertexArray, start2, end2, xColor, 3.f );

	context->BindTexture( nullptr );
	context->BindShader( (Shader*)nullptr );
	context->DrawVertexArray( vertexArray );
}


//---------------------------------------------------------------------------------------------------------
bool Rigidbody2D::DoesTakeForces() const
{
	if( !IsEnabled() )
	{
		return false;
	}
	else if( m_simulationMode == SIMULATION_MODE_STATIC || m_simulationMode == SIMULATION_MODE_KINEMATIC)
	{
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool Rigidbody2D::IsSimulated() const
{
	if( !IsEnabled() )
	{
		return false;
	}
	if( m_simulationMode == SIMULATION_MODE_DYNAMIC || m_simulationMode == SIMULATION_MODE_KINEMATIC )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
Rigidbody2D::~Rigidbody2D()
{
	GUARANTEE_OR_DIE( m_collider == nullptr, "Collider was not destroyed before rigidbody delete" );
}

