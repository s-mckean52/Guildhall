#pragma once

class RenderContext;
struct ID3D11SamplerState;

enum SamplerType
{
	SAMPLER_POINT,
	SAMPLER_BILINEAR,
};

class Sampler
{
public:
	Sampler ( RenderContext* context, SamplerType type );
	~Sampler();

	ID3D11SamplerState* GetHandle() const { return m_handle; }

public:
	RenderContext* m_owner;
	ID3D11SamplerState* m_handle;
};