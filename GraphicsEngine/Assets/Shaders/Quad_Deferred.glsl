#ifdef SCREEN_QUAD

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

layout(location = 0) out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D uRenderTarget;

void main()
{
	FragColor = texture(uRenderTarget, vTexCoord);
}

#endif /////////////////////////////////////////////////////////////////

#endif