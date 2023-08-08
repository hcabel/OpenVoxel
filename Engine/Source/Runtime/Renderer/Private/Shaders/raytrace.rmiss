#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload
{
	vec3 color;
	float distanceAlongTheRay;
	bool didHit;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main()
{
	vec3 origin    = gl_WorldRayOriginEXT;
	vec3 direction = gl_WorldRayDirectionEXT; // [-1, 1]

	vec3 rayEndPosition = origin + 100.0 * direction;

	vec3 rayPositionNormal = normalize(rayEndPosition);

	payload.color = rayPositionNormal;
	payload.didHit = false;
}