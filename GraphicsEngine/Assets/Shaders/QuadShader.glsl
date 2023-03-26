// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see below).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.

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

uniform sampler2D u_Texture;

void main()
{
	vec4 texColor = texture(u_Texture, TexCoord);
	FragColor = texColor;
}

#endif /////////////////////////////////////////////////////////////////

#endif