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
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/BuiltInShader.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/ShaderState.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Vertex_Master.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Platform/Window.hpp"


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

	Texture* backBufferTexture = m_swapchain->GetBackBuffer();
	m_defaultDepthStencil = CreateDepthStencilBuffer( backBufferTexture->GetImageTexelSize() );

	m_errorShader = CreateShaderFromSourceCode( BuiltInShader::BUILT_IN_ERROR );
	m_defaultShader = CreateShaderFromSourceCode( BuiltInShader::BUILT_IN_DEFAULT );

	m_immediateVBO = new VertexBuffer( this, MEMORY_HINT_DYNAMIC, Vertex_Master::LAYOUT );
	m_immediateIBO = new IndexBuffer( this, MEMORY_HINT_DYNAMIC );

	m_frameUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_modelUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_lightUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_materialUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	m_samplerPoint = new Sampler( this, SAMPLER_POINT );
	m_samplerLinear = new Sampler( this, SAMPLER_BILINEAR );
	m_textueDefaultColor = CreateTextureFromColor( Rgba8::WHITE );
	m_textureDefaultNormalColor = CreateTextureFromColor( Rgba8( 127, 127, 255, 255 ) );

	SetGameClock( nullptr );
	CreateBlendStates();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
	UpdateFrameUBO();
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

	delete m_samplerPoint;
	m_samplerPoint = nullptr;

	delete m_samplerLinear;
	m_samplerLinear = nullptr;

	delete m_frameUBO;
	m_frameUBO = nullptr;

	delete m_modelUBO;
	m_modelUBO = nullptr;

	delete m_lightUBO;
	m_lightUBO = nullptr;

	delete m_materialUBO;
	m_materialUBO = nullptr;

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	delete m_immediateIBO;
	m_immediateIBO = nullptr;

	delete m_swapchain;
	m_swapchain = nullptr;

	DX_SAFE_RELEASE( m_device );
	DX_SAFE_RELEASE( m_context );

	ReportLiveObjects();
	DestroyDebugModule();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateFrameUBO()
{
	frame_data_t frameData;
	frameData.system_time = static_cast<float>( m_gameClock->GetTotalElapsedSeconds() );
	frameData.system_delta_time = static_cast<float>( m_gameClock->GetLastDeltaSeconds() );
	frameData.gamma = m_gamma;
	frameData.inverseGamma = m_inverseGamma;

	frameData.fogFar = m_fogFar;
	frameData.fogNear = m_fogNear;
	frameData.fogNearColor = m_fogNearColor.GetValuesAsFractions();
	frameData.fogFarColor = m_fogFarColor.GetValuesAsFractions();

	m_frameUBO->Update( &frameData, sizeof( frameData ), sizeof( frameData ) );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateGamma( float gamma )
{
	m_gamma = gamma;
	m_inverseGamma = 1.f / gamma;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DisableFog()
{
	m_fogNear = -1.f;
	m_fogFar = -1.5f;
	m_fogNearColor = Rgba8::WHITE;
	m_fogFarColor = Rgba8::WHITE;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateLightUBO()
{
	light_data_t lightData;
	lightData.ambient = m_ambientLight;
	for( int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
	{
		lightData.lights[lightIndex] = m_lights[lightIndex];
	}

	m_lightUBO->Update( &lightData, sizeof( lightData ), sizeof( lightData ) );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EnableFog( float fogNear, float fogFar, Rgba8 const& fogNearColor, Rgba8 const& fogFarColor )
{
	m_fogFar = fogFar;
	m_fogNear = fogNear;
	m_fogFarColor = fogFarColor;
	m_fogNearColor = fogNearColor;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetGameClock( Clock* gameClock )
{
	m_gameClock = gameClock;
	if( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}
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
	case BlendMode::DISABLED:
		m_context->OMSetBlendState( m_disabledBlendStateHandle, zeroes, ~0U );
		break;
	default:
		ERROR_AND_DIE( Stringf( "Unkown or unsupported blend mode #%i", blendMode ) );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetDepthTest( CompareFunc compareFunc, bool writeDepthOnPass )
{
	if( !DoesDepthStateMatch( compareFunc, writeDepthOnPass ) ) 
	{
		DX_SAFE_RELEASE( m_currentDepthStencilState );

		D3D11_DEPTH_STENCIL_DESC desc;
		memset( &desc, 0, sizeof( desc ) );

		desc.DepthEnable = true;
		desc.DepthWriteMask =  writeDepthOnPass ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		switch( compareFunc )
		{
		case COMPARE_FUNC_NEVER:	desc.DepthFunc = D3D11_COMPARISON_NEVER; break;
		case COMPARE_FUNC_ALWAYS:	desc.DepthFunc = D3D11_COMPARISON_ALWAYS; break;
		case COMPARE_FUNC_LEQUAL:	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; break;
		case COMPARE_FUNC_GEQUAL:	desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
		case COMPARE_FUNC_EQUAL:	desc.DepthFunc = D3D11_COMPARISON_EQUAL; break;
		default: break;
		}

		m_device->CreateDepthStencilState( &desc, &m_currentDepthStencilState );
	}
	m_context->OMSetDepthStencilState( m_currentDepthStencilState, 0U );
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
void RenderContext::ClearDepth( Texture* depthStencilTexture, float depth )
{
	TextureView* view = depthStencilTexture->GetOrCreateDepthStencilView();
	ID3D11DepthStencilView* dsv = view->GetAsDSV();
	m_context->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, depth, 0 );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( Camera& camera )
{
	m_context->ClearState();
	
	SetBlendMode( BlendMode::ALPHA );
	m_currentVertexLayout = nullptr;

	Texture* colorTarget = camera.GetColorTarget();
	if( colorTarget == nullptr )
	{
		colorTarget = m_swapchain->GetBackBuffer();
	}

	TextureView* view = colorTarget->CreateOrGetRenderTargetView();
	ID3D11RenderTargetView* rtv = view->GetAsRTV();
	
	ID3D11DepthStencilView* dsv = nullptr;
	Texture* depthStencilTarget = camera.GetDepthStencilTarget();
	if( depthStencilTarget != nullptr )
	{
		TextureView* depthStencilView = depthStencilTarget->GetOrCreateDepthStencilView();
		dsv = depthStencilView->GetAsDSV();
	}
	m_context->OMSetRenderTargets( 1, &rtv, dsv );

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

	if( camera.ShouldClearDepth() && depthStencilTarget != nullptr )
	{
		ClearDepth( depthStencilTarget, camera.GetClearDepth() );
	}

	m_isDrawing = true;

	CreateRasterState();
	BindShader( (Shader*)nullptr );
	BindTexture( (Texture*)nullptr );
	BindNormalTexture( nullptr );
	BindSampler( (Sampler*)nullptr );
	m_lastBoundVBOHandle = nullptr;

	camera.UpdateCameraUBO();
	UpdateLightUBO();
	SetModelUBO( Mat44::IDENTITY );

	BindUniformBuffer( UBO_FRAME_SLOT, m_frameUBO );
	BindUniformBuffer( UBO_CAMERA_SLOT, camera.GetUBO() );
	BindUniformBuffer( UBO_MODEL_MATRIX_SLOT, m_modelUBO );
	BindUniformBuffer( UBO_LIGHT_SLOT, m_lightUBO );
	BindUniformBuffer( UBO_MATERIAL_SLOT, m_materialUBO );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
	DX_SAFE_RELEASE( m_currentDepthStencilState );
	DX_SAFE_RELEASE( m_rasterState );
	m_isDrawing = false;
}


//---------------------------------------------------------------------------------------------------------
bool RenderContext::DoesDepthStateMatch( CompareFunc compareFunc, bool writeDepthOnPass )
{
	if( m_currentDepthStencilState == nullptr ) return false;

	D3D11_DEPTH_STENCIL_DESC currentDesc;	
	m_currentDepthStencilState->GetDesc( &currentDesc );

	D3D11_DEPTH_WRITE_MASK newWriteMask = writeDepthOnPass ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

	D3D11_COMPARISON_FUNC newFunc;
	switch( compareFunc )
	{
	case COMPARE_FUNC_NEVER: newFunc = D3D11_COMPARISON_NEVER; break;
	case COMPARE_FUNC_ALWAYS: newFunc = D3D11_COMPARISON_ALWAYS; break;
	case COMPARE_FUNC_LEQUAL: newFunc = D3D11_COMPARISON_LESS_EQUAL; break;
	case COMPARE_FUNC_GEQUAL: newFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
	case COMPARE_FUNC_EQUAL: newFunc = D3D11_COMPARISON_EQUAL; break;
	default: ERROR_AND_DIE( "Unsupported compare func" ); break;
	}

	if( currentDesc.DepthFunc == newFunc && currentDesc.DepthWriteMask == newWriteMask )
	{
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::GetBackBuffer() const
{
	return m_swapchain->GetBackBuffer();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	m_context->Draw( numVertices, vertexOffset );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawIndexed( int numIndicies, int indexOffset, int vertexOffset )
{
	m_context->DrawIndexed( numIndicies, indexOffset, vertexOffset );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawIndexedVertexArray( std::vector<Vertex_PCU>& verticies, std::vector<unsigned int>& indicies )
{
	if( verticies.size() == 0 ) return;

	unsigned int vertexCount = static_cast<unsigned int>( verticies.size() );
	unsigned int vertexStride = sizeof( Vertex_PCU );
	unsigned int byteSize = vertexCount * vertexStride;
	m_immediateVBO->Update( &verticies[ 0 ], byteSize, vertexStride );
	m_immediateVBO->m_boundBufferAttribute = Vertex_PCU::LAYOUT;

	unsigned int indexCount = static_cast<unsigned int>( indicies.size() );

	BindVertexInput( m_immediateVBO );
	UpdateCurrentLayout( m_immediateVBO->m_boundBufferAttribute );

	bool hasIndicies = indexCount > 0;

	if ( hasIndicies )
	{
		m_immediateIBO->Update( indexCount, &indicies[ 0 ] );
		BindIndexBuffer( m_immediateIBO );
		DrawIndexed( indexCount );
	}
	else
	{
		Draw( vertexCount );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVerticies, const Vertex_PCU* verticies )
{
	//Update Vertex buffer
	size_t byteSize		= numVerticies * sizeof( Vertex_PCU );
	size_t elementSize	= sizeof( Vertex_PCU );
	m_immediateVBO->Update( verticies, byteSize, elementSize );
	m_immediateVBO->m_boundBufferAttribute = Vertex_PCU::LAYOUT;
	UpdateCurrentLayout( Vertex_PCU::LAYOUT );

	//Bind
	BindVertexInput( m_immediateVBO );
	UpdateCurrentLayout( m_immediateVBO->m_boundBufferAttribute );

	//Draw
	Draw( numVerticies );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray )
{
	DrawVertexArray( static_cast<int>( vertexArray.size() ), &vertexArray[ 0 ] );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawMesh( GPUMesh* mesh )
{
	BindVertexInput( mesh->GetVertexBuffer() );
	UpdateCurrentLayout( mesh->GetVertexBuffer()->m_boundBufferAttribute );

	bool hasIndicies = mesh->GetIndexCount() > 0;

	if( hasIndicies )
	{
		BindIndexBuffer( mesh->GetIndexBuffer() );
		DrawIndexed( mesh->GetIndexCount() );
	}
	else
	{
		Draw( mesh->GetVertexCount() );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::UpdateCurrentLayout( buffer_attribute_t const* newLayout )
{
	if( newLayout != m_currentVertexLayout || m_currentShader != m_lastBoundShader )
	{
		ID3D11InputLayout* inputLayout = m_currentShader->GetOrCreateInputLayout( newLayout );
		m_context->IASetInputLayout( inputLayout );
		m_currentVertexLayout = newLayout;
		m_lastBoundShader = m_currentShader;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetModelUBO( Mat44 const& modelMatrix, Rgba8 const& modelTint, float specularFactor, float specularPower )
{
	model_data_t modelData;
	modelData.model				= modelMatrix;
	modelData.specularFactor	= specularFactor;
	modelData.specularPower		= specularPower;
	modelData.tint				= modelTint.GetValuesAsFractions();

	m_modelUBO->Update( &modelData, sizeof( modelData ), sizeof( modelData ) );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetCullMode( CullMode cullMode )
{
	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );

	switch( cullMode )
	{
	case CULL_MODE_NONE:
		desc.CullMode = D3D11_CULL_NONE;
		break;
	case CULL_MODE_BACK:
		desc.CullMode = D3D11_CULL_BACK;
		break;
	case CULL_MODE_FRONT:
		desc.CullMode = D3D11_CULL_FRONT;
		break;
	}

	DX_SAFE_RELEASE( m_rasterState );
	m_device->CreateRasterizerState( &desc, &m_rasterState );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetFillMode( FillMode fillMode )
{
	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );

	switch( fillMode )
	{
	case FILL_MODE_SOLID:
		desc.FillMode = D3D11_FILL_SOLID;
		break;
	case FILL_MODE_WIREFRAME:
		desc.FillMode = D3D11_FILL_WIREFRAME;
		break;
	}

	DX_SAFE_RELEASE( m_rasterState );
	m_device->CreateRasterizerState( &desc, &m_rasterState );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetFrontFaceWindOrder( bool isCounterClockwise )
{
	D3D11_RASTERIZER_DESC desc;
	m_rasterState->GetDesc( &desc );
	
	desc.FrontCounterClockwise = isCounterClockwise;

	DX_SAFE_RELEASE( m_rasterState );
	m_device->CreateRasterizerState( &desc, &m_rasterState );
}


//---------------------------------------------------------------------------------------------------------
Shader* RenderContext::CreateShaderFromFilePath( char const* filename )
{
	Shader* newShader = new Shader( this );
	if( newShader->CreateFromFile( filename ) )
	{
		m_loadedShaders.push_back( newShader );
		return newShader;
	}

	delete newShader;
	return m_errorShader;
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


	D3D11_BLEND_DESC disabledDesc;
	disabledDesc.AlphaToCoverageEnable = false;
	disabledDesc.IndependentBlendEnable = false;

	disabledDesc.RenderTarget[0].BlendEnable			= true;
	disabledDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP_ADD;
	disabledDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_ONE;
	disabledDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_ZERO;

	disabledDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	disabledDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	disabledDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;

	disabledDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &disabledDesc, &m_disabledBlendStateHandle );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateRasterState()
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = D3D11_FILL_SOLID;	//Filled Triangle
	desc.CullMode = D3D11_CULL_NONE;
	desc.FrontCounterClockwise = TRUE;
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = TRUE;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	m_device->CreateRasterizerState( &desc, &m_rasterState );
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
	for( int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex )
	{
		Shader* shader = m_loadedShaders[ shaderIndex ];
		if( shader != nullptr && strcmp( shader->GetFilePath(), filename ) == 0 )
		{
			return shader;
		}
	}
	return CreateShaderFromFilePath( filename );
}


//---------------------------------------------------------------------------------------------------------
Shader* RenderContext::CreateShaderFromSourceCode( char const* sourceCode )
{
	Shader* newShader = new Shader( this );
	if( newShader->CreateFromSourceCode( sourceCode ) )
	{
		m_loadedShaders.push_back( newShader );
		return newShader;
	}
	delete newShader;
	return m_errorShader;
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
	DX_SAFE_RELEASE( m_disabledBlendStateHandle );
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
void RenderContext::BindNormalTexture( const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>( constTexture );
	if( constTexture == nullptr )
	{
		texture = m_textureDefaultNormalColor;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( 1, 1, &srvHandle ); //srv
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindMaterialTexture( unsigned int slot, const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>(constTexture);
	if (constTexture == nullptr)
	{
		texture = m_textueDefaultColor;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->GetAsSRV();
	m_context->PSSetShaderResources( slot, 1, &srvHandle ); //srv
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindShaderState( ShaderState* shaderState )
{
	BindShader( shaderState->GetShader() );
	SetBlendMode( shaderState->GetBlendMode() );
	SetCullMode( shaderState->GetCullMode() );
	SetFillMode( shaderState->GetFillMode() );
	SetDepthTest( shaderState->GetCompareMode(), shaderState->IsWriteOnDepth() );
	SetFrontFaceWindOrder( shaderState->IsCounterClockwiseWindorder() );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ReloadShaders()
{
	for( int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex )
	{
		Shader* currentShader = m_loadedShaders[ shaderIndex ];
		if( currentShader != nullptr && strcmp( currentShader->GetFilePath(), "" ) != 0)
		{
			currentShader->Recompile();
		}
	}
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
	m_context->RSSetState( m_rasterState );
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

	BindShader( m_currentShader );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindVertexInput( VertexBuffer* vbo )
{
	ID3D11Buffer* vboHandle = vbo->m_handle;
	UINT stride = static_cast<UINT>( vbo->m_elementByteSize );
	UINT offset = 0;

	if( m_lastBoundVBOHandle != vboHandle )
	{
		m_context->IASetVertexBuffers( 0, 1, &vboHandle, &stride, &offset );
		m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_lastBoundVBOHandle = vboHandle;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindIndexBuffer( IndexBuffer* ibo )
{
	ID3D11Buffer* iboHandle = ibo->m_handle;
	UINT offset = 0;

	m_context->IASetIndexBuffer( iboHandle, DXGI_FORMAT_R32_UINT, offset );
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
		sampler = m_samplerPoint;
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
Texture* RenderContext::CreateDepthStencilBuffer( IntVec2 const& imageTexelDimensions )
{
	D3D11_TEXTURE2D_DESC depthDesc;

	depthDesc.Width = static_cast<unsigned int>( imageTexelDimensions.x );
	depthDesc.Height = static_cast<unsigned int>( imageTexelDimensions.y );
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	ID3D11Texture2D* textureHandle = nullptr;
	m_device->CreateTexture2D( &depthDesc, nullptr, &textureHandle );
	Texture* newTexture = new Texture( this, textureHandle );
	m_loadedTextures.push_back( newTexture );
	return newTexture;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::CreateRenderTarget( IntVec2 const& imageTexelDimensions )
{
	D3D11_TEXTURE2D_DESC depthDesc;

	depthDesc.Width = static_cast<unsigned int>( imageTexelDimensions.x );
	depthDesc.Height = static_cast<unsigned int>( imageTexelDimensions.y );
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	ID3D11Texture2D* textureHandle = nullptr;
	m_device->CreateTexture2D( &depthDesc, nullptr, &textureHandle );
	Texture* newTexture = new Texture( this, textureHandle );
	m_loadedTextures.push_back( newTexture );
	return newTexture;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetAmbientColor( Rgba8 const& ambientColor )
{
	Vec4 ambientColorAsFloats = ambientColor.GetValuesAsFractions();
	m_ambientLight.x = ambientColorAsFloats.x;
	m_ambientLight.y = ambientColorAsFloats.y;
	m_ambientLight.z = ambientColorAsFloats.z;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetAmbientIntensity( float ambientIntensity )
{
	m_ambientLight.w = ambientIntensity;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetAmbientLight( Rgba8 const& ambientColor, float ambientIntensity )
{
	SetAmbientColor( ambientColor );
	SetAmbientIntensity( ambientIntensity );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EnableLight( unsigned int index, Light const& lightInfo )
{
	m_lights[ index ] = lightInfo;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DisableLight( unsigned int index )
{
	m_lights[ index ].intensity = 0.f;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DisableAllLights()
{
	for( unsigned int lightIndex = 0; lightIndex < 8; ++lightIndex )
	{
		DisableLight( lightIndex );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetMaterialUBO( void const* data, unsigned int dataSize )
{
	m_materialUBO->Update( data, dataSize, dataSize );
	BindUniformBuffer( UBO_MATERIAL_SLOT, m_materialUBO );
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