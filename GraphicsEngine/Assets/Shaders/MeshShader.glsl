#ifdef TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(binding = 1, std140) uniform Matrices
{
	mat4 uModel;
	mat4 uMVP;
};

out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vNormal;

void main()
{
	vTexCoord = aTexCoord;
	vFragPos = vec3(uModel * vec4(aPosition, 1.0));
	vNormal = vec3(uModel * vec4(aNormal, 0.0)); // As we will not perform non-uniform scale, we don't need a normal matrix for now

	gl_Position = uMVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

#define NUM_MAX_LIGHTS 2
struct Light
{
	int type;

	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light lights[NUM_MAX_LIGHTS];

struct Material
{
	sampler2D albedo;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material material;

uniform vec3 uViewPos;

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;

vec3 ComputeDirLight(Light light, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	// Direction Vectors
	vec3 norm = normalize(vNormal);
	vec3 viewDir = normalize(uViewPos - vFragPos);

	// Ambient
	//vec3 ambient = light.ambient * material.ambient;
	//vec3 ambient = light.ambient * vec3(texture(material.diffuse, vTexCoord));

	// Diffuse
	//vec3 diffuse = light.diffuse * diff * material.diffuse;
	//vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vTexCoord));

	// Specular
	//vec3 specular = light.specular * spec * material.specular;
	//vec3 specular = light.specular * spec * vec3(texture(material.specular, vTexCoord));

	vec3 result = vec3(0.0);

	for(int i = 0; i < NUM_MAX_LIGHTS; ++i)
	{
		if(lights[i].type == 0)
			result += ComputeDirLight(lights[i], norm, viewDir);
		else if(lights[i].type == 1)
			result += ComputePointLight(lights[i], norm, vFragPos, viewDir);
	}

	FragColor = vec4(result, 1.0);
}

vec3 ComputeDirLight(Light light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * vec3(texture(material.albedo, vTexCoord));

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.albedo, vTexCoord));

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * material.specular;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient * vec3(texture(material.albedo, vTexCoord));
	ambient *= attenuation;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.albedo, vTexCoord));
	diffuse *= attenuation;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * material.specular;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif