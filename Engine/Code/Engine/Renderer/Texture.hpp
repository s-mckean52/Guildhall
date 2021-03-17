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
	virtual ~Texture();
	Texture() {};
	explicit Texture( RenderContext* owner, ID3D11Texture2D* handle );
	explicit Texture( const char* filePath, RenderContext* owner, ID3D11Texture2D* handle );

	void				SetPixelData( int size, unsigned char* pixelData );

	bool				AreViewsMatching(Texture* textureToCompare) const;
	bool				IsRenderTarget()	const { return m_renderTargetView != nullptr; }
	bool				IsShaderResourse()	const { return m_shaderResourceView != nullptr; }
	bool				IsDepthStencil()	const { return m_depthStencilView != nullptr; }

	int					GetTextureID()		const { return m_textureID; }
	IntVec2				GetImageTexelSize() const { return m_imageTexelSize; }
	int					GetNumComponents()	const { return m_numComponents; }
	std::string const&	GetImageFilePath()	const { return m_imageFilePath; }
	RenderContext*		GetOwner()			const { return m_owner; }
	ID3D11Texture2D*	GetHandle()			const { return m_handle; }
	unsigned char*		GetPixelData()		const { return m_pixelData; }
	float				GetAspect()			const;
	Vec2				GetSize()			const;

	TextureView* CreateOrGetRenderTargetView();
	TextureView* GetOrCreateShaderResourceView();
	TextureView* GetOrCreateDepthStencilView();

protected:
	RenderContext*		m_owner					= nullptr;
	ID3D11Texture2D*	m_handle				= nullptr;
	TextureView*		m_renderTargetView		= nullptr;
	TextureView*		m_shaderResourceView	= nullptr;
	TextureView*		m_depthStencilView		= nullptr;

	unsigned char*		m_pixelData		= nullptr;
	std::string			m_imageFilePath = "";
	int					m_textureID		= 0;
	int					m_numComponents = 0;
	IntVec2				m_imageTexelSize;
};