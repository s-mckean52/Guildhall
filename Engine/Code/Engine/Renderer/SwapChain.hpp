#pragma once


class	RenderContext;
struct	Texture;
struct	IDXGISwapChain;


class SwapChain
{
public:
	SwapChain( RenderContext* owner, IDXGISwapChain* handle );
	~SwapChain();

	void Present( int vsync = 0 );

	Texture* GetBackBuffer();

public:
	RenderContext*	m_owner;
	IDXGISwapChain*	m_handle;

	Texture* m_backbuffer = nullptr;
};