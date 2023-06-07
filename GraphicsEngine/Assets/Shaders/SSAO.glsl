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

uniform float uRadius = 0.5;
uniform float uBias = 0.025;
uniform float uPower = 1.0;

const int kernelSize = 64;

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
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 offsetView = TBN * uSamples[i];
        vec3 samplePosView = fragPosView.xyz + offsetView * uRadius;

        vec4 sampleTexCoord = uProjection * vec4(samplePosView, 1.0);
        sampleTexCoord.xyz /= sampleTexCoord.w;
        sampleTexCoord.xyz = sampleTexCoord.xyz * 0.5 + 0.5;

        float sampledDepth = texture(gBufDepth, sampleTexCoord.xy).r;
        vec3 sampledPosView = ReconstructPixelPos(sampledDepth);

        float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(samplePosView.z - sampledPosView.z));
        rangeCheck *= rangeCheck;

        occlusion += (samplePosView.z < sampledPosView.z - uBias ? 1.0 : 0.0);
    }

    occlusion = 1.0 - (occlusion / float(kernelSize));
    FragColor = pow(occlusion, uPower);
}

#endif /////////////////////////////////////////////////////////////////

#endif