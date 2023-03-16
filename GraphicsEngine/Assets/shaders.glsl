
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
layout(location = 1) in vec2 texCoord;
//layout(location = 3) in vec3 tangent;
//layout(location = 4) in vec3 bitangent;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = texCoord;

	//float clippingScale = 5.0;
	//gl_Position = vec4(position, clippingScale);
	//gl_Position.z = -gl_Position.z;

	gl_Position = vec4(position,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	vec4 texColor = texture(u_Texture, v_TexCoord);
	color = texColor;
}

#endif /////////////////////////////////////////////////////////////////

#endif

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.