#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <vector>
#include <stdarg.h>


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
class DebugRenderSystem
{
public:
	~DebugRenderSystem();

	void SetIsRenderingEnabled( bool isRenderingEnabled );
	void SetRenderContext( RenderContext* context );
	void SetCamera( Camera* camera );
	void SetScreenHeight( float height );

	bool IsRenderingEnabled() const			{ return m_isRenderingEnabled; }
	RenderContext* GetRenderContext() const	{ return m_context; }
	Camera* GetCamera() const				{ return m_camera; }
	float GetScreenHeight() const			{ return m_screenHeight; }

private:
	bool m_isRenderingEnabled = false;
	float m_screenHeight = 1080.f;

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

void DebugRenderSystem::SetScreenHeight(float height)
{
	m_screenHeight = height;
}

//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


static DebugRenderSystem* s_debugRenderSystem = nullptr;
static BitmapFont* s_debugRenderFont = nullptr;


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	bool HasExpired() const { return m_hasExpired; }
/*	void AddVertsAndIndiciesToArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies );*/
	void SetAllVertsColor( const Rgba8& newColor );
	void SetRenderMode();
	void Draw();

public:
	Mat44 m_transformMatrix;

	std::vector<Vertex_PCU> m_objectVerticies;
	std::vector<unsigned int> m_objectIndicies;

	const Texture* m_texture = nullptr;
	bool m_hasExpired = false;
	bool m_isMarkedForDestroy = false;
	bool m_isBillboarded = false;
	bool m_isWireMesh = false;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	Timer m_durationTimer;
	eDebugRenderMode m_renderMode = DEBUG_RENDER_USE_DEPTH;
};


//---------------------------------------------------------------------------------------------------------
void DebugRenderObject::Draw()
{
	m_hasExpired = m_durationTimer.HasElapsed();

	if( m_isBillboarded )
	{
		Vec3 cameraPosition = s_debugRenderSystem->GetCamera()->GetPosition();
		m_transformMatrix = Mat44::LookAt( m_transformMatrix.GetTranslation3D(), cameraPosition );
	}

	RenderContext* context = s_debugRenderSystem->GetRenderContext();

	if( m_isWireMesh )
	{
		context->SetCullMode( CULL_MODE_NONE );
		context->SetFillMode( FILL_MODE_WIREFRAME );
	}
	else
	{
		context->SetCullMode( CULL_MODE_BACK );
		context->SetFillMode( FILL_MODE_SOLID );
	}

 	context->SetModelUBO( m_transformMatrix );
 	context->BindTexture( m_texture );
 	context->BindShader( (Shader*)nullptr );

	SetRenderMode();

	Rgba8 newColor = Rgba8Lerp( m_startColor, m_endColor, m_durationTimer.GetFractionComplete() );
	SetAllVertsColor( newColor );

	context->DrawIndexedVertexArray( m_objectVerticies, m_objectIndicies );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderObject::SetAllVertsColor( const Rgba8& newColor )
{
	for (int objectVertIndex = 0; objectVertIndex < m_objectVerticies.size(); ++objectVertIndex)
	{
		m_objectVerticies[ objectVertIndex ].m_color = newColor;
	}
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderObject::SetRenderMode()
{
	RenderContext* context = s_debugRenderSystem->GetRenderContext();

	switch( m_renderMode )
	{
	case DEBUG_RENDER_ALWAYS:
		context->SetDepthTest( COMPARE_FUNC_ALWAYS, false );
		break;
	case DEBUG_RENDER_USE_DEPTH:
		context->SetDepthTest( COMPARE_FUNC_LEQUAL, true );
		break;
	case DEBUG_RENDER_XRAY:
	{
		float xRaySubtractValue = 70.f;
		context->SetDepthTest( COMPARE_FUNC_GEQUAL, false );
		Rgba8 xRayColor = Rgba8Lerp( m_startColor, m_endColor, m_durationTimer.GetFractionComplete() );
		xRayColor.r = static_cast<unsigned char>( GetClamp(static_cast<float>( xRayColor.r ) - xRaySubtractValue, 0.0f, 255.0f ) );
		xRayColor.g = static_cast<unsigned char>( GetClamp(static_cast<float>( xRayColor.g ) - xRaySubtractValue, 0.0f, 255.0f ) );
		xRayColor.b = static_cast<unsigned char>( GetClamp(static_cast<float>( xRayColor.b ) - xRaySubtractValue, 0.0f, 255.0f ) );
		SetAllVertsColor( xRayColor );
		context->DrawIndexedVertexArray( m_objectVerticies, m_objectIndicies );
		context->SetDepthTest( COMPARE_FUNC_LEQUAL, true );
		break;
	}
	default:
		break;
	}
}
//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


static std::vector<DebugRenderObject*> s_debugRenderWorldObjects;
static std::vector<DebugRenderObject*> s_debugRenderScreenObjects;


//---------------------------------------------------------------------------------------------------------
void AppendDebugRenderObjectToVector( std::vector<DebugRenderObject*>& vectorToAppendTo, DebugRenderObject* objectToAppend )
{
	for( int objectIndex = 0; objectIndex < vectorToAppendTo.size(); ++objectIndex )
	{
		if( vectorToAppendTo[ objectIndex ] == nullptr )
		{
			vectorToAppendTo[ objectIndex ] = objectToAppend;
			return;
		}
	}
	vectorToAppendTo.push_back( objectToAppend );
}


//---------------------------------------------------------------------------------------------------------
eDebugRenderMode GetDebugRenderModeFromString( std::string renderModeAsString )
{
	if( renderModeAsString == "depth" )
	{
		return DEBUG_RENDER_USE_DEPTH;
	}
	else if( renderModeAsString == "xray" )
	{
		return DEBUG_RENDER_XRAY;
	}
	else if( renderModeAsString == "always" )
	{
		return DEBUG_RENDER_ALWAYS;
	}
	else
	{
		g_theConsole->PrintString( Rgba8::MAGENTA, Stringf( "Defaulted Render Mode to ALWAYS" ) );
		return DEBUG_RENDER_ALWAYS;
	}
}


//---------------------------------------------------------------------------------------------------------
eDebugRenderMode GetRenderModeFromArgs( NamedStrings* args )
{
	std::string defaultModeAsString = "depth";
	std::string modeAsString = args->GetValue( "mode", defaultModeAsString );

	return GetDebugRenderModeFromString( modeAsString );
}


//---------------------------------------------------------------------------------------------------------
static void debug_render( NamedStrings* args )
{
	bool defaultEnabled = true;
	bool enabled = args->GetValue( "enabled", defaultEnabled );
	
	enabled ? EnableDebugRendering() : DisableDebugRendering();

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Debug Rendering is %s", enabled ? "enabled" : "disabled" ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_world_point( NamedStrings* args )
{
	Vec3 defaultPosition = Vec3::ZERO;
	float defaultDurationSeconds = 0.0f;

	Vec3 position = args->GetValue( "position", defaultPosition );
	float durationSeconds = args->GetValue( "duration", defaultDurationSeconds );

	eDebugRenderMode mode = GetRenderModeFromArgs( args );
	DebugAddWorldPoint( position, Rgba8::WHITE, durationSeconds, mode );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Draw world point at - x:%.2f, y:%.2f, z:%.2f - for %.2f seconds", position.x, position.y, position.z, durationSeconds ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_world_wire_sphere( NamedStrings* args )
{
	Vec3 defaultPosition = Vec3::ZERO;
	float defaultRadius = 1.f;
	float defaultDuration = 0.f;

	Vec3 position = args->GetValue( "position", defaultPosition );
	float radius = args->GetValue( "radius", defaultRadius );
	float duration = args->GetValue( "duration", defaultDuration );

	eDebugRenderMode mode = GetRenderModeFromArgs( args );
	DebugAddWorldWireSphere( position, radius, Rgba8::MAGENTA, duration, mode );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Draw wire sphere at - x:%.2f, y:%.2f, z:%.2f - with radius %.2f - for %.2f seconds", position.x, position.y, position.z, radius, duration ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_world_wire_bounds(NamedStrings* args)
{
	Vec3 defaultMinPosition = Vec3::ZERO;
	Vec3 defaultMaxPosition = Vec3::UNIT;
	float defaultDuration = 0.f;

	Vec3 minPosition = args->GetValue( "min", defaultMinPosition );
	Vec3 maxPosition = args->GetValue( "max", defaultMaxPosition );
	float duration = args->GetValue( "duration", defaultDuration );

	AABB3 bounds = AABB3( minPosition, maxPosition );
	eDebugRenderMode mode = GetRenderModeFromArgs( args );
	DebugAddWorldWireBounds( bounds, Rgba8::BLUE, duration, mode );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, 
								 Stringf("Draw wire bounds at min - x:%.2f, y:%.2f, z:%.2f max - x:%.2f, y:%.2f, z:%.2f - for %.2f seconds",
								 minPosition.x, minPosition.y, minPosition.z, maxPosition.x, maxPosition.y, maxPosition.z, duration ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_world_billboard_text( NamedStrings* args )
{
	Vec3 defaultPosition = Vec3::ZERO;
	Vec2 defaultPivot = Vec2::ZERO;
	float defaultDuration = 10.f;
	std::string defaultText = "default text";

	Vec3 position = args->GetValue( "position", defaultPosition );
	Vec2 pivot = args->GetValue( "pivot", defaultPivot );
	float duration = args->GetValue( "duration", defaultDuration );
	std::string text = args->GetValue( "text", defaultText );

	eDebugRenderMode mode = GetRenderModeFromArgs( args );
	DebugAddWorldBillboardTextf( position, pivot, Rgba8::GREEN, duration, mode, text.c_str() );

	if( g_theConsole != nullptr)
	{
		g_theConsole->PrintString(	Rgba8::WHITE,
									Stringf( "Draw billboard text at - x:%.2f, y:%.2f, z:%.2f pivot - x:%.2f, y:%.2f - for %.2f seconds",
									position.x, position.y, position.z, pivot.x, pivot.y, duration ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_screen_point( NamedStrings* args )
{
	Vec2 defaultPosition = Vec2::UNIT * 12.5f;
	float defaultDurationSeconds = 0.0f;

	Vec2 position = args->GetValue( "position", defaultPosition );
	float durationSeconds = args->GetValue( "duration", defaultDurationSeconds );

	DebugAddScreenPoint( position, 25.f, Rgba8::GREEN, durationSeconds );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Draw screen point at - x:%.2f, y:%.2f - for %.2f seconds", position.x, position.y, durationSeconds ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_screen_quad( NamedStrings* args )
{
	Vec2 defaultMin = Vec2::ZERO;
	Vec2 defaultMax = Vec2::UNIT * 50.f;
	float defaultDuration = 0.f;

	Vec2 min = args->GetValue( "min", defaultMin );
	Vec2 max = args->GetValue( "max", defaultMax );
	float duration = args->GetValue( "duration", defaultDuration );

	AABB2 bounds = AABB2( min, max );
	DebugAddScreenQuad( bounds, Rgba8::YELLOW, duration );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Draw screen quad min - x:%.2f, y:%.2f max - x:%.2f, y%.2f - for %.2f seconds", min.x, min.y, max.x, max.y, duration ) );
	}
}


//---------------------------------------------------------------------------------------------------------
static void debug_add_screen_text( NamedStrings* args )
{
	Vec2 defaultPosition = Vec2::ZERO;
	Vec2 defaultPivot = Vec2::ZERO;
	float defaultDuration = 10.f;
	std::string defaultText = "default text";

	Vec2 position = args->GetValue( "position", defaultPosition );
	Vec2 pivot = args->GetValue( "pivot", defaultPivot );
	float duration = args->GetValue( "duration", defaultDuration );
	std::string text = args->GetValue( "text", defaultText );

	Vec4 ratioOffset = Vec4( 0.f, 0.f, position.x, position.y );

	DebugAddScreenText( ratioOffset, pivot, 10.f, Rgba8::ORANGE, Rgba8::ORANGE, duration, text.c_str() );

	if( g_theConsole != nullptr )
	{
		g_theConsole->PrintString( Rgba8::WHITE, Stringf( "Draw screen text at - x:%.2f, y:%.2f with pivot - x:%.2f, y%.2f - for %.2f seconds", position.x, position.y, pivot.x, pivot.y, duration ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderSystemStartup( RenderContext* context )
{
	s_debugRenderSystem = new DebugRenderSystem();
	s_debugRenderSystem->SetRenderContext( context );

	if( g_theEventSystem != nullptr )
	{
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_render", debug_render );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_world_point", debug_add_world_point );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_world_wire_sphere", debug_add_world_wire_sphere );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_world_wire_bounds", debug_add_world_wire_bounds );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_world_billboard_text", debug_add_world_billboard_text );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_screen_point", debug_add_screen_point );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_screen_quad", debug_add_screen_quad );
		g_theEventSystem->SubscribeEventCallbackFunction( "debug_add_screen_text", debug_add_screen_text );
	}
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
		if( s_debugRenderWorldObjects[ objectIndex ] != nullptr )
		{
			s_debugRenderWorldObjects[ objectIndex ]->m_isMarkedForDestroy = true;
		}
	}

	for (int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex)
	{
		if( s_debugRenderScreenObjects[ objectIndex ] != nullptr )
		{
			s_debugRenderScreenObjects[ objectIndex ]->m_isMarkedForDestroy = true;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* cam )
{
	if( !s_debugRenderSystem->IsRenderingEnabled() ) 
	{
		ClearDebugRendering();
		return;
	}

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
	debugCamera->SetDepthStencilTarget( cam->GetDepthStencilTarget() );
	debugCamera->SetProjectionMatrix( cam->GetProjectionMatrix() );
	debugCamera->SetViewMatrix( cam->GetViewMatrix() );
	debugCamera->SetClearMode( CLEAR_NONE, Rgba8::BLACK );

	s_debugRenderSystem->SetCamera( debugCamera );

	context->BeginCamera( *debugCamera );
	for( int objectIndex = 0; objectIndex < s_debugRenderWorldObjects.size(); ++objectIndex )
	{	
		DebugRenderObject* object = s_debugRenderWorldObjects[ objectIndex ];

		if( object == nullptr ) continue;

		object->Draw();
		if( object->HasExpired() )
		{
			object->m_isMarkedForDestroy = true;
		}
	}

	context->EndCamera( *debugCamera );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{
	if( !s_debugRenderSystem->IsRenderingEnabled() )
	{
		ClearDebugRendering();
		return;
	}

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
	s_debugRenderSystem->SetScreenHeight( max.y );
	camera->SetProjectionOrthographic( max.y );
	camera->SetTransform( Transform() );
	camera->SetPosition( Vec3( max, 0.f ) * 0.5f );
	camera->SetClearMode( CLEAR_NONE, Rgba8::BLACK );

	s_debugRenderSystem->SetCamera( camera );

	context->BeginCamera( *camera );
	for( int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex )
	{
		DebugRenderObject* object = s_debugRenderScreenObjects[ objectIndex ];

		if( object == nullptr ) continue;

		object->Draw();
		if( object->HasExpired() )
		{
			object->m_isMarkedForDestroy = true;
		}
	}

	context->EndCamera( *camera );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	for( int objectIndex = 0; objectIndex < s_debugRenderWorldObjects.size(); ++objectIndex )
	{
		if( s_debugRenderWorldObjects[ objectIndex ] != nullptr && s_debugRenderWorldObjects[ objectIndex ]->m_isMarkedForDestroy )
		{
			delete s_debugRenderWorldObjects[ objectIndex ];
			s_debugRenderWorldObjects[ objectIndex ] = nullptr;
		}
	}

	for( int objectIndex = 0; objectIndex < s_debugRenderScreenObjects.size(); ++objectIndex )
	{
		if( s_debugRenderScreenObjects[ objectIndex ] != nullptr && s_debugRenderScreenObjects[ objectIndex ]->m_isMarkedForDestroy )
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
	object->m_isBillboarded = true;
	object->m_renderMode = mode;


	float halfSize = size * 0.5f;
	Vec3 min = Vec3( -halfSize, -halfSize, pos.z );
	Vec3 max = Vec3( halfSize, halfSize, pos.z );
	AABB2 box = AABB2( min.x, min.y, max.x, max.y );

	AppendVertsForAABB2D( object->m_objectVerticies, box, start_color );
	//object->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, size, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 pos, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, 0.2f, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldLine( Vec3 p0, Vec3 p1, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( p0 );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;

	AddCylinderToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, Vec3::ZERO, 0.05f, p1 - p0, 0.05f, start_color, 32 );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldLine( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldLine( start, end, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldArrow( Vec3 p0, Vec3 p1, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D(p0);
	object->m_durationTimer.SetSeconds(duration);
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;

	float coneSize = 0.1f;
	Vec3 displacement = p1 - p0;
	Vec3 coneStartPosition = displacement - displacement.GetNormalize() * ( coneSize * 1.5f );

	AddCylinderToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, Vec3::ZERO, coneSize * 0.5f, coneStartPosition, coneSize * 0.5f, start_color, 32 );
	AddConeToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, coneStartPosition, coneSize, displacement, start_color, 32 );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldArrow( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldArrow( start, end, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldCone( Vec3 const& startPos, float startRadius, Vec3 const& endPos, float endRadius, Rgba8 const& startColor, Rgba8 const& endColor, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( startPos );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = startColor;
	object->m_endColor = endColor;
	object->m_renderMode = mode;
	object->m_isWireMesh = true;

	AddCylinderToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, Vec3::ZERO, startRadius, endPos - startPos, endRadius, startColor, 32 );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldCone( Vec3 const& startPos, Vec3 const& endPos, float endRadius, Rgba8 const& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldCone( startPos, 0.f, endPos, endRadius, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldQuad( Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, AABB2 uvs, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( p0 );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;

	AppendVertsForQuad3D( object->m_objectVerticies, Vec3::ZERO, p1 - p0, p2 - p0, p3 - p0, start_color, uvs.mins, uvs.maxes );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldPoly( Vec3 const* counterClockwisePolyPoints, Rgba8 const& start_color, Rgba8 const& end_color, float duration, eDebugRenderMode mode )
{
	UNUSED(counterClockwisePolyPoints);
	UNUSED(start_color);
	UNUSED(end_color);
	UNUSED(duration);
	UNUSED(mode);
// 	DebugRenderObject* object = new DebugRenderObject();
// 
// 	object->m_transformMatrix = Mat44::IDENTITY;
// 	object->m_durationTimer.SetSeconds( duration );
// 	object->m_startColor = start_color;
// 	object->m_endColor = end_color;
// 	object->m_renderMode = mode;
// 
// 	for (int i = 0; i < simplex.size(); ++i)
// 	{
// 		Vec3 edgeStart = Vec3( simplex[i], 0.f );
// 		Vec3 edgeEnd;
// 		if (i >= simplex.size() - 1 )
// 		{
// 			edgeEnd = Vec3(simplex[0], 0.f);
// 		}
// 		else
// 		{
// 			edgeEnd = Vec3(simplex[i + 1], 0.f);
// 		}
// 		DebugAddWorldLine( edgeStart, edgeEnd, Rgba8::ORANGE, debugduration, DEBUG_RENDER_ALWAYS );
// 	}
// 
// 	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix = bounds.m_transformMatrix;
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;
	object->m_isWireMesh = true;

	AABB3 localBounds = AABB3( -bounds.m_halfDimensions, bounds.m_halfDimensions );

	AddBoxToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, localBounds, start_color );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldWireBounds( bounds, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( AABB3 bounds, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( bounds.mins );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = color;
	object->m_endColor = color;
	object->m_renderMode = mode;
	object->m_isWireMesh = true;

	AABB3 localBounds = AABB3( Vec3::ZERO, bounds.GetDimensions() );

	AddBoxToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, localBounds, color );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode )
{
	DebugRenderObject* object = new DebugRenderObject();

	object->m_transformMatrix.SetTranslation3D( pos );
	object->m_durationTimer.SetSeconds( duration );
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;
	object->m_isWireMesh = true;

	AddUVSphereToIndexedVertexArray( object->m_objectVerticies, object->m_objectIndicies, Vec3::ZERO, radius, 16, 32, start_color );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldWireSphere( pos, radius, color, color, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldBasis( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float duration, eDebugRenderMode mode )
{
	Vec4 startTintZeroToOne = Vec4( static_cast<float>( start_tint.r ), static_cast<float>( start_tint.g ), static_cast<float>( start_tint.b ), static_cast<float>( start_tint.a ) ) / 255.f;
	Vec4 endTintZeroToOne = Vec4( static_cast<float>( end_tint.r ), static_cast<float>( end_tint.g ), static_cast<float>( end_tint.b ), static_cast<float>( end_tint.a ) ) / 255.f;

	Vec4 iBasisColor = Vec4( 1.f, 0.f, 0.f, 1.f );
	Vec4 jBasisColor = Vec4( 0.f, 1.f, 0.f, 1.f );
	Vec4 kBasisColor = Vec4( 0.f, 0.f, 1.f, 1.f );

	Vec4 iBasisStartColor = iBasisColor * startTintZeroToOne;
	Vec4 iBasisEndColor = iBasisColor * endTintZeroToOne;
	Rgba8 iBasisStartRGBA8 = Rgba8( iBasisStartColor );
	Rgba8 iBasisEndRGBA8 = Rgba8( iBasisEndColor );
	
	Vec4 jBasisStartColor = jBasisColor * startTintZeroToOne;
	Vec4 jBasisEndColor = jBasisColor * endTintZeroToOne;
	Rgba8 jBasisStartRGBA8 = Rgba8( jBasisStartColor );
	Rgba8 jBasisEndRGBA8 = Rgba8( jBasisEndColor );	

	Vec4 kBasisStartColor = kBasisColor * startTintZeroToOne;
	Vec4 kBasisEndColor = kBasisColor * endTintZeroToOne;
	Rgba8 kBasisStartRGBA8 = Rgba8( kBasisStartColor );
	Rgba8 kBasisEndRGBA8 = Rgba8( kBasisEndColor );

	Vec3 translation = basis.GetTranslation3D();

	DebugAddWorldArrow( translation, basis.GetIBasis3D() + translation, iBasisStartRGBA8, iBasisEndRGBA8, duration, mode );
	DebugAddWorldArrow( translation, basis.GetJBasis3D() + translation, jBasisStartRGBA8, jBasisEndRGBA8, duration, mode );
	DebugAddWorldArrow( translation, basis.GetKBasis3D() + translation, kBasisStartRGBA8, kBasisEndRGBA8, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldBasis(Mat44 basis, float duration, eDebugRenderMode mode )
{
	DebugAddWorldBasis( basis, Rgba8::WHITE, Rgba8::WHITE, duration, mode );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode, char const* text )
{
	float textSize = 0.2f;

	if( s_debugRenderFont == nullptr )
	{
		RenderContext* context = s_debugRenderSystem->GetRenderContext();
		s_debugRenderFont = context->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	}

	Vec2 textDimensions = s_debugRenderFont->GetDimensionsForText2D( textSize, text );
	Vec2 textStartPos = -( textDimensions * pivot );

	DebugRenderObject* object = new DebugRenderObject();

	object->m_durationTimer.SetSeconds( duration );
	object->m_transformMatrix = basis;
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;
	object->m_texture = s_debugRenderFont->GetTexture();

	s_debugRenderFont->AddVertsForText2D( object->m_objectVerticies, textStartPos, textSize, text, start_color );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldTextf( Mat44 basis, Vec2 pivot, Rgba8 color, float duration, eDebugRenderMode mode, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddWorldText( basis, pivot, color, color, duration, mode, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldTextf( Mat44 basis, Vec2 pivot, Rgba8 color, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[textLength];
	va_list variableArgumentList;

	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, textLength, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[textLength - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddWorldText( basis, pivot, color, color, 0.f, DEBUG_RENDER_USE_DEPTH, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldBillboardText( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode, char const* text )
{
	float textSize = 0.2f;

	if( s_debugRenderFont == nullptr )
	{
		RenderContext* context = s_debugRenderSystem->GetRenderContext();
		s_debugRenderFont = context->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	}

	Vec2 textDimensions = s_debugRenderFont->GetDimensionsForText2D( textSize, text );
	Vec2 textStartPos = -( textDimensions * pivot );

	DebugRenderObject* object = new DebugRenderObject();

	object->m_durationTimer.SetSeconds( duration );
	object->m_transformMatrix.SetTranslation3D( origin );
	object->m_startColor = start_color;
	object->m_endColor = end_color;
	object->m_renderMode = mode;
	object->m_texture = s_debugRenderFont->GetTexture();
	object->m_isBillboarded = true;

	s_debugRenderFont->AddVertsForText2D( object->m_objectVerticies, textStartPos, textSize, text, start_color );

	AppendDebugRenderObjectToVector( s_debugRenderWorldObjects, object );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldBillboardTextf( Vec3 origin, Vec2 pivot, Rgba8 color, float duration, eDebugRenderMode mode, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddWorldBillboardText( origin, pivot, color, color, duration, mode, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddWorldBillboardTextf( Vec3 origin, Vec2 pivot, Rgba8 color, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddWorldBillboardText( origin, pivot, color, color, 0.f, DEBUG_RENDER_USE_DEPTH, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height )
{
	s_debugRenderSystem->SetScreenHeight( height );
}


//---------------------------------------------------------------------------------------------------------
AABB2 DebugGetScreenBounds()
{
	float aspectRatio = 16.f / 9.f;
	if( s_debugRenderSystem->GetCamera() != nullptr )
	{
		aspectRatio = s_debugRenderSystem->GetCamera()->GetAspectRatio();
	}

	float screenHeight = s_debugRenderSystem->GetScreenHeight();
	return AABB2( 0.f, 0.f, aspectRatio * screenHeight, screenHeight );
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
	//screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration )
{
	DebugAddScreenPoint( pos, size, color, color, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, Rgba8 color )
{
	DebugAddScreenPoint( pos, 20.f, color, color, 0.f );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenLine( Vec2 p0, Vec2 p1, Rgba8 start_color, Rgba8 end_color, float duration )
{
	DebugRenderObject* screenObject = new DebugRenderObject();
	
	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( p0 );
	screenObject->m_startColor = start_color;
	screenObject->m_endColor = end_color;

	AppendVertsForLineBetweenPoints( screenObject->m_objectVerticies, Vec2::ZERO, p1 - p0, start_color, 10.f );
	//screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenLine(Vec2 p0, Vec2 p1, Rgba8 color, float duration )
{
	DebugAddScreenLine( p0,  p1, color, color, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenArrow( Vec2 p0, Vec2 p1, Rgba8 start_color, Rgba8 end_color, float duration )
{
	DebugRenderObject* screenObject = new DebugRenderObject();

	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( p0 );
	screenObject->m_startColor = start_color;
	screenObject->m_endColor = end_color;

	AppendVertsForArrowBetweenPoints( screenObject->m_objectVerticies, Vec2::ZERO, p1 - p0, start_color, 10.f );
	//screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenArrow( Vec2 p0, Vec2 p1, Rgba8 color, float duration )
{
	DebugAddScreenArrow( p0, p1, color, color, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenQuad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration )
{
	DebugRenderObject* screenObject = new DebugRenderObject();

	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( bounds.mins );
	screenObject->m_startColor = start_color;
	screenObject->m_endColor = end_color;

	AABB2 localBounds = AABB2( Vec2::ZERO, bounds.maxes - bounds.mins );

	AppendVertsForAABB2D( screenObject->m_objectVerticies, localBounds, start_color );
	//screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenQuad( AABB2 bounds, Rgba8 color, float duration )
{
	DebugAddScreenQuad( bounds, color, color, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 start_tint, Rgba8 end_tint, float duration )
{
	DebugRenderObject* screenObject = new DebugRenderObject();

	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( bounds.mins );
	screenObject->m_startColor = start_tint;
	screenObject->m_endColor = end_tint;
	screenObject->m_texture = tex;

	AABB2 localBounds = AABB2( Vec2::ZERO, bounds.maxes - bounds.mins );

	AppendVertsForAABB2D( screenObject->m_objectVerticies, localBounds, start_tint, uvs.mins, uvs.maxes );
	//screenObject->m_objectIndicies = { 0, 1, 2, 3, 4, 5 };

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 tint, float duration )
{
	DebugAddScreenTexturedQuad( bounds, tex, uvs, tint, tint, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, Rgba8 tint, float duration )
{
	DebugAddScreenTexturedQuad( bounds, tex, AABB2(), tint, tint, duration );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenText( Vec4 ratioOffset, Vec2 pivot, float size, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	if (s_debugRenderFont == nullptr)
	{
		RenderContext* context = s_debugRenderSystem->GetRenderContext();
		s_debugRenderFont = context->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	}

	AABB2 screenBounds = DebugGetScreenBounds();
	Vec2 textPos = screenBounds.GetDimensions() * Vec2( ratioOffset.x, ratioOffset.y );
	Vec2 textPosOffset = Vec2( ratioOffset.z, ratioOffset.w );
	Vec2 textStartPos = textPos + textPosOffset;

	Vec2 textDimensions = s_debugRenderFont->GetDimensionsForText2D( size, text );
	textStartPos -= textDimensions * pivot;

	DebugRenderObject* screenObject = new DebugRenderObject();

	screenObject->m_durationTimer.SetSeconds( duration );
	screenObject->m_transformMatrix.SetTranslation2D( textStartPos );
	screenObject->m_startColor = start_color;
	screenObject->m_endColor = end_color;
	screenObject->m_texture = s_debugRenderFont->GetTexture();

	s_debugRenderFont->AddVertsForText2D( screenObject->m_objectVerticies, Vec2::ZERO, size, text, start_color );

	AppendDebugRenderObjectToVector( s_debugRenderScreenObjects, screenObject );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTextf( Vec4 pos, Vec2 pivot, float size, Rgba8 start_color, Rgba8 end_color, float duration, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddScreenText( pos, pivot, size, start_color, end_color, duration, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTextf( Vec4 pos, Vec2 pivot, float size, Rgba8 color, float duration, char const* format, ... )
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddScreenText( pos, pivot, size, color, color, duration, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTextf(Vec4 pos, Vec2 pivot, float size, Rgba8 color, char const* format, ...)
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddScreenText( pos, pivot, size, color, color, 0.f, textLiteral );
}


//---------------------------------------------------------------------------------------------------------
void DebugAddScreenTextf(Vec4 pos, Vec2 pivot, Rgba8 color, char const* format, ...)
{
	const int textLength = 2048;

	char textLiteral[ textLength ];
	va_list variableArgumentList;

	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, textLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[ textLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	DebugAddScreenText( pos, pivot, 10.f, color, color, 0.f, textLiteral );
}

