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
Vec2 Rigidbody2D::GetFrameAcceleration( float gravity )
{
	return Vec2();
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
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetSimulationMode( SimulationMode simulationMode )
{
	m_simulationMode = simulationMode;
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
	context->DrawVertexArray( vertexArray );
}


//---------------------------------------------------------------------------------------------------------
bool Rigidbody2D::DoesMove() const
{
	if( IsEnabled() )
	{
		return false;
	}
	else if( m_simulationMode == SIMULATION_MODE_STATIC )
	{
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
Rigidbody2D::~Rigidbody2D()
{
	GUARANTEE_OR_DIE( m_collider == nullptr, "Collider was not destroyed before rigidbody delete" );
}

