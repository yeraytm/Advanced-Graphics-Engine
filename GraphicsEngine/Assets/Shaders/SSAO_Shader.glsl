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
uniform sampler2D gBufDepth;
uniform sampler2D noiseTexture;

uniform vec3 samples[64];
uniform mat4 projection;
uniform mat4 view;
uniform vec2 displaySize;

in vec2 vTexCoord;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
    vec2 noiseScale = displaySize / textureSize(noiseTexture, 0);

    vec4 fragPosView = view * vec4(texture(gBufPosition, vTexCoord).rgb, 1.0);
    vec3 normalView = mat3(view) * texture(gBufNormal, vTexCoord).rgb;
    vec3 noise = texture(noiseTexture, vTexCoord * noiseScale).rgb;

    vec3 tangent = normalize(noise - normalView * dot(noise, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalView);

    // Iterate over each sample
    float occlusion = 0.0;

    // for(int i = 0; i < kernelSize; ++i)
    // {
    //     vec3 samplePos = TBN * samples[i];
    //     samplePos = fragPos + samplePos * radius;

    //     vec4 offset = vec4(samplePos, 1.0);
    //     offset = projection * offset; //from view to clip-space
    //     offset.xyz /= offset.w; //perspective divide
    //     offset.xyz = offset.xyz * 0.5 + 0.5; //transform to range 0.0 - 1.0

    //     float sampleDepth = vec3(view * texture(gBufPosition, offset.xy)).z;
    //     float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
    //     occlusion += (sampleDepth <= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    // }

    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 offsetView = TBN * samples[i];
        vec3 samplePosView = fragPosView.xyz + offsetView * radius;

        vec4 sampleTexCoord = projection * vec4(samplePosView, 1.0);
        sampleTexCoord.xyz /= sampleTexCoord.w;
        sampleTexCoord.xyz = sampleTexCoord.xyz * 0.5 + 0.5;

        float sampledDepth = texture(gBufDepth, sampleTexCoord.xy).r;
        //----func----
        float xndc = gl_FragCoord.x / displaySize.x * 2.0 - 1.0;
        float yndc = gl_FragCoord.y / displaySize.y * 2.0 - 1.0;
        float zndc = sampledDepth * 2.0 - 1.0;
        vec4 posNDC = vec4(xndc, yndc, zndc, 1.0);
        vec4 posView = inverse(projection) * posNDC;
        vec3 result = posView.xyz / posView.w;
        //------------
        vec3 sampledPosView = result;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(samplePosView.z - sampledPosView.z));
        rangeCheck *= rangeCheck;

        occlusion += (samplePosView.z < sampledPosView.z - 0.02 ? 1.0 : 0.0);
    }

    FragColor = 1.0 - (occlusion / float(kernelSize));
}

#endif /////////////////////////////////////////////////////////////////

#endif