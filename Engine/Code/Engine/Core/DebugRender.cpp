#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include <vector>


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
class DebugRenderSystem
{
public:
	~DebugRenderSystem();

	void SetIsRenderingEnabled( bool isRenderingEnabled );
	void SetRenderContext( RenderContext* context );
	void SetCamera( Camera* camera );

	bool IsRenderingEnabled() const			{ return m_isRenderingEnabled; }
	RenderContext* GetRenderContext() const	{ return m_context; }
	Camera* GetCamera() const				{ return m_camera; }

private:
	bool m_isRenderingEnabled = false;

	RenderContext* m_context = nullptr;
	Camera* m_camera = nullptr;
};


DebugRenderSystem::~DebugRenderSystem()
{
	delete m_camera;
	m_camera = nullptr;
}

void DebugRenderSystem::SetIsRenderingEnabled( bool isRenderingEnabled )
{
	m_isRenderingEnabled = isRenderingEnabled;
}

void DebugRenderSystem::SetRenderContext( RenderContext* context )
{
	m_context = context;
}

void DebugRenderSystem::SetCamera( Camera* camera )
{
	m_camera = camera;
}
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	bool HasExpired() const { return m_hasExpired; }
	void AddVertsAndIndiciesToArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies );

public:
	Mat44 m_transformMatrix;

	std::vector<Vertex_PCU> m_objectVerticies;
	std::vector<unsigned int> m_objectIndicies;

	bool m_hasExpired = false;
	bool m_isMarkedForDestroy = false;

	Rgba8 m_startColor;
	Rgba8 m_endColor;

	Timer m_durationTimer;
};


void DebugRenderObject::AddVertsAndIndiciesToArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies )
{
	m_hasExpired = m_durationTimer.CheckAndDecrement();

	Rgba8 colorDifference = m_endColor - m_startColor;
	colorDifference *= m_durationTimer.GetFractionComplete();
	Rgba8 newColor = m_startColor + colorDifference;

	unsigned int startIndexOffset = static_cast<unsigned int>( verticies.size() );
	for( int objectVertIndex = 0; objectVertIndex < m_objectVerticies.size(); ++objectVertIndex )
	{
		Vertex_PCU newVert = m_objectVerticies[ objectVertIndex ];
		newVert.m_color = newColor;
		newVert.m_position = m_transformMatrix.TransformPosition3D( newVert.m_position );
		verticies.push_back( newVert );
	}

	for( int objectIndiciesIndex = 0; objectIndiciesIndex < m_objectIndicies.size(); ++objectIndiciesIndex )
	{
		indicies.push_back( m_objectIndicies[ objectIndiciesIndex ] + startIndexOffset );
	}
}
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


static DebugRenderSystem* s_debugRenderSystem = nullptr;
static std::vector<DebugRenderObject*> s_debugRenderWorldObjects;
static std::vector<DebugRenderObject*> s_debugRenderScreenObjects;


//---------------------------------------------------------------------------------------------------------
void DebugRenderSystemStartup()
{
	s_debugRenderSystem = new DebugRenderSystem();
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
	delete s_debugRenderSystem;
	s_debugRenderSystem = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void EnableDebugRendering()
{
	s_debugRenderSystem->SetIsRenderingEnabled( true );
}


//---------------------------------------------------------------------------------------------------------
void DisableDebugRendering()
{
	s_debugRenderSystem->SetIsRenderingEnabled( false );
}


//---------------------------------------------------------------------------------------------------------
void ClearDebugRendering()
{
	for( int objectIndex = 0; objectIndex < s_debugRenderWorldObjects.size(); ++objectIndex )
	{
		s_debugRenderWorldObjects[ objectIndex ]->m_isMarkedForDestroy = true;
	}

	for (int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex)
	{
		s_debugRenderScreenObjects[ objectIndex ]->m_isMarkedForDestroy = true;
	}
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* cam )
{
	if( !s_debugRenderSystem->IsRenderingEnabled() ) return;

	RenderContext* context = cam->GetRenderContext();
	s_debugRenderSystem->SetRenderContext( context );

	Camera* debugCamera = nullptr;
	if( s_debugRenderSystem->GetCamera() != nullptr )
	{
		debugCamera = s_debugRenderSystem->GetCamera();
	}
	else
	{
		debugCamera = new Camera( context );
	}

	debugCamera->SetTransform( cam->GetTransform() );
	debugCamera->SetColorTarget( cam->GetColorTarget() );
	debugCamera->SetProjectionMatrix( cam->GetProjectionMatrix() );
	debugCamera->SetViewMatrix( cam->GetViewMatrix() );

	s_debugRenderSystem->SetCamera( debugCamera );

	std::vector<Vertex_PCU> verticies;
	std::vector<unsigned int> indicies;
	for( int objectIndex = 0; objectIndex < s_debugRenderWorldObjects.size(); ++objectIndex )
	{
		DebugRenderObject* object = s_debugRenderWorldObjects[ objectIndex ];

		if( object == nullptr ) continue;

		object->AddVertsAndIndiciesToArray( verticies, indicies );
		if( object->HasExpired() )
		{
			object->m_isMarkedForDestroy = true;
		}
	}

	context->BeginCamera( *debugCamera );

	context->SetModelMatrix( Mat44::IDENTITY );
	context->BindTexture( nullptr);
	context->BindShader( (Shader*)nullptr );
	context->DrawIndexedVertexArray( verticies, indicies );

	context->EndCamera( *debugCamera );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{
	if( !s_debugRenderSystem->IsRenderingEnabled() ) return;

	RenderContext* context = output->GetOwner();
	s_debugRenderSystem->SetRenderContext( context );

	Camera* camera = nullptr;
	if( s_debugRenderSystem->GetCamera() != nullptr )
	{
		camera = s_debugRenderSystem->GetCamera();
	}
	else
	{
		camera = new Camera( context );
	}
	camera->SetColorTarget( output );
	Vec2 min = Vec2::ZERO;
	Vec2 max = output->GetSize();
	camera->SetProjectionOrthographic( max.y );
	camera->SetTransform( Transform() );

	s_debugRenderSystem->SetCamera( camera );

	std::vector<Vertex_PCU> verticies;
	std::vector<unsigned int> indicies;
	for( int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex )
	{
		DebugRenderObject* object = s_debugRenderScreenObjects[ objectIndex ];

		if( object == nullptr ) continue;

		object->AddVertsAndIndiciesToArray( verticies, indicies );
		if( object->HasExpired() )
		{
			object->m_isMarkedForDestroy = true;
		}
	}

	context->BeginCamera( *camera );

	context->SetModelMatrix( Mat44::IDENTITY );
	context->BindTexture( nullptr );
	context->BindShader( (Shader*)nullptr );
	context->DrawIndexedVertexArray( verticies, indicies );

	context->EndCamera( *camera );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	for( int objectIndex = 0; objectIndex < s_debugRenderWorldObjects.size(); ++objectIndex )
	{
		if( s_debugRenderWorldObjects[ objectIndex ] && s_debugRenderWorldObjects[ objectIndex ]->m_isMarkedForDestroy )
		{
			delete s_debugRenderWorldObjects[ objectIndex ];
			s_debugRenderWorldObjects[ objectIndex ] = nullptr;
		}
	}

	for( int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex )
	{
		if( s_debugRenderScreenObjects[ objectIndex ] && s_debugRenderScreenObjects[ objectIndex ]->m_isMarkedForDestroy)
		{
			delete s_debugRenderScreenObjects[ objectIndex ];
			s_debugRenderScreenObjects[ objectIndex ] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( pos );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = start_color;
	object->m_endColor = end_color;

	float halfSize = size * 0.5f;
	Vec3 min = Vec3( -halfSize, -halfSize, pos.z );
	Vec3 max = Vec3( halfSize, halfSize, pos.z );
	AABB2 box = AABB2( min.x, min.y, max.x, max.y );

	AppendVertsForAABB2D( object->m_objectVerticies, box, start_color );
	object->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	s_debugRenderWorldObjects.push_back( object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, size, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 pos, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, 1.0f, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height )
{

}


//---------------------------------------------------------------------------------------------------------
AABB2 DebugGetScreenBounds()
{
	return AABB2();
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration )
{
	DebugRenderObject* screenObject = new DebugRenderObject();

	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( pos );
	screenObject->m_startColor = start_color;
	screenObject->m_endColor = end_color;

	float halfSize = size * 0.5f;
	Vec2 min = Vec2( -halfSize, -halfSize );
	Vec2 max = Vec2( halfSize, halfSize );
	AABB2 box = AABB2( min, max );

	AppendVertsForAABB2D( screenObject->m_objectVerticies, box, start_color );
	screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	s_debugRenderScreenObjects.push_back( screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration )
{
	DebugAddScreenPoint( pos, size, color, color, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, Rgba8 color )
{
	DebugAddScreenPoint( pos, 10.f, color, color, 0.f );
}

