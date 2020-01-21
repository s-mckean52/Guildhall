#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <vector>

class BitmapFont;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
};

class RenderContext
{
private:
	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;

public:
	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();
	
	void SetBlendMode( BlendMode blendMode );
	void ClearScreen( const Rgba8& clearColor );
	void BeginCamera( const Camera& camera );
	void EndCamera(const Camera& camera);
	
	void DrawVertexArray( int numVerticies, const Vertex_PCU* verticies );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray );
	
	void		BindTexture( const Texture* texture );
	Texture*	CreateOrGetTextureFromFile( const char* imageFilePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* imageFilePath );

private:
	void CreateTextureFromFile( const char* imageFilePath );
	void CreateBitmapFontFromFile( const char* fontFilePath );
};