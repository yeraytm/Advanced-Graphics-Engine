#ifdef SSAO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;

    gl_Position = vec4(aPosition, 0.0, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out float FragColor;

uniform sampler2D gBufPosition;
uniform sampler2D gBufNormal;
uniform sampler2D noiseTexture;

uniform vec3 samples[64];
uniform mat4 projection;
uniform vec2 displaySize;

in vec2 vTexCoord;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
    vec2 noiseScale = vec2(displaySize / 4.0);

    vec3 fragPos = texture(gBufPosition, vTexCoord).rgb;
    vec3 normal = normalize(texture(gBufNormal, vTexCoord).rgb);
    vec3 noise = normalize(texture(noiseTexture, vTexCoord * noiseScale).rgb);

    vec3 tangent = normalize(noise - normal * dot(noise, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Iterate over each sample
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; //from view to clip-space
        offset.xyz /= offset.w; //perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; //transform to range 0.0 - 1.0

        float sampleDepth = texture(gBufPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    FragColor = 1.0 - (occlusion / float(kernelSize));
}

#endif /////////////////////////////////////////////////////////////////

#endif