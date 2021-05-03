struct vs_input_ocean_t
{
	// we are not defining our own input data; 
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;

	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float3 normal : NORMAL;

	float4 jacobian : JACOBIAN;
};


struct v2f_ocean_t
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;

	float3 camera_position : POSITION0;
	float3 world_position : POSITION1;
	float3 world_normal : NORMAL;
	float3 world_tangent : TANGENT;
	float3 world_bitangent : BITANGENT;

	float4 jacobian : JACOBIAN;
};


cbuffer ocean_constants : register(b5)
{
	float2 NORMALS1_SCROLL_DIR;
	float2 NORMALS2_SCROLL_DIR;
    
	float2 NORMALS_SCROLL_SPEED;
	float NEAR_PLANE;
	float FAR_PLANE;

	float3 UPWELLING_COLOR;
	float SNELL;
    
	float MAX_DEPTH;
	float INVERSE_MAX_DEPTH;
	float FOAM_THICKNESS;
    float FOAMINESS;
    
    float4x4 WORLD_INVERSE_PROJECTION;
};


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
	hitPixel = float2(-1.f, -1.f);
	hitPoint = float3(0.f, 0.f, 0.f);
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
	float4 clipSpaceStartPos = mul(proj, float4(csOrig, 1.0));
	float4 clipSpaceEndPos = mul(proj, float4(csEndPoint, 1.0));
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
	screenSpaceEndPos += ((distanceSquared(screenSpaceStartPos, screenSpaceEndPos) < 0.0001f) ? float2(0.01f, 0.01f) : float2(0.0f, 0.0f));
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
		sceneZMax = csZBuffer.Load(int3(hitPixel, 0)).x;
		sceneZMax = LinearizeDepth(sceneZMax, nearPlaneZ, 100.f);
	}

	// Advance Q based on the number of steps
	homogenousPos.xy += dHomogenous.xy * stepCount;
	hitPoint = homogenousPos * (1.0f / k);
	//return float4(screenSpaceStartPos / csZBufferSize, 0.f, 1.f);
	return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);
}


//--------------------------------------------------------------------------------------
float3 GetPerturbedColor(float2 dirToPerturb, float perturbationFactor, float2 pixelPosition, Texture2D backBuffer, Texture2D refractionStencil, float2 backBufferDim, out float2 perturbedPixel)
{
	float2 offset = dirToPerturb * perturbationFactor;
	float2 potentialPerturbedPixel = pixelPosition + offset;
	float4 stencilColor = refractionStencil.Load(int3(potentialPerturbedPixel, 0));
	if (stencilColor.x == 0.f)
	{
		perturbedPixel = potentialPerturbedPixel;
	}
	else if (stencilColor.x == 1.f)
	{
		perturbedPixel = pixelPosition;
	}

	perturbedPixel = clamp(perturbedPixel, float2(0.f, 0.f), backBufferDim - float2(1.f, 1.f));
	return backBuffer.Load(int3(perturbedPixel, 0));

	//Creates artifacts if too close to objects
//  float2 perturbedPixelUV = saturate( perturbedPixel / backBufferDim );
//  perturbedPixel = perturbedPixelUV * ( backBufferDim - float2( 1.f, 1.f ) );
//  return backBuffer.Sample( sSampler, perturbedPixelUV );
}


//---------------------------------------------------------------------------------------------------------
float GetWaterFallOffForDepth( float depth, float inverseMaxDepth )
{
	float depthFraction = saturate( depth * inverseMaxDepth );
	depthFraction = sqrt( sqrt( depthFraction ) );
	return depthFraction;
}