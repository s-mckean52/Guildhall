#pragma once
#include "Engine/Math/IntVec2.hpp"


struct Texture
{
public:
	~Texture() {};
	Texture() {};
	explicit Texture( const char* imageFilePath, int textureID, IntVec2 imageTexelSize, int m_numComponents );

	int			GetTextureID()		const { return m_textureID; }
	IntVec2		GetImageTexelSize() const { return m_imageTexelSize; }
	int			GetNumComponents()	const { return m_numComponents; }
	const char* GetImageFilePath()	const { return m_imageFilePath; }
	float		GetAspect()			const;

private:
	const char* m_imageFilePath = "";
	
	int			m_textureID = 0;
	IntVec2		m_imageTexelSize;
	int			m_numComponents = 0;
};