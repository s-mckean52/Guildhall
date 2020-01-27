#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/TextureView.hpp"


//---------------------------------------------------------------------------------------------------------
TextureView::~TextureView()
{
	DX_SAFE_RELEASE( m_rtv );
}
