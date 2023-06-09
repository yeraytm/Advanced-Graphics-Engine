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

in vec2 vTexCoord;

uniform sampler2D gBufPosition;
uniform sampler2D gBufNormal;
uniform sampler2D gBufDepth;

uniform sampler2D uNoiseTexture;
uniform vec3 uSamples[64];
uniform mat4 uProjection;
uniform mat4 uView;
uniform vec2 uDisplaySize;

struct SSAOptions
{
    bool uRangeCheck;
    float uRadius;
    float uBias;
    float uPower;
    int uKernelSize;
};
uniform SSAOptions uSSAOptions;

vec3 ReconstructPixelPos(float depth)
{
    float xndc = gl_FragCoord.x / uDisplaySize.x * 2.0 - 1.0;
    float yndc = gl_FragCoord.y / uDisplaySize.y * 2.0 - 1.0;
    float zndc = depth * 2.0 - 1.0;
    vec4 posNDC = vec4(xndc, yndc, zndc, 1.0);
    vec4 posView = inverse(uProjection) * posNDC;
    return posView.xyz / posView.w;
}

void main()
{
    vec2 noiseScale = uDisplaySize / textureSize(uNoiseTexture, 0);

    vec4 fragPosView = uView * vec4(texture(gBufPosition, vTexCoord).rgb, 1.0);
    vec3 normalView = mat3(uView) * texture(gBufNormal, vTexCoord).rgb;
    vec3 noise = texture(uNoiseTexture, vTexCoord * noiseScale).rgb;

    vec3 tangent = normalize(noise - normalView * dot(noise, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalView);

    // Iterate over each sample
    float occlusion = 0.0;
    for(int i = 0; i < uSSAOptions.uKernelSize; ++i)
    {
        vec3 offsetView = TBN * uSamples[i];
        vec3 samplePosView = fragPosView.xyz + offsetView * uSSAOptions.uRadius;

        vec4 sampleTexCoord = uProjection * vec4(samplePosView, 1.0);
        sampleTexCoord.xyz /= sampleTexCoord.w;
        sampleTexCoord.xyz = sampleTexCoord.xyz * 0.5 + 0.5;

        float sampledDepth = texture(gBufDepth, sampleTexCoord.xy).r;
        vec3 sampledPosView = ReconstructPixelPos(sampledDepth);

        float rangeCheck = 1.0;
        if(uSSAOptions.uRangeCheck)
        {
            rangeCheck = smoothstep(0.0, 1.0, uSSAOptions.uRadius / abs(samplePosView.z - sampledPosView.z));
            rangeCheck *= rangeCheck;
        }

        occlusion += (samplePosView.z < sampledPosView.z - uSSAOptions.uBias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(uSSAOptions.uKernelSize));
    FragColor = pow(occlusion, uSSAOptions.uPower);
}

#endif /////////////////////////////////////////////////////////////////

#endif