#pragma once

// By Morgan McGuire and Michael Mara at Williams College 2014
// Released as open source under the BSD 2-Clause License
// http://opensource.org/licenses/BSD-2-Clause

float distanceSquared( vec2 a, vec2 b ) { a -= b; return dot(a, a); }

// Returns true if the ray hit something
bool traceScreenSpaceRay1(
	float3 csOrig,
	float3 csDir,
	mat4x4 proj,
	sampler2D csZBuffer,
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
	// Clip to the near plane  
	float maxZPos = csOrig.z + csDir.z * maxDistance;
	float rayLength;
	if( maxZPos > nearPlaneZ )
	{
		rayLength = ( nearPlaneZ - csOrig.z ) / csDir.z
	}
	else
	{
		rayLength = maxDistance;
	}
	float3 csEndPoint = csOrig + csDir * rayLength;

	// Project into homogeneous clip space
	float4 clipSpaceStartPos = proj * vec4(csOrig, 1.0);
	float4 clipSpaceEndPos = proj * vec4(csEndPoint, 1.0);
	float k0 = 1.0 / clipSpaceStartPos.w;
	float k1 = 1.0 / clipSpaceEndPos.w;

	// The interpolated homogeneous version of the camera-space points  
	float3 homogenousStartPos = csOrig * k0;
	float3 homogenousEndPos = csEndPoint * k1;

	// Screen-space endpoints
	float2 screenSpaceStartPos = clipSpaceStartPos.xy * k0;
	float2 screenSpaceEndPos = clipSpaceEndPos.xy * k1;

	// If the line is degenerate, make it cover at least one pixel
	// to avoid handling zero-pixel extent as a special case later
	screenSpaceEndPos += vec2((distanceSquared(screenSpaceStartPos, screenSpaceEndPos) < 0.0001f) ? 0.01f : 0.0f);
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
	float3  dQ = (homogenousEndPos - homogenousStartPos) * invdx;
	float	dk = (k1 - k0) * invdx;
	float2  dP = float2(stepDir, delta.y * invdx);

	// Scale derivatives by the desired pixel stride and then
	// offset the starting values by the jitter fraction
	dP *= stride;
	dQ *= stride;
	dk *= stride;
	screenSpaceStartPos += dP * jitter;
	homogenousStartPos += dQ * jitter;
	k0 += dk * jitter;

	// Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
	float3 Q = homogenousStartPos;

	// Adjust end condition for iteration direction
	float  end = screenSpaceEndPos.x * stepDir;

	float k = k0;
	float stepCount = 0.0;
	float prevZMaxEstimate = csOrig.z;
	float rayZMin = prevZMaxEstimate;
	float rayZMax = prevZMaxEstimate;
	float sceneZMax = rayZMax + 100.f;
	for (float2 P = screenSpaceStartPos;
		((P.x * stepDir) <= end) && (stepCount < maxSteps) &&
		((rayZMax < sceneZMax - zThickness) || (rayZMin > sceneZMax)) &&
		(sceneZMax != 0);
		P += dP, Q.z += dQ.z, k += dk, ++stepCount) {

		rayZMin = prevZMaxEstimate;
		rayZMax = (dQ.z * 0.5f + Q.z) / (dk * 0.5f + k);
		prevZMaxEstimate = rayZMax;
		if (rayZMin > rayZMax) {
			float t = rayZMin;
			rayZMin = rayZMax;
			rayZMax = t;
		}

		hitPixel = permute ? P.yx : P;
		// You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
		// is different than ours in screen space
		sceneZMax = texelFetch(csZBuffer, int2(hitPixel), 0);
	}

	// Advance Q based on the number of steps
	Q.xy += dQ.xy * stepCount;
	hitPoint = Q * (1.0f / k);
	return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);
}