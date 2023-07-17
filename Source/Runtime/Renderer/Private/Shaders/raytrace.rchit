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
	// calculate the distance of hit
	payload.distanceAlongTheRay = gl_HitTEXT;
	payload.color = vec3(1.0, 1.0, 1.0);
	payload.didHit = true;
}

