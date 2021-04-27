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
float4 FragmentFunction(v2f_t input) : SV_Target0
{
	//add to buffer
	//------------------------------------------------------------------------------
	const float4 normalmap_scroll = float4( 1.f, 0.f, 0.f, 1.f );
	const float2 normalmap_scroll_speed = float2( 0.01f, 0.01f );
	
	//Water Constants
	float NEAR_PLANE = -0.9f;
	float FAR_PLANE = -100.f;
	float3 upwelling =	float3( 0.f, 0.2f, 0.3f );
	float3 sky =		float3( 0.69f, 0.84f, 1.f );
	float3 air =		float3( 0.1f, 0.1f, 0.1f );
	float nSnell	= 1.34f;
	float kDiffuse	= 0.91f;
	float MAX_DEPTH = 15.f;
	float waterFalloff = 1.f / MAX_DEPTH;
	//------------------------------------------------------------------------------


	float2 normal_scroll_uv1 = input.uv + SYSTEM_TIME_SECONDS * normalmap_scroll.xy * normalmap_scroll_speed.x;
	float2 normal_scroll_uv2 = input.uv + SYSTEM_TIME_SECONDS * normalmap_scroll.zw * normalmap_scroll_speed.y;
	
	float4 normal_color1 = tScrollingNormal1.Sample(sSampler, normal_scroll_uv1);
	float4 normal_color2 = tScrollingNormal2.Sample(sSampler, normal_scroll_uv2);

	float3 normal1 = GetNormalFromColor( normal_color1.xyz );
	float3 normal2 = GetNormalFromColor( normal_color2.xyz );

	float3x3 tbn = GetVertexTBN( input );

	float3 world_normal = normalize( mul( normal1, tbn ) );
	world_normal += normalize( mul( normal2, tbn ) );
	world_normal = normalize( world_normal );
    world_normal = input.world_normal.xyz;

	float2 backBufferDim;
	tBackBuffer.GetDimensions( backBufferDim.x, backBufferDim.y );

	//Rotation Matricies for converting skybox to game basis
	float3x3 rotation_on_x = float3x3(
		float3(1.f,  0.f, 0.f),
		float3(0.f,  0.f, 1.f),
		float3(0.f, -1.f, 0.f));
	float3x3 rotation_on_z = float3x3(
		float3( 0.f, 1.f, 0.f),
		float3(-1.f, 0.f, 0.f),
		float3( 0.f, 0.f, 1.f));


	//Skybox Sample
	float3	incident	= normalize(input.world_position - CAMERA_POSITION);
	float3	reflection	= reflect(incident, world_normal);
			//reflection	= mul( inverseView, reflection );
			reflection	= mul(rotation_on_x, mul(rotation_on_z, reflection));
	float4	sky_color	= tSkybox.Sample(sSampler, reflection);
	sky_color = float4( sky, 1.f );


	float reflectivity;
	float cos_theta_incident = abs( dot( incident, world_normal ) );
	float theta_incident = acos( cos_theta_incident );
	float sin_theta_transmission = sin( theta_incident ) / nSnell;
	float theta_transmission = asin( sin_theta_transmission );
	if( theta_incident == 0.0f )
	{
		reflectivity = ( nSnell - 1 ) / ( nSnell + 1 );
		reflectivity = reflectivity * reflectivity;
	}
	else
	{
		float fs = sin( theta_transmission - theta_incident ) / sin( theta_transmission + theta_incident );
		float ts = tan( theta_transmission - theta_incident ) / tan( theta_transmission + theta_incident );
		reflectivity = 0.5 * ( fs * fs + ts * ts );
	}
	float transmissivity = 1.f - reflectivity;

	//Depth Sample ( real transmission/refractance )
	//float c2 = sqrt( 1.f - ( nSnell * nSnell * ( 1.f - cos_theta_incident ) * ( 1.f - cos_theta_incident ) ) );
	//float3 transmission_dir = ( nSnell * ( incident + world_normal * cos_theta_incident ) ) - ( c2 * world_normal );
	//transmission_dir = float3( 1.f, 0.f, 0.f );
	//float3 rayStartPos = mul( VIEW, float4( input.world_position.xyz, 1.f ) ).xyz;
    //float3 rotatedTransmissionDir = normalize(mul(PROJECTION, float4(transmission_dir, 0.f)).xyz);
	//float3 rayDir = normalize( mul( VIEW, float4( rotatedTransmissionDir, 0.f ) ).xyz );
	//return float4( GetColorFromNormalDir( rayDir ), 1.f );

    /*
	float2 halfBufferDim = backBufferDim * 0.5f;
	float2 rangeFraction = -1.f * backBufferDim * 0.5f;
	float4x4 rangeMap = float4x4(
		float4( halfBufferDim.x,	0.f,				0.f,	-rangeFraction.x ),
		float4( 0.f,				halfBufferDim.y,	0.f,	-rangeFraction.y ),
		float4( 0.f,				0.f,				1.f,	0.f ),
		float4( 0.f,				0.f,				0.f,	1.f )
		);

	float2 hitPixel;
	float3 hitPoint;
	int2 pixelCoord = int2(input.position.x, input.position.y);
	bool rayHit = traceScreenSpaceRay1(
		rayStartPos,
		rayDir,
		mul(rangeMap, PROJECTION),
		tDepthStencil,
		backBufferDim,
		1.f,//abs(FAR_PLANE - NEAR_PLANE),
		NEAR_PLANE,
		1.0f,
		0.f,//float((pixelCoord.x + pixelCoord.y) & 1) * 0.5f,
		150.f,
		100.f,
		hitPixel,
		hitPoint );
	//return float4(normalize(hitPoint), RangeMap(length(hitPoint), 0.f, 100.f, 0.f, 1.f));
	//rayHit = true;
	//hitPixel = input.position.xy;
	float3 floor_color;
    rayHit = true;
    hitPixel = input.position.xy;
	if( !rayHit )
	{
		floor_color = tSkybox.Sample( sSampler, rotatedTransmissionDir );
		floor_color *= float3(1.f, 0.2f, 0.2f);
	}
	else
	{
		floor_color = tBackBuffer.Load(int3(hitPixel, 0)).xyz;
		floor_color *= upwelling;
	}
*/
    
    float2 perturbedPixel;
    float3 floor_color = GetPerturbedColor(world_normal.xy, 25.f * cos_theta_incident, input.position.xy, tBackBuffer, backBufferDim, perturbedPixel);
	//floor_color *= upwelling;

	float backBufferSample = tDepthStencil.Load( int3( perturbedPixel, 0 ) ).x;
    float4 backBufferNDCPosition = float4( 0.f, 0.f, backBufferSample, 1.f );
    float3 backBufferCameraPosition = NDCToCameraSpace( backBufferNDCPosition, INVERSE_PROJECTION );
    float backBufferDepth = backBufferCameraPosition.x;
    float pixelDepth = input.camera_position.x;
	//backBufferDepth = -LinearizeDepth( backBufferDepth, NEAR_PLANE, FAR_PLANE );
    
    float refractionDepth = backBufferDepth - pixelDepth;
	float depthFraction = saturate( refractionDepth * waterFalloff );
    depthFraction = sqrt( sqrt( depthFraction ) );
    if( refractionDepth <= 0.1f && refractionDepth > 0.f )
    {
        return float4( 1.f, 1.f, 1.f, 0.75f );
    }
	floor_color = lerp( floor_color, upwelling, depthFraction.xxx );

    //Get Specular
    float3 dir_to_eye = normalize( CAMERA_POSITION - input.world_position );
    light_t light = LIGHTS[0];
    float3 light_color      = light.color.xyz;
    float3 light_position   = light.world_position;
    float3 light_direction  = normalize(light.direction);

    float3 pos_to_light             = light_position - input.world_position;
    float dist_to_light_position    = length(pos_to_light);
    float directional_distance      = dot(-pos_to_light, light_direction);

    float3 dir_to_light = lerp(normalize(pos_to_light), -light_direction, light.is_directional);
    float dist_to_light = lerp(dist_to_light_position, directional_distance, light.is_directional);

    float cos_angle_to_light_dir = dot(-dir_to_light, light_direction);

    float att_factor    = smoothstep(light.cos_outter_half_angle, light.cos_inner_half_angle, cos_angle_to_light_dir);
    float3 att_vec      = float3(1.0f, dist_to_light, dist_to_light * dist_to_light);
    float specular_att  = (light.intensity / dot(att_vec, light.spec_attenuation)) * att_factor;

    float facing        = smoothstep(-0.5, 0.0f, dot(dir_to_light, world_normal));
    float3 half_vector  = normalize(dir_to_light + dir_to_eye);
    float specular      = max(0.0f, dot(world_normal, half_vector));
    specular            = SPECULAR_FACTOR * pow(specular, SPECULAR_POWER);
    specular            *= specular_att;
    specular            *= facing;

    float3 specular_color = light_color * specular;
    
	//Water Color
    float3 reflection_color = reflectivity * sky_color.xyz;
    float3 transmission_color = transmissivity * floor_color;
	float3 water_color = reflection_color + transmission_color;
    water_color += specular_color;
	//return float4(water_color, 1.f);


	//Jacobian Foam
	//float turbulance = max(2.0f - input.jacobian.x + dot(0.3f * normalize(world_normal.xy), float2(1.2f, 1.2f)), 0.f);
	float foam_color = float3( 0.2f, 0.4f, 0.5f );
    float jacobianSign = sign( input.jacobian.x );
    float jacobainStep = sqrt( sqrt( saturate(input.jacobian.x) ) );
    //jacobainStep = abs(jacobainStep);// * jacobianSign;
    jacobainStep = saturate( jacobainStep );
    float foam_factor = smoothstep( 1.f, 0.f, jacobainStep );
    water_color += foam_color * foam_factor;
	return float4( water_color, 1.f);
}
