#ifdef EQUIRECT_TO_CUBEMAP

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPos;

out vec3 vWorldPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vWorldPos = aPos;
    gl_Position = uProjection * uView * vec4(aPos,1.0);
} 

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec3 vWorldPos;

uniform sampler2D uEquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vWorldPos));
    vec3 color = min(vec3(1000.0), texture(uEquirectangularMap, uv).rgb);
	FragColor = vec4(color, 1.0);
}

#endif /////////////////////////////////////////////////////////////////

#endif