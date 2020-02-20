#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>


class Sampler;
class BitmapFont;
class Window;
class SwapChain;
class Shader;
class RenderBuffer;
class VertexBuffer;	
struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11DeviceContext;
struct ID3D11BlendState;
struct IDXGIDebug;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	DISABLED,
};

enum BufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
};

struct frame_data_t
{
	float system_time;
	float system_delta_time;

	float padding[2];
};

struct camera_data_t
{				  
	Mat44 projection;
	Mat44 view;
};

class RenderContext
{
public:
	void StartUp( Window* theWindow );
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void UpdateFrameTime( float deltaSeconds );
	
	void SetBlendMode( BlendMode blendMode );
	void ClearScreen( const Rgba8& clearColor );
	void BeginCamera( const Camera& camera );
	void UpdateCameraData( Camera const& camera );
	void EndCamera( const Camera& camera );

	bool IsDrawing() const { return m_isDrawing; }
	
	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawVertexArray( int numVerticies, const Vertex_PCU* verticies );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray );
	
	void		BindShader( Shader* shader );
	void		BindShader( const char* filepath );
	void		BindVertexInput( VertexBuffer* vbo );
	void		BindUniformBuffer( unsigned int slot, RenderBuffer* ubo );
	void		BindSampler( Sampler* sampler );

	void		BindTexture( const Texture* constTexture );
	Texture*	CreateOrGetTextureFromFile( const char* imageFilePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* imageFilePath );
	Shader*		GetOrCreateShader( char const* filename );
	Shader*		CreateShaderFromSourceCode( char const* sourceCode);
	Texture*	CreateTextureFromColor( Rgba8 const& color );

	void ReleaseLoadedAssets();
	void ReleaseBlendStates();

private:
	void CreateBlendStates();
	bool CreateTextureFromFile( const char* imageFilePath );
	bool CreateBitmapFontFromFile( const char* fontFilePath );

	void ReportLiveObjects();
	void CreateDebugModule();
	void DestroyDebugModule();

private:
	bool m_isDrawing = false;
	ID3D11Buffer* m_lastBoundVBOHandle = nullptr;

	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;
	std::vector<Shader*>		m_loadedShaders;

public:
	void*					m_debugModule			= nullptr;
	IDXGIDebug*				m_debug					= nullptr;

	ID3D11Device*			m_device				= nullptr;
	ID3D11DeviceContext*	m_context				= nullptr;
	SwapChain*				m_swapchain				= nullptr;
	Shader*					m_currentShader			= nullptr;
	Shader*					m_defaultShader			= nullptr;
	Shader*					m_errorShader			= nullptr;
	Texture*				m_textueDefaultColor	= nullptr;
	Sampler*				m_samplerDefault		= nullptr;
	VertexBuffer*			m_immediateVBO			= nullptr;
	RenderBuffer*			m_frameUBO				= nullptr;

	ID3D11BlendState* m_alphaBlendStateHandle		= nullptr;
	ID3D11BlendState* m_additiveBlendStateHandle	= nullptr;
	ID3D11BlendState* m_disabledBlendStateHandle	= nullptr;
};