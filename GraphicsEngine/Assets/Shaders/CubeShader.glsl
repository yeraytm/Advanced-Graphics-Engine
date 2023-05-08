#ifdef CUBE_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

struct Light
{
	unsigned int type;

	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
};

layout(binding = 0, std140) uniform GlobalParameters
{
	vec3 uViewPos;
	unsigned int uNumLights;
	Light uLights[16];
};

layout(binding = 1, std140) uniform LocalParameters
{
	mat4 uModel;
	mat4 uMVP;
};

out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vNormal;
out vec3 vViewDir;

void main()
{
	vTexCoord = aTexCoord;
	vFragPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = normalize(vec3(uModel * vec4(aNormal, 0.0))); // As we will not perform non-uniform scale, we don't need a normal matrix for now
	vViewDir = normalize(uViewPos - vFragPos);

	gl_Position = uMVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

struct Light
{
	unsigned int type;

	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
};

layout(binding = 0, std140) uniform GlobalParameters
{
	vec3 uViewPos;
	unsigned int uNumLights;
	Light uLights[16];
};

struct Material
{
	sampler2D albedo;
	sampler2D specular;
	float shininess;
};
uniform Material material;

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vViewDir;

struct LightMap
{
	vec3 diffuse;
	vec3 specular;
} lightMap;

vec3 ComputeDirLight(Light light, LightMap lightMap, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, LightMap lightMap, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	lightMap.diffuse = vec3(texture(material.albedo, vTexCoord));
	lightMap.specular = vec3(texture(material.specular, vTexCoord));

	vec3 result = vec3(0.0);

	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].type == 0)
			result += ComputeDirLight(uLights[i], lightMap, vNormal, vViewDir);
		else if(uLights[i].type == 1)
			result += ComputePointLight(uLights[i], lightMap, vNormal, vFragPos, vViewDir);
	}

	FragColor = vec4(result, 1.0);
}

vec3 ComputeDirLight(Light light, LightMap lightMap, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * lightMap.diffuse;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * lightMap.diffuse;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * lightMap.specular;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, LightMap lightMap, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient * lightMap.diffuse;
	ambient *= attenuation;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * lightMap.diffuse;
	diffuse *= attenuation;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * lightMap.specular;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif