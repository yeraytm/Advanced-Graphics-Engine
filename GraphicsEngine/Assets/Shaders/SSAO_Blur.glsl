#ifdef SSAO_BLUR

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

uniform sampler2D uSSAOColor;
uniform int uNoiseSize;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(uSSAOColor, 0));

    int arraySize = int(sqrt(float(uNoiseSize))/2);

    float result = 0.0;
    for(int x = -arraySize; x < arraySize; ++x)
    {
        for(int y = -arraySize; y < arraySize; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(uSSAOColor, vTexCoord + offset).r;
        }
    }
    FragColor = result / float(uNoiseSize);
}
#endif /////////////////////////////////////////////////////////////////

#endif