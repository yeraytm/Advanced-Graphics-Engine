// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see below).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.

#ifdef TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

out vec2 uTexCoord;

//uniform mat4 uModel;
//uniform mat4 uView;
//uniform mat4 uProjection;

layout(binding = 1, std140) uniform Matrices
{
	mat4 uModel;
	mat4 uMVP;
};

void main()
{
	uTexCoord = aTexCoord;

	gl_Position = uMVP * vec4(aPosition, 1.0);
	//gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec2 uTexCoord;

uniform sampler2D uTexture;

void main()
{
	vec4 texColor = texture(uTexture, uTexCoord);
	FragColor = texColor;
}

#endif /////////////////////////////////////////////////////////////////

#endif