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


// void DebugRenderObject::AddVertsAndIndiciesToArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies )
// {
// 	m_hasExpired = m_durationTimer.CheckAndDecrement();
// 
// 	Rgba8 newColor = Rgba8Lerp( m_startColor, m_endColor, m_durationTimer.GetFractionComplete() );
// 
// 	unsigned int startIndexOffset = static_cast<unsigned int>( verticies.size() );
// 	for( int objectVertIndex = 0; objectVertIndex < m_objectVerticies.size(); ++objectVertIndex )
// 	{
// 		Vertex_PCU newVert = m_objectVerticies[ objectVertIndex ];
// 		newVert.m_color = newColor;
// 		newVert.m_position = m_transformMatrix.TransformPosition3D( newVert.m_position );
// 		verticies.push_back( newVert );
// 	}
// 
// 	for( int objectIndiciesIndex = 0; objectIndiciesIndex < m_objectIndicies.size(); ++objectIndiciesIndex )
// 	{
// 		indicies.push_back( m_objectIndicies[ objectIndiciesIndex ] + startIndexOffset );
// 	}
// }


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
		context->SetCullMode( CULL_MODE_NONE );
		context->SetFillMode( FILL_MODE_SOLID );
	}

 	context->SetModelMatrix( m_transformMatrix );
 	context->BindTexture( m_texture );
 	context->BindShader( (Shader*)nullptr );

	SetRenderMode();

	Rgba8 newColor = Rgba8Lerp( m_startColor, m_endColor, m_durationTimer.GetFractionComplete() );
	SetAllVertsColor( newColor );

	context->DrawIndexedVertexArray( m_objectVerticies, m_objectIndicies );
}


void DebugRenderObject::SetAllVertsColor( const Rgba8& newColor )
{
	for (int objectVertIndex = 0; objectVertIndex < m_objectVerticies.size(); ++objectVertIndex)
	{
		m_objectVerticies[ objectVertIndex ].m_color = newColor;
	}
}


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
void DebugRenderSystemStartup( RenderContext* context )
{
	s_debugRenderSystem = new DebugRenderSystem();
	s_debugRenderSystem->SetRenderContext( context );
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
	debugCamera->SetDepthStencilTarget( cam->GetDepthStencilTarget() );
	debugCamera->SetProjectionMatrix( cam->GetProjectionMatrix() );
	debugCamera->SetViewMatrix( cam->GetViewMatrix() );
	debugCamera->SetClearMode( CLEAR_NONE, Rgba8::BLACK );

	s_debugRenderSystem->SetCamera( debugCamera );

	context->BeginCamera( *debugCamera );
	//context->SetCullMode( CULL_MODE_BACK );
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
	DebugAddWorldPoint( pos, 0.5f, color, color, duration, mode );
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


	DebugAddWorldArrow( Vec3::ZERO, basis.GetIBasis3D(), iBasisStartRGBA8, iBasisEndRGBA8, duration, mode );
	DebugAddWorldArrow( Vec3::ZERO, basis.GetJBasis3D(), jBasisStartRGBA8, jBasisEndRGBA8, duration, mode );
	DebugAddWorldArrow( Vec3::ZERO, basis.GetKBasis3D(), kBasisStartRGBA8, kBasisEndRGBA8, duration, mode );
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

	if (s_debugRenderFont == nullptr)
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

	AppendDebugRenderObjectToVector(s_debugRenderScreenObjects, screenObject);
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
void DebugAddScreenText( Vec4 pos, Vec2 pivot, float size, Rgba8 start_color, Rgba8 end_color, float duration, char const* text )
{
	if (s_debugRenderFont == nullptr)
	{
		RenderContext* context = s_debugRenderSystem->GetRenderContext();
		s_debugRenderFont = context->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	}

	Vec2 textPos = Vec2( pos.x, pos.y );
	Vec2 textPosOffset = Vec2( pos.z, pos.w );
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

