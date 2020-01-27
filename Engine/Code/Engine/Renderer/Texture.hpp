#pragma once
#include "Engine/Math/IntVec2.hpp"

class	RenderContext;
class	TextureView;
struct	ID3D11Texture2D;

struct Texture
{
public:
	~Texture();
	Texture() {};
	explicit Texture( const char* imageFilePath, int textureID, IntVec2 imageTexelSize, int m_numComponents );
	explicit Texture( RenderContext* owner, ID3D11Texture2D* handle );

	int			GetTextureID()		const { return m_textureID; }
	IntVec2		GetImageTexelSize() const { return m_imageTexelSize; }
	int			GetNumComponents()	const { return m_numComponents; }
	const char* GetImageFilePath()	const { return m_imageFilePath; }
	float		GetAspect()			const;

	TextureView* GetRenderTargetView();

private:
	RenderContext*		m_owner			= nullptr;
	ID3D11Texture2D*	m_handle		= nullptr;
	TextureView*		m_renderTargetView = nullptr;

	const char*			m_imageFilePath = "";
	int					m_textureID		= 0;
	int					m_numComponents = 0;
	IntVec2				m_imageTexelSize;
};