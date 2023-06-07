#ifdef SKYBOX

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPos;

out vec3 vTexCoord;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vTexCoord = aPos;
    vec4 pos = uProjection * uView * vec4(aPos,1.0);
    gl_Position = pos.xyww;
} 

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec3 vTexCoord;

uniform samplerCube uSkybox;

void main()
{
    vec3 envColor = texture(uSkybox, vTexCoord).rgb;

    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

	FragColor = vec4(envColor, 1.0);
}

#endif /////////////////////////////////////////////////////////////////

#endif