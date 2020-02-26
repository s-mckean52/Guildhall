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
class IndexBuffer;
class Transform;
class GPUMesh;
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
	UBO_FRAME_SLOT			= 0,
	UBO_CAMERA_SLOT			= 1,
	UBO_MODEL_MATRIX_SLOT	= 2,
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

struct model_matrix_t
{
	Mat44 model;
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
	void BeginCamera( Camera& camera );
	void EndCamera( const Camera& camera );

	bool IsDrawing() const { return m_isDrawing; }
	
	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawIndexed( int numIndicies, int indexOffset = 0, int vertexOffset = 0 );
	void DrawVertexArray( int numVerticies, const Vertex_PCU* verticies );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray );
	void DrawMesh( GPUMesh* mesh );

	void		SetModelMatrix( Mat44 const& modelMatrix );
	
	void		BindShader( Shader* shader );
	void		BindShader( const char* filepath );
	void		BindVertexInput( VertexBuffer* vbo );
	void		BindIndexBuffer( IndexBuffer* ibo );
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
	RenderBuffer*			m_modelUBO				= nullptr;

	ID3D11BlendState* m_alphaBlendStateHandle		= nullptr;
	ID3D11BlendState* m_additiveBlendStateHandle	= nullptr;
	ID3D11BlendState* m_disabledBlendStateHandle	= nullptr;
};