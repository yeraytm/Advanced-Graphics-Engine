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

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(uSSAOColor, 0));
    float result = 0.0;

    for(int x = -2; x < 2; ++x)
    {
        for(int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(uSSAOColor, vTexCoord + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}
#endif /////////////////////////////////////////////////////////////////

#endif