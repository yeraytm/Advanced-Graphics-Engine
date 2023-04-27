#ifdef TEXTURED_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;

void main()
{
	vec4 texColor = texture(uTexture, TexCoord);
	FragColor = texColor;
}

#endif /////////////////////////////////////////////////////////////////

#endif