#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include <vector>


class Sampler;
class BitmapFont;
class Window;
class SwapChain;
class Shader;
class RenderBuffer;
class VertexBuffer;
class IndexBuffer;
class Transform;
class GPUMesh;
class Clock;
struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11DeviceContext;
struct ID3D11BlendState;
struct IDXGIDebug;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11InputLayout;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	DISABLED,
};

enum BufferSlot
{
	UBO_FRAME_SLOT			= 0,
	UBO_CAMERA_SLOT			= 1,
	UBO_MODEL_MATRIX_SLOT	= 2,
	UBO_LIGHT_SLOT			= 3,
	UBO_MATERIAL_SLOT		= 5,
};

enum CompareFunc
{
	COMPARE_FUNC_NEVER,           // D3D11_COMPARISON_NEVER
	COMPARE_FUNC_ALWAYS,          // D3D11_COMPARISON_ALWAYS
	COMPARE_FUNC_LEQUAL,          // D3D11_COMPARISON_LESS_EQUAL
	COMPARE_FUNC_GEQUAL,          // D3D11_COMPARISON_GREATER_EQUAL
	COMPARE_FUNC_EQUAL,
};

enum CullMode
{
	CULL_MODE_NONE,		//D3D11_CULL_NONE
	CULL_MODE_BACK,		//D3D11_CULL_BACK
	CULL_MODE_FRONT,	//D3D11_CULL_FRONT
};

enum FillMode
{
	FILL_MODE_SOLID,		//D3D11_FILL_SOLID
	FILL_MODE_WIREFRAME,	//D3D11_FILL_WIREFRAME
};

struct frame_data_t
{
	float system_time;
	float system_delta_time;
	float gamma;
	float inverseGamma;

	Vec4 fogNearColor;
	Vec4 fogFarColor;
	float fogNear;
	float fogFar;
	float padding[2];
};

struct camera_data_t
{				  
	Mat44 projection;
	Mat44 view;
	Mat44 model;
	Vec3 position;
	float padding;
};

struct model_data_t
{
	Mat44 model;
	Vec4 tint;
	float specularFactor;
	float specularPower;
	Vec2 padding;
};

struct light_data_t
{
	Vec4 ambient;
	Light lights[MAX_LIGHTS];
};


class RenderContext
{
public:
	void StartUp( Window* theWindow );
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void UpdateFrameUBO();
	float GetGamma() const			{ return m_gamma; }
	void UpdateGamma( float gamma );
	void DisableFog();
	void EnableFog( float fogNear, float fogFar, Rgba8 const& fogNearColor, Rgba8 const& fogFarColor );
	
	void SetGameClock( Clock* clock );
	void SetBlendMode( BlendMode blendMode );
	void SetDepthTest( CompareFunc compareFunc, bool writeDepthOnPass );
	void ClearScreen( const Rgba8& clearColor );
	void ClearDepth( Texture* depthStencilTexture, float depth );
	void BeginCamera( Camera& camera );
	void EndCamera( const Camera& camera );

	bool DoesDepthStateMatch( CompareFunc compareFunc, bool writeDepthOnPass );
	bool IsDrawing() const { return m_isDrawing; }
	Texture* GetBackBuffer() const;
	
	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawIndexed( int numIndicies, int indexOffset = 0, int vertexOffset = 0 );
	void DrawIndexedVertexArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies );
	void DrawVertexArray( int numVerticies, const Vertex_PCU* verticies );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray );
	void DrawMesh( GPUMesh* mesh );
	void UpdateCurrentLayout( buffer_attribute_t const* newLayout );

	void		SetModelUBO( Mat44 const& modelMatrix, Rgba8 const& modelTint = Rgba8::WHITE, float specularFactor = 1.f, float specularPower = 1.f );
	void		SetCullMode( CullMode cullMode );
	void		SetFillMode( FillMode fillMode );
	void		SetFrontFaceWindOrder( bool isCounterClockwise );
	
	void		BindShader( Shader* shader );
	void		BindShader( const char* filepath );
	void		BindVertexInput( VertexBuffer* vbo );
	void		BindIndexBuffer( IndexBuffer* ibo );
	void		BindUniformBuffer( unsigned int slot, RenderBuffer* ubo );
	void		BindSampler( Sampler* sampler );
	void		BindTexture( const Texture* constTexture );
	void		BindNormalTexture( const Texture* constTexture );
	void		BindMaterialTexture( unsigned int slot, const Texture* constTexture );

	void		ReloadShaders();
	Texture*	CreateOrGetTextureFromFile( const char* imageFilePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* imageFilePath );
	Shader*		GetOrCreateShader( char const* filename );
	Shader*		CreateShaderFromSourceCode( char const* sourceCode );
	Texture*	CreateTextureFromColor( Rgba8 const& color );
	Texture*	CreateDepthStencilBuffer( IntVec2 const& imageDimensions );

	//Light Methods
	void UpdateLightUBO();
	void SetAmbientColor( Rgba8 const& ambientColor );
	void SetAmbientIntensity( float ambientIntensity );
	void SetAmbientLight( Rgba8 const& ambientColor, float ambientIntensity );
	void EnableLight( unsigned int index, Light const& lightInfo );
	void DisableLight( unsigned int index );
	void DisableAllLights();

	//Material Methods
	void SetMaterialUBO( void const* data, unsigned int dataSize );

	//Clean Up Methods
	void ReleaseLoadedAssets();
	void ReleaseBlendStates();

private:
	void CreateBlendStates();
	void CreateRasterState();
	bool CreateTextureFromFile( const char* imageFilePath );
	bool CreateBitmapFontFromFile( const char* fontFilePath );
	Shader* CreateShaderFromFilePath( char const* filename );

	void ReportLiveObjects();
	void CreateDebugModule();
	void DestroyDebugModule();

private:
	Clock* m_gameClock = nullptr;

	float m_gamma = 2.f;
	float m_inverseGamma = 1 / m_gamma;
	Rgba8 m_fogNearColor = Rgba8::WHITE;
	Rgba8 m_fogFarColor = Rgba8::WHITE;
	float m_fogNear = -1.f;
	float m_fogFar = -1.5f;

	Vec4 m_ambientLight = Vec4( 1.f, 1.f, 1.f, 1.f );
	Light m_lights[MAX_LIGHTS];

	bool m_isDrawing = false;
	ID3D11Buffer* m_lastBoundVBOHandle = nullptr;

	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;
	std::vector<Shader*>		m_loadedShaders;

public:
	void*						m_debugModule				= nullptr;
	IDXGIDebug*					m_debug						= nullptr;

	ID3D11Device*				m_device					= nullptr;
	ID3D11DeviceContext*		m_context					= nullptr;
	ID3D11DepthStencilState*	m_currentDepthStencilState	= nullptr;
	ID3D11RasterizerState*		m_rasterState				= nullptr;
	SwapChain*					m_swapchain					= nullptr;
	Shader*						m_currentShader				= nullptr;
	Shader*						m_defaultShader				= nullptr;
	Shader*						m_errorShader				= nullptr;
	Texture*					m_textueDefaultColor		= nullptr;
	Texture*					m_textureDefaultNormalColor	= nullptr;
	Sampler*					m_samplerPoint				= nullptr;
	Sampler*					m_samplerLinear				= nullptr;
	VertexBuffer*				m_immediateVBO				= nullptr;
	IndexBuffer*				m_immediateIBO				= nullptr;
	RenderBuffer*				m_frameUBO					= nullptr;
	RenderBuffer*				m_modelUBO					= nullptr;
	RenderBuffer*				m_lightUBO					= nullptr;
	RenderBuffer*				m_materialUBO				= nullptr;
	Texture*					m_defaultDepthStencil		= nullptr;
	buffer_attribute_t const*	m_currentVertexLayout		= nullptr;
	Shader*						m_lastBoundShader			= nullptr;

	ID3D11BlendState* m_alphaBlendStateHandle		= nullptr;
	ID3D11BlendState* m_additiveBlendStateHandle	= nullptr;
	ID3D11BlendState* m_disabledBlendStateHandle	= nullptr;
};