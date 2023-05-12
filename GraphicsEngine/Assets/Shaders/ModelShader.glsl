#ifdef MODEL_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

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
layout(location = 1) out vec4 DepthColor;
layout(location = 2) out vec4 NormalsColor;

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
	vec3 specular;
	float shininess;
};
uniform Material uMaterial;

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vViewDir;

struct LightMap
{
	vec3 diffuse;
} lightMap;

vec3 ComputeDirLight(Light light, LightMap lightMap, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, LightMap lightMap, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1;
float far = 100.0;
float LinearDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	lightMap.diffuse = vec3(texture(uMaterial.albedo, vTexCoord));

	vec3 result = vec3(0.0);

	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].type == 0)
			result += ComputeDirLight(uLights[i], lightMap, vNormal, vViewDir);
		else if(uLights[i].type == 1)
			result += ComputePointLight(uLights[i], lightMap, vNormal, vFragPos, vViewDir);
	}
	
	FragColor = vec4(result, 1.0);

	float depth = LinearDepth(gl_FragCoord.z) / far;
	DepthColor = vec4(vec3(depth), 1.0);
	NormalsColor = vec4(vNormal, 1.0);
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

	// Temporary fix because if shininess is 0 wrong specularity values appear
	float shininess = uMaterial.shininess > 0.0 ? uMaterial.shininess : 32.0;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

	vec3 specular = light.specular * spec * uMaterial.specular;

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

	float shininess = uMaterial.shininess > 0.0 ? uMaterial.shininess : 32.0; // Temporary fix because if shininess is 0.0, wrong specularity values appear
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

	vec3 specular = light.specular * spec * uMaterial.specular;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif