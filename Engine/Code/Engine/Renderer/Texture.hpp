#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>

class	RenderContext;
class	TextureView;
struct	Vec2;
struct	ID3D11Texture2D;

struct Texture
{
public:
	~Texture();
	Texture() {};
	explicit Texture( const char* imageFilePath, int textureID, IntVec2 imageTexelSize, int m_numComponents );
	explicit Texture( RenderContext* owner, ID3D11Texture2D* handle );
	explicit Texture( const char* filePath, RenderContext* owner, ID3D11Texture2D* handle );

	int					GetTextureID()		const { return m_textureID; }
	IntVec2				GetImageTexelSize() const { return m_imageTexelSize; }
	int					GetNumComponents()	const { return m_numComponents; }
	std::string const&	GetImageFilePath()	const { return m_imageFilePath; }
	RenderContext*		GetOwner()			const { return m_owner; }
	float				GetAspect()			const;
	Vec2				GetSize()			const;

	TextureView* CreateOrGetRenderTargetView();
	TextureView* GetOrCreateShaderResourceView();
	TextureView* GetOrCreateDepthStencilView();

private:
	RenderContext*		m_owner					= nullptr;
	ID3D11Texture2D*	m_handle				= nullptr;
	TextureView*		m_renderTargetView		= nullptr;
	TextureView*		m_shaderResourceView	= nullptr;
	TextureView*		m_depthStencilView		= nullptr;

	std::string			m_imageFilePath = "";
	int					m_textureID		= 0;
	int					m_numComponents = 0;
	IntVec2				m_imageTexelSize;
};