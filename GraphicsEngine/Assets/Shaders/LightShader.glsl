#ifdef LIGHT_CASTER

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(binding = 1, std140) uniform Matrices
{
	mat4 uModel;
	mat4 uMVP;
};

void main()
{
	gl_Position = uMVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 DepthColor;

void main()
{
	FragColor = vec4(1.0);
	DepthColor = vec4(1.0);
}

#endif /////////////////////////////////////////////////////////////////

#endif