//---------------------------------------------------------------------------------------------------------
// Structs
//---------------------------------------------------------------------------------------------------------
struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position : POSITION;
	float4 color    : COLOR;
	float2 uv       : TEXCOORD;

	float3 tangent      : TANGENT;
	float3 bitangent    : BITANGENT;
	float3 normal       : NORMAL;
};


struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 uv       : UV;

	float3 world_position   : POSITION0;
	float3 world_normal     : NORMAL;
	float3 world_tangent    : TANGENT;
	float3 world_bitangent  : BITANGENT;
};


struct light_t
{
	float3 world_position;
	float intensity;

	float3 direction;
	float cos_inner_half_angle;

	float3 color;
	float cos_outter_half_angle;

	float3 attenuation;
	float is_directional;

	float3 spec_attenuation;
	float padding;
};


//---------------------------------------------------------------------------------------------------------
// Buffers
//---------------------------------------------------------------------------------------------------------
cbuffer time_constants : register(b0)
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
	float GAMMA;
	float INVERSE_GAMMA;
};

cbuffer camera_constants : register(b1)
{
	float4x4 PROJECTION;            // CAMERA_TO_CLIP
	float4x4 VIEW;		            //WORLD_TO_CAMERA

	float4x4 CAMERA_MODEL;
	float3 CAMERA_POSITION;
	float padding_00;

	float4x4 INVERSE_PROJECTION;    // CLIP_TO_CAMERA
}

cbuffer model_constants : register(b2)
{
	float4x4 MODEL;
	float4 TINT;

	float SPECULAR_FACTOR;
	float SPECULAR_POWER;
	float2 padding_01;
}

cbuffer light_constants : register(b3)
{
	float4 AMBIENT;
	light_t LIGHTS[8];
}


//---------------------------------------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
float RangeMap(float val, float inMin, float inMax, float outMin, float outMax)
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
}

//--------------------------------------------------------------------------------------
float3 GetNormalFromColor(float3 sampledColor)
{
	return (sampledColor * float3(2.0f, 2.0f, 1.0f)) - float3(1.0f, 1.0f, 0.0f);
}


//--------------------------------------------------------------------------------------
float3 GetColorFromNormalDir(float3 normalizedDir)
{
	return (normalizedDir + float3(1.f, 1.f, 1.f)) * float3(0.5f, 0.5f, 0.5f);
}


//--------------------------------------------------------------------------------------
float3x3 GetVertexTBN( float3 tangent, float3 bitangent, float3 normal )
{
	normal		= normalize( normal );
	tangent		= normalize( tangent );
	bitangent	= normalize( bitangent );
	float3x3 tbn = float3x3( tangent, bitangent, normal );
	return tbn;
}


//--------------------------------------------------------------------------------------
float LinearizeDepth(float depth, float nearPlaneDepth, float farPlaneDepth)
{
	float zRange = farPlaneDepth - nearPlaneDepth;
	return -(nearPlaneDepth * farPlaneDepth) / ((depth * zRange) - farPlaneDepth);
}


//--------------------------------------------------------------------------------------
float3 NDCToCameraSpace(float4 ndcPosition, float4x4 inverseProjection)
{
	float4 cameraPos = mul(inverseProjection, ndcPosition);
	cameraPos /= cameraPos.w;

	return cameraPos.xyz;
}