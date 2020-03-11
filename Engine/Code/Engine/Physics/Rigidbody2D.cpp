#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

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
	CalculateMoment( 1.f );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Rigidbody2D::GetFrameAcceleration()
{
	Vec2 acceleration = m_frameForces / m_mass;
	return acceleration;
}


//---------------------------------------------------------------------------------------------------------
float Rigidbody2D::GetFrameAngularAcceleration()
{
	float angularAaceleration = m_frameTorque / m_moment;
	return angularAaceleration;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Rigidbody2D::GetVelocity() const
{
	return m_velocity;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Rigidbody2D::GetImpactVelocityAtPoint( Vec2 const& point ) const
{
	Vec2 displacementToPoint = point - m_worldPosition;
	Vec2 tangentialRotationVelocity = displacementToPoint.GetRotated90Degrees();

	return GetVerletVelocity() + ( tangentialRotationVelocity * m_angularVelocity );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::UpdateVerletVelocity( float frameTime )
{
	Vec2 frameDispalcement = m_worldPosition - m_frameStartPosition;
	m_verletVelocity = frameDispalcement / frameTime;
}


//---------------------------------------------------------------------------------------------------------
float Rigidbody2D::GetRotationDegrees() const
{
	return ConvertRadiansToDegrees( m_rotationRadians );
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
	float oldMass = m_mass;
	m_mass = mass;
	CalculateMoment( oldMass );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetDrag( float drag )
{
	m_drag = drag;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetRotationRadians( float rotationRadians )
{
	const float twoPi = 2.f * 3.14159265f;

	m_rotationRadians = rotationRadians;
	if( m_rotationRadians >= twoPi )
	{
		m_rotationRadians -= twoPi;
	}
	else if( m_rotationRadians < 0.f )
	{
		m_rotationRadians = twoPi - m_rotationRadians;
	}

	if( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetRotationDegrees( float rotationDegrees )
{
	SetRotationRadians( ConvertDegreesToRadians( rotationDegrees ) );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddRotationRadians( float rotationRadiansToAdd )
{
	float newRotationRadians = m_rotationRadians + rotationRadiansToAdd;
	SetRotationRadians( newRotationRadians );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddRotationDegrees( float rotationDegreesToAdd )
{
	AddRotationRadians( ConvertDegreesToRadians( rotationDegreesToAdd ) );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetAngularVelocity( float angularVelocity )
{
	m_angularVelocity = angularVelocity;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddAngularVelocity(float angularVelocityToAdd)
{
	m_angularVelocity += angularVelocityToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetMoment( float moment )
{
	m_moment = moment;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::MarkForDestroy( bool isMarkedForDestroy )
{
	m_isMarkedForDestroy = isMarkedForDestroy;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyImpulseAt( const Vec2& worldContactPosition, const Vec2& impulse )
{
	float inverseMass = 1.f / m_mass;
	m_velocity += impulse * inverseMass;
	
	Vec2 displacementToContact = worldContactPosition - m_worldPosition;
	float torqueImpulse = ( -impulse.x * displacementToContact.y ) + ( impulse.y * displacementToContact.x );
	m_angularVelocity += torqueImpulse / m_moment;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyFrictionAt( Vec2 const& worldContactPosition, float frictionCoefficient, Vec2 const& collisionNormal, float normalImpulse, Vec2 const& collisionTangent, float tangentImpulse )
{
	UNUSED( collisionNormal );
	if( abs( tangentImpulse ) > frictionCoefficient * normalImpulse )
	{
		tangentImpulse = Signf( tangentImpulse ) * normalImpulse * frictionCoefficient;
	}
	ApplyImpulseAt( worldContactPosition, collisionTangent * tangentImpulse );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyDragForce()
{
	Vec2 dragForce = -m_verletVelocity * m_drag;
	AddForce( dragForce );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::CalculateMoment( float oldMass )
{
	if( m_moment == 0.f )
	{
		m_moment = m_collider->CalculateMoment( m_mass );
	}
	else
	{
		m_moment *= m_mass / oldMass;
	}
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddForce( Vec2 const& forceToAdd )
{
	m_frameForces += forceToAdd;
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::AddFrameTorque( float torqueToAdd )
{
	m_frameTorque = torqueToAdd;
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

