//---------------------------------------------------------------------------------------------------------
//		THIRD PARTY INCLUDES
//---------------------------------------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

#include "Engine/Renderer/D3D11Common.hpp"

#pragma comment( lib, "d3d11.lib" )         // needed a01
#pragma comment( lib, "dxgi.lib" )          // needed a01
#pragma comment( lib, "d3dcompiler.lib" )   // needed when we get to shaders

//---------------------------------------------------------------------------------------------------------

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Sampler.hpp"


//---------------------------------------------------------------------------------------------------------
void RenderContext::StartUp( Window* theWindow )
{
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(RENDER_DEBUG)
	CreateDebugModule();
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGISwapChain* swapchain;

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	memset( &swapchainDesc, 0, sizeof( swapchainDesc ) );
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.Flags = 0;

	HWND hwnd = (HWND) theWindow->m_hwnd;
	swapchainDesc.OutputWindow = hwnd;
	swapchainDesc.SampleDesc.Count = 1;

	swapchainDesc.Windowed = TRUE;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferDesc.Width = theWindow->GetClientWidth();
	swapchainDesc.BufferDesc.Height = theWindow->GetClientHeight();

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags, //controls the type of device we make
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapchainDesc,
		&swapchain,
		&m_device,
		nullptr,
		&m_context );

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Failed to create rendering pipeline" );

	m_swapchain = new SwapChain( this, swapchain );
	//m_defaultShader = new Shader( this );
	m_defaultShader = GetOrCreateShader( "Data/Shaders/Default.hlsl" );

	m_immediateVBO = new VertexBuffer( this, MEMORY_HINT_DYNAMIC );

	m_frameUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_cameraUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	m_samplerDefault = new Sampler( this, SAMPLER_POINT );
	m_textueDefaultColor = CreateTextureFromColor( Rgba8::WHITE );

	CreateBlendStates();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	m_swapchain->Present();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ShutDown()
{
	ReleaseLoadedAssets();
	ReleaseBlendStates();

	delete m_samplerDefault;
	m_samplerDefault = nullptr;

	delete m_cameraUBO;
	m_cameraUBO = nullptr;

	delete m_frameUBO;
	m_frameUBO = nullptr;

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	delete m_swapchain;
	m_swapchain = nullptr;

	DX_SAFE_RELEASE( m_device );
	DX_SAFE_RELEASE( m_context );

	ReportLiveObjects();
	DestroyDebugModule();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateFrameTime( float deltaSeconds )
{
	frame_data_t frameData;
	frameData.system_time = static_cast<float>( GetCurrentTimeSeconds() );
	frameData.system_delta_time = deltaSeconds;

	m_frameUBO->Update( &frameData, sizeof( frameData ), sizeof( frameData ) );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetBlendMode( BlendMode blendMode )
{
	float const zeroes[] = { 0, 0, 0, 0 };
	
	switch( blendMode )
	{
	case BlendMode::ALPHA:
		m_context->OMSetBlendState( m_alphaBlendStateHandle, zeroes, ~0U );
		break;
	case BlendMode::ADDITIVE:
		m_context->OMSetBlendState( m_additiveBlendStateHandle, zeroes, ~0U );
		break;
	default:
		ERROR_AND_DIE( Stringf( "Unkown or unsupported blend mode #%i", blendMode ) );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( const Rgba8& clearColor )
{
	float clearFloats [4];
	clearFloats[0] = (float)clearColor.r / 255.0f;
	clearFloats[1] = (float)clearColor.g / 255.0f;
	clearFloats[2] = (float)clearColor.b / 255.0f;
	clearFloats[3] = (float)clearColor.a / 255.0f;

	Texture* backbuffer = m_swapchain->GetBackBuffer();
	TextureView* backbuffer_rtv = backbuffer->CreateOrGetRenderTargetView();

	ID3D11RenderTargetView* rtv = backbuffer_rtv->GetAsRTV();
	m_context->ClearRenderTargetView( rtv, clearFloats );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( const Camera& camera )
{
	m_context->ClearState();
	
	SetBlendMode( BlendMode::ALPHA );

	Texture* colorTarget = camera.GetColorTarget();
	if( colorTarget == nullptr )
	{
		colorTarget = m_swapchain->GetBackBuffer();
	}

	TextureView* view = colorTarget->CreateOrGetRenderTargetView();
	ID3D11RenderTargetView* rtv = view->GetAsRTV();
	m_context->OMSetRenderTargets( 1, &rtv, nullptr );

	IntVec2 outputSize = colorTarget->GetImageTexelSize();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(outputSize.x);
	viewport.Height = static_cast<float>(outputSize.y);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_context->RSSetViewports( 1, &viewport );

	if( camera.ShouldClearColor() )
	{
		ClearScreen( camera.GetClearColor() );
	}

	m_isDrawing = true;

	BindShader( (Shader*)nullptr );
	BindTexture( (Texture*)nullptr );
	BindSampler( (Sampler*)nullptr );
	m_lastBoundVBOHandle = nullptr;

	UpdateCameraData( camera );

	BindUniformBuffer( UBO_FRAME_SLOT, m_frameUBO );
	BindUniformBuffer( UBO_CAMERA_SLOT, m_cameraUBO );
}



//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateCameraData( Camera const& camera )
{
	camera_data_t cameraData;
	cameraData.projection = camera.GetProjectionMatrix();
	cameraData.view = Mat44::CreateTranslationXYZ( -camera.m_position );

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

//---------------------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
	m_isDrawing = false;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	ID3D11InputLayout* inputLayout = m_currentShader->GetOrCreateInputLayout( Vertex_PCU::LAYOUT );
	m_context->IASetInputLayout( inputLayout );

	m_context->Draw( numVertices, vertexOffset );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVerticies, const Vertex_PCU* verticies )
{
	//Update Vertex buffer
	size_t byteSize		= numVerticies * sizeof( Vertex_PCU );
	size_t elementSize	= sizeof( Vertex_PCU );
	m_immediateVBO->Update( verticies, byteSize, elementSize );

	//Bind
	BindVertexInput( m_immediateVBO );

	//Draw
	Draw( numVerticies );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray )
{
	DrawVertexArray( static_cast<int>( vertexArray.size() ), &vertexArray[ 0 ] );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateBlendStates()
{
	D3D11_BLEND_DESC alphaDesc;
	alphaDesc.AlphaToCoverageEnable = false;
	alphaDesc.IndependentBlendEnable = false;

	alphaDesc.RenderTarget[0].BlendEnable			= true;
	alphaDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
	alphaDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;

	alphaDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	alphaDesc.RenderTarget[0].DestBlendAlpha		= D3D11_BLEND_ZERO;

	alphaDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &alphaDesc, &m_alphaBlendStateHandle );


	D3D11_BLEND_DESC additiveDesc;
	additiveDesc.AlphaToCoverageEnable = false;
	additiveDesc.IndependentBlendEnable = false;

	additiveDesc.RenderTarget[0].BlendEnable			= true;
	additiveDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_ONE;

	additiveDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;

	additiveDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &alphaDesc, &m_additiveBlendStateHandle );
}


//---------------------------------------------------------------------------------------------------------
bool RenderContext::CreateTextureFromFile( const char* imageFilePath )
{
	int imageTexelSizeX = 0;
	int imageTexelSizeY = 0;
	int numComponents = 0; 
	int numComponentsRequested = 4;

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 );
	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( numComponents == 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );

	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width				= imageTexelSizeX;
	desc.Height				= imageTexelSizeY;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count	= 1; //MSAA
	desc.SampleDesc.Quality	= 0;
	desc.Usage				= D3D11_USAGE_IMMUTABLE;
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags		= 0;
	desc.MiscFlags			= 0;

	// Initialize Memory
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem				= imageData;
	initialData.SysMemPitch			= imageTexelSizeX * 4;
	initialData.SysMemSlicePitch	= 0; 

	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, &initialData, &texHandle );

// 	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( imageData );

	Texture* newTexture = new Texture( imageFilePath, this, texHandle );
	m_loadedTextures.push_back( newTexture );
	return true;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::CreateOrGetTextureFromFile( const char* imageFilePath )
{
	Texture* loadedTexture;
	for( int loadedTexturesIndex = 0; loadedTexturesIndex < m_loadedTextures.size(); ++loadedTexturesIndex )
	{
		loadedTexture = m_loadedTextures[ loadedTexturesIndex ];
		if( imageFilePath == loadedTexture->GetImageFilePath() )
		{
			return loadedTexture;
		}
	}
	CreateTextureFromFile( imageFilePath );
	return m_loadedTextures[ m_loadedTextures.size() - 1 ];
}


//---------------------------------------------------------------------------------------------------------
BitmapFont* RenderContext::CreateOrGetBitmapFontFromFile( const char* imageFilePath )
{
	BitmapFont* loadedBitFont;
	for( int loadedFontIndex = 0; loadedFontIndex < m_loadedFonts.size(); ++loadedFontIndex )
	{
		loadedBitFont = m_loadedFonts[ loadedFontIndex ];
		if( imageFilePath == loadedBitFont->m_fontName )
		{
			return loadedBitFont;
		}
	}
	CreateBitmapFontFromFile( imageFilePath );
	return m_loadedFonts[ m_loadedFonts.size() - 1 ];
}


//---------------------------------------------------------------------------------------------------------
Shader* RenderContext::GetOrCreateShader( char const* filename )
{
	Shader* newShader = new Shader( this );
	
	for( int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex )
	{
		Shader* shader = m_loadedShaders[ shaderIndex ];
		if( shader->GetFilePath() == filename )
		{
			return shader;
		}
	}
	newShader->CreateFromFile( filename );
	m_loadedShaders.push_back( newShader );
	return newShader;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::CreateTextureFromColor( Rgba8 const& color )
{
	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width				= 1;
	desc.Height				= 1;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count	= 1; //MSAA
	desc.SampleDesc.Quality	= 0;
	desc.Usage				= D3D11_USAGE_IMMUTABLE;
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags		= 0;
	desc.MiscFlags			= 0;

	// Initialize Memory
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem				= &color;
	initialData.SysMemPitch			= 4;
	initialData.SysMemSlicePitch	= 0;

	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, &initialData, &texHandle );

	Texture* newTexture = new Texture( this, texHandle );
	m_loadedTextures.push_back( newTexture );
	return newTexture;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ReleaseLoadedAssets()
{
	for( int textureIndex = 0; textureIndex < m_loadedTextures.size(); ++textureIndex )
	{
		delete m_loadedTextures[ textureIndex ];
		m_loadedTextures[ textureIndex ] = nullptr;
	}

	for( int fontIndex = 0; fontIndex < m_loadedFonts.size(); ++fontIndex )
	{
		delete m_loadedFonts[ fontIndex ];
		m_loadedFonts[ fontIndex ] = nullptr;
	}

	for( int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex )
	{
		delete m_loadedShaders[ shaderIndex ];
		m_loadedShaders[ shaderIndex ] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ReleaseBlendStates()
{
	DX_SAFE_RELEASE( m_additiveBlendStateHandle );
	DX_SAFE_RELEASE( m_alphaBlendStateHandle );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindTexture( const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>( constTexture );
	if( constTexture == nullptr )
	{
		texture = m_textueDefaultColor;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 0, 1, &srvHandle ); //srv

}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindShader( Shader* shader )
{
	ASSERT_OR_DIE( IsDrawing(), "Cannot bind shader if begin camera has not been called first" );

	m_currentShader = shader;
	if( m_currentShader == nullptr )
	{
		m_currentShader = m_defaultShader;
	}

	m_context->VSSetShader( m_currentShader->m_vertexStage.m_vs, nullptr, 0 );
	m_context->RSSetState( m_currentShader->m_rasterState );
	m_context->PSSetShader( m_currentShader->m_fragmentStage.m_fs, nullptr, 0 );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindShader( const char* filepath )
{
	m_currentShader = GetOrCreateShader( filepath );

	if( m_currentShader == nullptr )
	{
		m_currentShader = m_defaultShader;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindVertexInput( VertexBuffer* vbo )
{
	ID3D11Buffer* vboHandle = vbo->m_handle;
	UINT stride = sizeof( Vertex_PCU );
	UINT offset = 0;

	if( m_lastBoundVBOHandle != vboHandle )
	{
		m_context->IASetVertexBuffers( 0, 1, &vboHandle, &stride, &offset );
		m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_lastBoundVBOHandle = vboHandle;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer( unsigned int slot, RenderBuffer* ubo )
{
	ID3D11Buffer* uboHandle = ubo->m_handle; // GetHandle()

	m_context->VSSetConstantBuffers( slot, 1, &uboHandle );
	m_context->PSSetConstantBuffers( slot, 1, &uboHandle );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindSampler( Sampler* sampler )
{
	if( sampler == nullptr )
	{
		sampler = m_samplerDefault;
	}

	ID3D11SamplerState* sampleHandle = sampler->GetHandle();
	m_context->PSSetSamplers( 0, 1, &sampleHandle);
}


//---------------------------------------------------------------------------------------------------------
bool RenderContext::CreateBitmapFontFromFile( const char* fontFilePath )
{
	std::string fontImagePath = Stringf( "%s.png", fontFilePath );
	Texture* fontTexture = CreateOrGetTextureFromFile( fontImagePath.c_str() );
	BitmapFont* newFont = new BitmapFont( fontFilePath, fontTexture );
	m_loadedFonts.push_back( newFont );
	return true;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ReportLiveObjects()
{
	if( m_debug != nullptr )
	{
		m_debug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateDebugModule()
{
	m_debugModule = ::LoadLibraryA( "Dxgidebug.dll" );
	if( m_debugModule == nullptr )
	{
		ERROR_RECOVERABLE( "Failed to find dxgidebug.dll. No debug features enabled." );
	}
	else
	{
		typedef HRESULT( WINAPI* GetDebugModuleCB )(REFIID, void**);
		GetDebugModuleCB cb = (GetDebugModuleCB) ::GetProcAddress( (HMODULE)m_debugModule, "DXGIGetDebugInterface" );
		HRESULT hr = cb( __uuidof(IDXGIDebug), (void**)&m_debug );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Debug module failed to initilize." );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DestroyDebugModule()
{
	if( m_debug != nullptr )
	{
		DX_SAFE_RELEASE( m_debug );
		FreeLibrary( (HMODULE)m_debugModule );
		m_debug = nullptr;
		m_debugModule = nullptr;
	}
}