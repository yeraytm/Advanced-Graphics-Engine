#ifdef TEXTURED_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

float near = 0.1;
float far = 100.0;
float LinearDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

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

	float depth = LinearDepth(gl_FragCoord.z) / far;
	FragColor = vec4(vec3(depth), 1.0);
}

#endif /////////////////////////////////////////////////////////////////

#endif