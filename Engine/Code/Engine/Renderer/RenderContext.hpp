#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <vector>


class BitmapFont;
class Window;
class SwapChain;
class Shader;
class RenderBuffer;
class VertexBuffer;
struct ID3D11Device;
struct ID3D11DeviceContext;


enum class BlendMode
{
	ALPHA,
	ADDITIVE,
};

class RenderContext
{
public:
	void StartUp( Window* theWindow );
	void BeginFrame();
	void EndFrame();
	void ShutDown();
	
	void SetBlendMode( BlendMode blendMode );
	void ClearScreen( const Rgba8& clearColor );
	void BeginCamera( const Camera& camera );
	void EndCamera(const Camera& camera);
	
	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawVertexArray( int numVerticies, const Vertex_PCU* verticies );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray );
	
	void		BindShader( Shader* shader );
	void		BindVertexInput( VertexBuffer* vbo );

	void		BindTexture( const Texture* texture );
	Texture*	CreateOrGetTextureFromFile( const char* imageFilePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* imageFilePath );

private:
	void CreateTextureFromFile( const char* imageFilePath );
	void CreateBitmapFontFromFile( const char* fontFilePath );

private:
	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;

public:
	ID3D11Device*			m_device		= nullptr;
	ID3D11DeviceContext*	m_context		= nullptr;
	SwapChain*				m_swapchain		= nullptr;
	Shader*					m_currentShader = nullptr;
	Shader*					m_defaultShader = nullptr;
	VertexBuffer*			m_immediateVBO	= nullptr;
};