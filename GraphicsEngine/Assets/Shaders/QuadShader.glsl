#ifdef SCREEN_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	TexCoord = aTexCoord;

	gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uScreenTexture;

void main()
{
	FragColor = texture(uScreenTexture, TexCoord);
}

#endif /////////////////////////////////////////////////////////////////

#endif