//--------------------------------------------------------------------------------------
// Stream Input
// ------
// Stream Input is input that is walked by the vertex shader.  
// If you say "Draw(3,0)", you are telling to the GPU to expect '3' sets, or 
// elements, of input data.  IE, 3 vertices.  Each call of the VertxShader
// we be processing a different element. 
//--------------------------------------------------------------------------------------

// inputs are made up of internal names (ie: uv) and semantic names
// (ie: TEXCOORD).  "uv" would be used in the shader file, where
// "TEXCOORD" is used from the client-side (cpp code) to attach ot. 
// The semantic and internal names can be whatever you want, 
// but know that semantics starting with SV_* usually denote special 
// inputs/outputs, so probably best to avoid that naming.
struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position      : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;

	float3 tangent		: TANGENT;
	float3 bitangent	: BITANGENT;
	float3 normal		: NORMAL;

	float4 jacobian		: JACOBIAN;
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


Texture2D<float4>	tDiffuse			: register(t0);
Texture2D<float4>	tNormal				: register(t1);
TextureCube<float4> tSkybox				: register(t5);
Texture2D<float4>	tBackBuffer			: register(t6);
Texture2D<float4>	tDepthStencil		: register(t7);
Texture2D<float4>	tScrollingNormal1	: register(t8);
Texture2D<float4>	tScrollingNormal2	: register(t9);

SamplerState sSampler					: register(s0);


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
	float2 uv       : UV;

    float3 camera_position  : POSITION0;
	float3 world_position   : POSITION1;
	float3 world_normal     : NORMAL;
	float3 world_tangent    : TANGENT;
	float3 world_bitangent  : BITANGENT;

	float4 jacobian : JACOBIAN;
};

//--------------------------------------------------------------------------------------
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float domain = inMax - inMin;
	float range = outMax - outMin;
	return ((val - inMin) / domain) * range + outMin;
}

//--------------------------------------------------------------------------------------
float3 GetNormalFromColor( float3 sampledColor )
{
	return ( sampledColor * float3( 2.0f, 2.0f, 1.0f ) ) - float3( 1.0f, 1.0f, 0.0f );
}

//--------------------------------------------------------------------------------------
float3 GetColorFromNormalDir( float3 normalizedDir )
{
	return ( normalizedDir + float3( 1.f, 1.f, 1.f ) ) * float3( 0.5f, 0.5f, 0.5f );
}

//--------------------------------------------------------------------------------------
float3x3 GetVertexTBN( v2f_t input )
{
	float3 normal = normalize( input.world_normal );
	float3 tangent = normalize( input.world_tangent );
	float3 bitangent = normalize( input.world_bitangent );
	float3x3 tbn = float3x3( tangent, bitangent, normal );
	return tbn;
}

//--------------------------------------------------------------------------------------
float LinearizeDepth( float depth, float nearPlaneDepth, float farPlaneDepth )
{
    float zRange = farPlaneDepth - nearPlaneDepth;
    return -( nearPlaneDepth * farPlaneDepth ) / ( ( depth * zRange ) - farPlaneDepth );
}


//--------------------------------------------------------------------------------------
float3 NDCToCameraSpace( float4 ndcPosition, float4x4 inverseProjection )
{
    float4 cameraPos = mul( inverseProjection, ndcPosition );
    cameraPos /= cameraPos.w;
    
    return cameraPos.xyz;
}


//--------------------------------------------------------------------------------------
// By Morgan McGuire and Michael Mara at Williams College 2014
// Released as open source under the BSD 2-Clause License
// http://opensource.org/licenses/BSD-2-Clause

float distanceSquared(float2 a, float2 b) { a -= b; return dot(a, a); }

// Returns true if the ray hit something
bool traceScreenSpaceRay1(
	float3 csOrig,
	float3 csDir,
	float4x4 proj,
	Texture2D csZBuffer,
	float2 csZBufferSize,
	float zThickness,
	float nearPlaneZ,
	float stride,
	float jitter,
	const float maxSteps,
	float maxDistance,
	out float2 hitPixel,
	out float3 hitPoint)
{
	hitPixel = float2( -1.f, -1.f );
	hitPoint = float3( 0.f, 0.f, 0.f );
	// Clip to the near plane  
	float maxZPos = csOrig.z + csDir.z * maxDistance;
	float rayLength;
	if (maxZPos > nearPlaneZ)
	{
		rayLength = (nearPlaneZ - csOrig.z) / csDir.z;
	}
	else
	{
		rayLength = maxDistance;
	}
	float3 csEndPoint = csOrig + csDir * rayLength;

	// Project into homogeneous clip space
	float4 clipSpaceStartPos = mul( proj, float4(csOrig, 1.0) );
	float4 clipSpaceEndPos = mul( proj, float4(csEndPoint, 1.0) );
	float k0 = 1.0 / clipSpaceStartPos.w;
	float k1 = 1.0 / clipSpaceEndPos.w;

	// The interpolated homogeneous version of the camera-space points  
	float3 homogenousStartPos = csOrig * k0;
	float3 homogenousEndPos = csEndPoint * k1;

	// Screen-space endpoints
	float2 screenSpaceStartPos = clipSpaceStartPos.xy * k0;
	float2 screenSpaceEndPos = clipSpaceEndPos.xy * k1;
//	screenSpaceStartPos.x = RangeMap( screenSpaceStartPos.x, -1.f, 1.f, 0.f, csZBufferSize.x );
//	screenSpaceStartPos.y = RangeMap( screenSpaceStartPos.y, -1.f, 1.f, 0.f, csZBufferSize.y );
//	screenSpaceEndPos.x = RangeMap( screenSpaceEndPos.x, -1.f, 1.f, 0.f, csZBufferSize.x );
//	screenSpaceEndPos.y = RangeMap( screenSpaceEndPos.y, -1.f, 1.f, 0.f, csZBufferSize.y );

	// If the line is degenerate, make it cover at least one pixel
	// to avoid handling zero-pixel extent as a special case later
	screenSpaceEndPos += ( ( distanceSquared( screenSpaceStartPos, screenSpaceEndPos ) < 0.0001f ) ? float2( 0.01f, 0.01f ) : float2( 0.0f, 0.0f ) );
	float2 delta = screenSpaceEndPos - screenSpaceStartPos;

	// Permute so that the primary iteration is in x to collapse
	// all quadrant-specific DDA cases later
	bool permute = false;
	if (abs(delta.x) < abs(delta.y)) {
		// This is a more-vertical line
		permute = true;
		delta = delta.yx;
		screenSpaceStartPos = screenSpaceStartPos.yx;
		screenSpaceEndPos = screenSpaceEndPos.yx;
	}

	float stepDir = sign(delta.x);
	float invdx = stepDir / delta.x;

	// Track the derivatives of Q and k
	float3  dHomogenous = (homogenousEndPos - homogenousStartPos) * invdx;
	float	dk = (k1 - k0) * invdx;
	float2  dScreenSpace = float2(stepDir, delta.y * invdx);

	// Scale derivatives by the desired pixel stride and then
	// offset the starting values by the jitter fraction
	dScreenSpace *= stride;
	dHomogenous *= stride;
	dk *= stride;
	screenSpaceStartPos += dScreenSpace * jitter;
	homogenousStartPos += dHomogenous * jitter;
	k0 += dk * jitter;

	// Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
	float3 homogenousPos = homogenousStartPos;

	// Adjust end condition for iteration direction
	float  end = screenSpaceEndPos.x * stepDir;

	float k = k0;
	float stepCount = 0.0;
	float prevZMaxEstimate = csOrig.z;
	float rayZMin = prevZMaxEstimate;
	float rayZMax = prevZMaxEstimate;
	float sceneZMax = rayZMax + 1000.f;
	for (float2 screenSpacePos = screenSpaceStartPos;
		((screenSpacePos.x * stepDir) <= end) && (stepCount < maxSteps) &&
		((rayZMax < sceneZMax - zThickness) || (rayZMin > sceneZMax)) &&
		(sceneZMax != 0);
		screenSpacePos += dScreenSpace, homogenousPos.z += dHomogenous.z, k += dk, ++stepCount) {

		rayZMin = prevZMaxEstimate;
		rayZMax = (dHomogenous.z * 0.5f + homogenousPos.z) / (dk * 0.5f + k);
		prevZMaxEstimate = rayZMax;
		if (rayZMin > rayZMax) {
			float t = rayZMin;
			rayZMin = rayZMax;
			rayZMax = t;
		}

		hitPixel = permute ? screenSpacePos.yx : screenSpacePos;
		hitPixel.y = csZBufferSize.y - hitPixel.y;
		// You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
		// is different than ours in screen space
		sceneZMax = csZBuffer.Load( int3( hitPixel, 0 ) ).x;
		sceneZMax = LinearizeDepth( sceneZMax, nearPlaneZ, 100.f );
	}

	// Advance Q based on the number of steps
	homogenousPos.xy += dHomogenous.xy * stepCount;
	hitPoint = homogenousPos * (1.0f / k);
	//return float4(screenSpaceStartPos / csZBufferSize, 0.f, 1.f);
	return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);
}


//--------------------------------------------------------------------------------------
float3 GetPerturbedColor( float2 dirToPerturb, float perturbationFactor, float2 pixelPosition, Texture2D backBuffer, float2 backBufferDim, out float2 perturbedPixel )
{
    float2 offset = dirToPerturb * perturbationFactor;
    perturbedPixel = pixelPosition + offset;
    float2 perturbedPixelUV = saturate( perturbedPixel / backBufferDim );
    perturbedPixel = perturbedPixelUV * ( backBufferDim - float2( 1.f, 1.f ) );
    
    return backBuffer.Sample( sSampler, perturbedPixelUV );
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
	v2f_t v2f = (v2f_t)0;

	float4 local_position = float4(input.position, 1.0f);
	float4 world_position = mul(MODEL, local_position);
	float4 camera_position = mul(VIEW, world_position);
	float4 clip_position = mul(PROJECTION, camera_position);

	float4 local_normal = float4(input.normal, 0.0f);
	float4 local_tangent = float4(input.tangent, 0.0f);
	float4 local_bitangent = float4(input.bitangent, 0.0f);
	float4 world_normal = mul(MODEL, local_normal);
	float4 world_tangent = mul(MODEL, local_tangent);
	float4 world_bitangent = mul(MODEL, local_bitangent);
    
	v2f.position = clip_position;
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;
    
    v2f.camera_position = camera_position.xyz;
	v2f.world_position = world_position.xyz;
	v2f.world_normal = world_normal.xyz;
	v2f.world_tangent = world_tangent.xyz;
	v2f.world_bitangent = world_bitangent.xyz;
	v2f.jacobian = input.jacobian;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction(v2f_t input) : SV_Target1
{
    return float4( 0.f, 0.f, 0.f, 1.f );
}
