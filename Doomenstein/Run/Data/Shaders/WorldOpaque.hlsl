

struct vs_input_t
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION;
	float4x4 VIEW;
}

cbuffer model_constants : register(b2)
{
	float4x4 MODEL;
	float4 TINT;
}

Texture2D <float4> tDiffuse	: register(t0);
SamplerState sSampler : register(s0);

struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;
};


v2f_t VertexFunction(vs_input_t input)
{
	v2f_t v2f = (v2f_t)0;

	v2f.position = float4(input.position, 1.0f);
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	float4 worldPos = float4(input.position, 1);
	float4 modelPos = mul(MODEL, worldPos);
	float4 cameraPos = mul(VIEW, modelPos);
	float4 clipPos = mul(PROJECTION, cameraPos);

	v2f.position = clipPos;

	return v2f;
}

float4 FragmentFunction(v2f_t input) : SV_Target0
{
	float4 color = tDiffuse.Sample(sSampler, input.uv);
	float4 final_color = color * input.color;

	if( final_color.a == 0.0f )
	{
		discard;
	}
	return final_color;
}