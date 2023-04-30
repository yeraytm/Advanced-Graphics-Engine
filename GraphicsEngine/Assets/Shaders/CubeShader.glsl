#ifdef CUBE_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(binding = 1, std140) uniform Matrices
{
	mat4 uModel;
	mat4 uMVP;
};

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

void main()
{
	gl_Position = uMVP * vec4(aPosition, 1.0);
	TexCoord = aTexCoord;
	FragPos = vec3(uModel * vec4(aPosition, 1.0));
	
	// As we will not perform non-uniform scale, we don't need a normal matrix for now
	Normal = vec3(uModel * vec4(aNormal, 0.0));
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

uniform vec3 uObjColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D uTexture;

void main()
{
	float ambientCoef = 0.1;
	vec3 ambient = ambientCoef * uLightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(uLightPos - FragPos);
	vec3 viewDir = normalize(uViewPos - FragPos);

	float diffuseCoef = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffuseCoef * uLightColor;

	float specularCoef = 0.5;
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularCoef * spec * uLightColor;

	vec3 result = (ambient + diffuse + specular) * uObjColor;

	FragColor = vec4(result, 1.0);
	
	//vec4 texColor = texture(uTexture, TexCoord);
	//FragColor = texColor;
}

#endif /////////////////////////////////////////////////////////////////

#endif