//---------------------------------------------------------------------------------------------------------
//		THIRD PARTY INCLUDES
//---------------------------------------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

//#include "Engine/Renderer/D3D11Common.hpp"

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
#include "Engine/Renderer/GPUSubMesh.hpp"
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

	m_effectCamera = new Camera( this );
	m_effectCamera->SetClearMode( CLEAR_NONE, Rgba8::BLACK );

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

	m_samplerPoint = GetOrCreateSampler( SAMPLER_POINT );
	m_samplerLinear = GetOrCreateSampler( SAMPLER_BILINEAR );
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

	delete m_effectCamera;
	m_effectCamera = nullptr;

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
void RenderContext::CopyTexture( Texture* destination, Texture* source )
{
	m_context->CopyResource( destination->GetHandle(), source->GetHandle() );
}


//---------------------------------------------------------------------------------------------------------
Sampler* RenderContext::GetOrCreateSampler( SamplerType samplerType, TextureAddressMode textureMode )
{
	for( uint samplerIndex = 0; samplerIndex < m_loadedSamplers.size(); ++samplerIndex )
	{
		Sampler* currentSampler = m_loadedSamplers[ samplerIndex ];
		if( currentSampler->IsMatching( samplerType, textureMode ) )
		{
			return currentSampler;
		}
	}

	Sampler* createdSampler = CreateSampler( samplerType, textureMode );
	return createdSampler;
}


//---------------------------------------------------------------------------------------------------------
ShaderState* RenderContext::GetOrCreateShaderStateFromFile( char const* filepath )
{
	for( uint shaderStateIndex = 0; shaderStateIndex < m_loadedShaderStates.size(); ++shaderStateIndex )
	{
		ShaderState* currentShaderState = m_loadedShaderStates[ shaderStateIndex ];
		if( currentShaderState->GetFilePath() == filepath )
		{
			return currentShaderState;
		}
	}

	ShaderState* createdShaderState = CreateShaderState( filepath );
	return createdShaderState;
}


//---------------------------------------------------------------------------------------------------------
Material* RenderContext::GetOrCreateMaterialFromFile( char const* filepath )
{
	for( uint materialIndex = 0; materialIndex < m_loadedMaterials.size(); ++materialIndex )
	{
		Material* currentMaterial = m_loadedMaterials[ materialIndex ];
		if( currentMaterial->m_filepath == filepath )
		{
			return currentMaterial;
		}
	}

	Material* createdMaterial = CreateMaterial( filepath );
	return createdMaterial;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ApplyFullscreenEffect( Texture* source, Texture* destination, Material* fullscreenMaterial )
{
	Shader* fullscreenShader = fullscreenMaterial->m_shaderState->GetShader();
	BeginFullscreenEffect( source, destination, fullscreenShader );
	BindMaterial( fullscreenMaterial );
	EndFullscreenEffect();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginFullscreenEffect( Texture* source, Texture* destination, Shader* fullscreenShader )
{
	m_effectCamera->SetColorTarget( destination );

	BeginCamera( *m_effectCamera );
	BindShader( fullscreenShader );
	BindTexture( source );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndFullscreenEffect()
{
	m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_context->Draw( 3, 0 );
	EndCamera( *m_effectCamera );
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
		case COMPARE_FUNC_LESS:		desc.DepthFunc = D3D11_COMPARISON_LESS; break;
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
void RenderContext::ClearRenderTargets( const Rgba8& clearColor, std::vector<ID3D11RenderTargetView*>& renderTargetsToClear )
{
	float clearFloats [4];
	clearFloats[0] = (float)clearColor.r / 255.0f;
	clearFloats[1] = (float)clearColor.g / 255.0f;
	clearFloats[2] = (float)clearColor.b / 255.0f;
	clearFloats[3] = (float)clearColor.a / 255.0f;

	for( uint rtvIndex = 0; rtvIndex < renderTargetsToClear.size(); ++rtvIndex )
	{
		ID3D11RenderTargetView* rtvToClear = renderTargetsToClear[ rtvIndex ];
		m_context->ClearRenderTargetView( rtvToClear, clearFloats );
	}
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
	
	std::vector<ID3D11RenderTargetView*> rtvs;
	uint rtvCount = camera.GetColorTargetCount();
	rtvs.resize( rtvCount );

	for( uint colorTargetIndex = 0; colorTargetIndex < rtvCount; ++colorTargetIndex )
	{
		rtvs[colorTargetIndex] = nullptr;

		Texture* colorTarget = camera.GetColorTarget( colorTargetIndex );
		if( colorTargetIndex == 0 && colorTarget == nullptr )
		{
			colorTarget = m_swapchain->GetBackBuffer();
		}

		if( colorTarget != nullptr )
		{
			TextureView* view = colorTarget->CreateOrGetRenderTargetView();
			rtvs[colorTargetIndex] = view->GetAsRTV();
		}
	}

	ID3D11DepthStencilView* dsv = nullptr;
	Texture* depthStencilTarget = camera.GetDepthStencilTarget();
	if( depthStencilTarget != nullptr )
	{
		TextureView* depthStencilView = depthStencilTarget->GetOrCreateDepthStencilView();
		dsv = depthStencilView->GetAsDSV();
	}
	m_context->OMSetRenderTargets( rtvCount, rtvs.data(), dsv );

	IntVec2 outputSize = camera.GetColorTargetTexelSize( 0 );

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
		ClearRenderTargets( camera.GetClearColor(), rtvs );
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
	case COMPARE_FUNC_LESS: newFunc = D3D11_COMPARISON_LESS; break;
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
	for( uint subMeshIndex = 0; subMeshIndex < mesh->GetSubMeshCount(); ++subMeshIndex )
	{
		DrawSubMesh( mesh->GetSubMesh( subMeshIndex ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawMeshWithMaterials( GPUMesh* mesh, Material** subMeshMaterials )
{
	for( uint subMeshIndex = 0; subMeshIndex < mesh->GetSubMeshCount(); ++subMeshIndex )
	{
		BindMaterial( subMeshMaterials[subMeshIndex] );
		DrawSubMesh( mesh->GetSubMesh( subMeshIndex ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawSubMesh( GPUSubMesh* subMesh )
{
	BindVertexInput( subMesh->GetVertexBuffer() );
	UpdateCurrentLayout( subMesh->GetVertexBuffer()->m_boundBufferAttribute );

	bool hasIndicies = subMesh->GetIndexCount() > 0;

	if( hasIndicies )
	{
		BindIndexBuffer( subMesh->GetIndexBuffer() );
		DrawIndexed( subMesh->GetIndexCount() );
	}
	else
	{
		Draw( subMesh->GetVertexCount() );
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
void RenderContext::UpdateModelUBO()
{
	model_data_t modelData;
	modelData.model = m_modelMatrix;
	modelData.specularFactor = m_specularFactor;
	modelData.specularPower = m_specularPower;
	modelData.tint = m_modelTint.GetValuesAsFractions();

	m_modelUBO->Update( &modelData, sizeof( modelData ), sizeof( modelData ) );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetModelUBO( Mat44 const& modelMatrix, Rgba8 const& modelTint, float specularFactor, float specularPower )
{
	SetModelMatrix( modelMatrix );
	SetModelTint( modelTint );
	SetSpecularFactor( specularFactor );
	SetSpecularPower( specularPower );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetModelMatrix( Mat44 const& modelMatrix )
{
	m_modelMatrix = modelMatrix;
	UpdateModelUBO();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetSpecularFactor( float specFactor )
{
	m_specularFactor = specFactor;
	UpdateModelUBO();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetSpecularPower( float specPower )
{
	m_specularPower = specPower;
	UpdateModelUBO();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetModelTint( Rgba8 const& modelTint )
{
	m_modelTint = modelTint;
	UpdateModelUBO();
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
Sampler* RenderContext::CreateSampler( SamplerType samplerType, TextureAddressMode textureMode )
{
	Sampler* newSampler = new Sampler( this, samplerType, textureMode );
	m_loadedSamplers.push_back( newSampler );
	return newSampler;
}


//---------------------------------------------------------------------------------------------------------
ShaderState* RenderContext::CreateShaderState( char const* filepath )
{
	ShaderState* newShaderState = new ShaderState( this, filepath );
	m_loadedShaderStates.push_back( newShaderState );
	return newShaderState;
}


//---------------------------------------------------------------------------------------------------------
Material* RenderContext::CreateMaterial( char const* filepath )
{
	Material* newMaterial = new Material( this, filepath );
	m_loadedMaterials.push_back( newMaterial );
	return newMaterial;
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

	Texture* newTexture = new Texture( imageFilePath, this, texHandle );
	newTexture->SetPixelData( imageTexelSizeX * imageTexelSizeY * 4, imageData );
	m_loadedTextures.push_back( newTexture );

// 	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( imageData );

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

	for( int renderTargetIndex = 0; renderTargetIndex < m_renderTargetPool.size(); ++renderTargetIndex )
	{
		delete m_renderTargetPool[ renderTargetIndex ];
		m_renderTargetPool[ renderTargetIndex ] = nullptr;
	}

	for( int samplerIndex = 0; samplerIndex < m_loadedSamplers.size(); ++samplerIndex )
	{
		delete m_loadedSamplers[ samplerIndex ];
		m_loadedSamplers[ samplerIndex ] = nullptr;
	}

	for( int shaderStateIndex = 0; shaderStateIndex < m_loadedShaderStates.size(); ++shaderStateIndex )
	{
		delete m_loadedShaderStates[ shaderStateIndex ];
		m_loadedShaderStates[ shaderStateIndex ] = nullptr;
	}

	for( int materialIndex = 0; materialIndex < m_loadedMaterials.size(); ++materialIndex )
	{
		delete m_loadedMaterials[ materialIndex ];
		m_loadedMaterials[ materialIndex ] = nullptr;
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
	if( shaderState != nullptr )
	{
		SetBlendMode( shaderState->GetBlendMode() );
		SetCullMode( shaderState->GetCullMode() );
		SetFillMode( shaderState->GetFillMode() );
		SetDepthTest( shaderState->GetCompareMode(), shaderState->IsWriteOnDepth() );
		SetFrontFaceWindOrder( shaderState->IsCounterClockwiseWindorder() );
		BindShader( shaderState->GetShader() );
		return;
	}

	BindShader( (Shader*)nullptr );
	SetBlendMode( BlendMode::ADDITIVE );
	SetCullMode( CULL_MODE_NONE );
	SetFillMode( FILL_MODE_SOLID );
	SetDepthTest( COMPARE_FUNC_LEQUAL, true );
	SetFrontFaceWindOrder( true );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindMaterial( Material* material )
{
	if( material != nullptr )
	{
		SetModelTint( material->m_tint );
		SetSpecularFactor( material->m_specularFactor );
		SetSpecularPower( material->m_specularPower );

		BindShaderState( material->m_shaderState );
		if( material->m_diffuseTexture != nullptr )
		{
			BindTexture( material->m_diffuseTexture );
		}
		else
		{
			BindTexture( nullptr );
		}

		if( material->m_normalTexture != nullptr )
		{
			BindNormalTexture( material->m_normalTexture );
		}
		else
		{
			BindNormalTexture( nullptr );
		}
		
		std::vector<Texture*>& texturesToBind = material->m_materialTexturesPerSlot;
		for( uint textureIndex = 0; textureIndex < texturesToBind.size(); ++ textureIndex )
		{
			BindMaterialTexture( 8 + textureIndex, texturesToBind[textureIndex] );
		}

		std::vector<Sampler*>& samplersToBind = material->m_samplersPerSlot;
		for( uint samplerIndex = 0; samplerIndex < samplersToBind.size(); ++samplerIndex )
		{
			BindSampler( samplersToBind[ samplerIndex ], samplerIndex );
		}

		material->UpdateUBOIfDirty();
		BindUniformBuffer( UBO_MATERIAL_SLOT, material->m_ubo );
		return;
	}

	SetModelTint( Rgba8::WHITE );
	SetSpecularFactor( 0.f );
	SetSpecularPower( 32.f );

	BindShaderState( nullptr );
	BindTexture( nullptr );
	BindSampler( m_samplerPoint );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindTextureByPath( char const* filepath )
{
	Texture* textureToBind = CreateOrGetTextureFromFile( filepath );
	BindTexture( textureToBind );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindShaderByPath( const char* filepath )
{
	Shader* shaderToBind = GetOrCreateShader( filepath );
	BindShader( shaderToBind );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindShaderStateByPath( const char* filepath )
{
	ShaderState* shaderStateToBind = GetOrCreateShaderStateFromFile( filepath );
	BindShaderState( shaderStateToBind );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindMaterialByPath( const char* filepath )
{
	Material* materialToBind = GetOrCreateMaterialFromFile( filepath );
	BindMaterial( materialToBind );
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
void RenderContext::BindSampler( Sampler* sampler, uint slot )
{
	Sampler* samplerToUse = sampler;
	if( samplerToUse == nullptr )
	{
		samplerToUse = m_samplerPoint;
	}

	ID3D11SamplerState* sampleHandle = samplerToUse->GetHandle();
	m_context->PSSetSamplers( slot, 1, &sampleHandle);
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
Texture* RenderContext::CreateDepthStencilBuffer( IntVec2 const& imageTexelDimensions, bool addToLoadedTexturePool )
{
	D3D11_TEXTURE2D_DESC depthDesc;

// 	depthDesc.Width = static_cast<unsigned int>( imageTexelDimensions.x );
// 	depthDesc.Height = static_cast<unsigned int>( imageTexelDimensions.y );
// 	depthDesc.MipLevels = 1;
// 	depthDesc.ArraySize = 1;
// 	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
// 	depthDesc.SampleDesc.Count = 1;
// 	depthDesc.SampleDesc.Quality = 0;
// 	depthDesc.Usage = D3D11_USAGE_DEFAULT;
// 	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
// 	depthDesc.CPUAccessFlags = 0;
// 	depthDesc.MiscFlags = 0;

	depthDesc.Width = static_cast<unsigned int>( imageTexelDimensions.x );
	depthDesc.Height = static_cast<unsigned int>( imageTexelDimensions.y );
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	ID3D11Texture2D* textureHandle = nullptr;
	m_device->CreateTexture2D( &depthDesc, nullptr, &textureHandle );
	Texture* newTexture = new Texture( this, textureHandle );

	if( addToLoadedTexturePool )
	{
		m_loadedTextures.push_back( newTexture );
	}

	newTexture->GetOrCreateDepthStencilView();
	return newTexture;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::CreateRenderTarget( IntVec2 const& imageTexelDimensions )
{
	D3D11_TEXTURE2D_DESC renderDesc;

	renderDesc.Width = static_cast<unsigned int>( imageTexelDimensions.x );
	renderDesc.Height = static_cast<unsigned int>( imageTexelDimensions.y );
	renderDesc.MipLevels = 1;
	renderDesc.ArraySize = 1;
	renderDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderDesc.SampleDesc.Count = 1;
	renderDesc.SampleDesc.Quality = 0;
	renderDesc.Usage = D3D11_USAGE_DEFAULT;
	renderDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderDesc.CPUAccessFlags = 0;
	renderDesc.MiscFlags = 0;

	ID3D11Texture2D* textureHandle = nullptr;
	m_device->CreateTexture2D( &renderDesc, nullptr, &textureHandle );
	Texture* newTexture = new Texture( this, textureHandle );
	return newTexture;
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::AcquireRenderTargetMatching( Texture* textureToMatch )
{
	IntVec2 textureTexelSize = textureToMatch->GetImageTexelSize();

	for( int renderTargetIndex = 0; renderTargetIndex < m_renderTargetPool.size(); ++renderTargetIndex )
	{
		Texture* renderTarget = m_renderTargetPool[ renderTargetIndex ];
		if( renderTarget->GetImageTexelSize() == textureTexelSize )
		{
			if( ( !renderTarget->IsDepthStencil() && textureToMatch->IsDepthStencil() ) ||
				( !textureToMatch->IsDepthStencil() && renderTarget->IsDepthStencil() ) )
			{
				continue;
			}

			m_renderTargetPool[ renderTargetIndex ] = m_renderTargetPool[ m_renderTargetPool.size() - 1 ];
			m_renderTargetPool.pop_back();
			return renderTarget;
		}
	}

	++m_totalRenderTargetsMade;
	Texture* newRenderTarget = nullptr;
	if( textureToMatch->IsDepthStencil() )
	{
		newRenderTarget = CreateDepthStencilBuffer( textureTexelSize, false );
	}
	else
	{
		newRenderTarget = CreateRenderTarget( textureTexelSize );
	}
	return newRenderTarget;
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ReleaseRenderTarget( Texture* textureToRelease )
{
	m_renderTargetPool.push_back( textureToRelease );
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