#ifdef SKYBOX

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos,1.0);
    gl_Position = pos.xyww;
} 

#elif defined(FRAGMENT) ///////////////////////////////////////////////

out vec4 FragColor;
in vec3 textureDir;
uniform samplerCube skybox;

void main()
{
	FragColor = texture(skybox,textureDir);
}

#endif /////////////////////////////////////////////////////////////////

#endif