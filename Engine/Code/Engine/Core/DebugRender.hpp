#pragma once
#include "Engine/Core/Rgba8.hpp"

struct Vec3;
struct Vec2;
struct Vec4;
struct Mat44;
struct Texture;
struct AABB2;
struct AABB3;
struct OBB3;
class Camera;
class GPUMesh;
class RenderContext;


/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// Only effects "world" mode stuff; 
enum eDebugRenderMode
{
	DEBUG_RENDER_ALWAYS,          // what is rendered always shows up
	DEBUG_RENDER_USE_DEPTH,       // respect the depth buffer
	DEBUG_RENDER_XRAY,            // renders twice - once darker when it should be hidden, and once more saturated when it should appear
};

/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/
//------------------------------------------------------------------------
//  System
//------------------------------------------------------------------------
// setup
void DebugRenderSystemStartup( RenderContext* context );    // may be used to allocate resources to the system
void DebugRenderSystemShutdown();   // cleans up the system to prevent leaks.

// control
void EnableDebugRendering();
void DisableDebugRendering();
void ClearDebugRendering();

// output
void DebugRenderBeginFrame();                   // Does nothing, here for completeness.
void DebugRenderWorldToCamera( Camera* cam );   // Draws all world objects to this camera 
void DebugRenderScreenTo( Texture* output );    // Draws all screen objects onto this texture (screen coordinate system is up to you.  I like a 1080p default)
void DebugRenderEndFrame();                     // Clean up dead objects

//------------------------------------------------------------------------
// World Rendering
//------------------------------------------------------------------------
// points
void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( Vec3 pos, float size, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( Vec3 pos, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// lines
void DebugAddWorldLine( Vec3 p0, Vec3 p1, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode );
void DebugAddWorldLine( Vec3 start, Vec3 end, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// line strip [extra]
void DebugAddWorldLineStrip( unsigned int count, Vec3 const* positions,
	Rgba8 start_p0_color, Rgba8 start_pf_color,    // color of first/end point at the beginning of duration
	Rgba8 end_p0_color, Rgba8 end_pf_color,        // color of first/end point at the end of duration
	float duration,
	eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// arrows
void DebugAddWorldArrow( Vec3 p0, Vec3 p1, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode );
void DebugAddWorldArrow( Vec3 start, Vec3 end, Rgba8 color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// Quads
void DebugAddWorldQuad( Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, AABB2 uvs, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// bounds
void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireBounds( OBB3 bounds, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireBounds( AABB3 bounds, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireSphere( Vec3 pos, float radius, Rgba8 color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// basis
void DebugAddWorldBasis( Mat44 basis, Rgba8 start_tint, Rgba8 end_tint, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldBasis( Mat44 basis, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// text
// non-billboarded will be oriented in the world based on the passed in basis matrix 
void DebugAddWorldText( Mat44 basis, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode, char const* text );
void DebugAddWorldTextf( Mat44 basis, Vec2 pivot, Rgba8 color, float duration, eDebugRenderMode mode, char const* format, ... );
void DebugAddWorldTextf( Mat44 basis, Vec2 pivot, Rgba8 color, char const* format, ... ); // assume DEBUG_RENDER_USE_DEPTH

// this text will always orient itself to the current camera
void DebugAddWorldBillboardText( Vec3 origin, Vec2 pivot, Rgba8 start_color, Rgba8 end_color, float duration, eDebugRenderMode mode, char const* text );
void DebugAddWorldBillboardTextf( Vec3 origin, Vec2 pivot, Rgba8 color, float duration, eDebugRenderMode mode, char const* format, ... );
void DebugAddWorldBillboardTextf( Vec3 origin, Vec2 pivot, Rgba8 color, char const* format, ... );

// grid [extra] 
void DebugAddWorldGrid( Vec3 origin,
	Vec3 i, float iMin, float iMax, float iMinorSegment, float iMajorSegment, Rgba8 iMinorColor, Rgba8 iMajorColor, Rgba8 iOriginColor,
	Vec3 j, float jMin, float jMax, float jMinorSegment, float jMajorSegment, Rgba8 jMinorcolor, Rgba8 jMajorColor, Rgba8 jOriginColor,
	Rgba8 axisColor );
void DebugAddWorldXYGrid();
void DebugAddWorldXZGrid();
void DebugAddWorldYZGrid();

// mesh [extra]
void DebugAddWireMeshToWorld( Mat44 transform, GPUMesh* mesh, Rgba8 start_tint, Rgba8 end_tint, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWireMeshToWorld( Mat44 transform, GPUMesh* mesh, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );


//------------------------------------------------------------------------
// Screen Rendering
//------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height ); // default to 1080.0f when system starts up.  Meaning (0,0) should always be bottom left, (aspect * height, height) is top right
AABB2 DebugGetScreenBounds();                    // useful if you want to align to top right for something

// points
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration );
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration = 0.0f );
void DebugAddScreenPoint( Vec2 pos, Rgba8 color ); // assumed size;

// lines
void DebugAddScreenLine( Vec2 p0, Vec2 p1, Rgba8 start_color, Rgba8 end_color, float duration );
void DebugAddScreenLine( Vec2 p0, Vec2 p1, Rgba8 color, float duration = 0.0f );

// arrows
void DebugAddScreenArrow( Vec2 p0, Vec2 p1, Rgba8 start_color, Rgba8 end_color,	float duration );
void DebugAddScreenArrow( Vec2 p0, Vec2 p1, Rgba8 color, float duration = 0.0f );

// quad
void DebugAddScreenQuad( AABB2 bounds, Rgba8 start_color, Rgba8 end_color, float duration );
void DebugAddScreenQuad( AABB2 bounds, Rgba8 color, float duration = 0.0f );

// texture
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 start_tint, Rgba8 end_tint, float duration = 0.0f );
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, AABB2 uvs, Rgba8 tint, float duration = 0.0f );
void DebugAddScreenTexturedQuad( AABB2 bounds, Texture* tex, Rgba8 tint = Rgba8::WHITE, float duration = 0.0f ); // assume UVs are full texture

// text
void DebugAddScreenText( Vec4 ratioOffset, Vec2 pivot, float size, Rgba8 start_color, Rgba8 end_color, float duration, char const* text );
void DebugAddScreenTextf( Vec4 ratioOffset, Vec2 pivot, float size, Rgba8 start_color, Rgba8 end_color, float duration, char const* format, ... );
void DebugAddScreenTextf( Vec4 ratioOffset, Vec2 pivot, float size, Rgba8 color, float duration, char const* format, ... );
void DebugAddScreenTextf( Vec4 ratioOffset, Vec2 pivot, float size, Rgba8 color, char const* format, ... );
void DebugAddScreenTextf( Vec4 ratioOffset, Vec2 pivot, Rgba8 color, char const* format, ... );

// screen basis [extra]
void DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 start_tint, Rgba8 end_tint, float duration );
void DebugAddScreenBasis( Vec2 screen_origin_location, Mat44 basis_to_render, Rgba8 tint = Rgba8::WHITE, float duration = 0.0f );

// message log system [extra]
void DebugAddMessage( float duration, Rgba8 color, char const* format, ...);
void DebugAddMessage( Rgba8 color, char const* format, ...);
