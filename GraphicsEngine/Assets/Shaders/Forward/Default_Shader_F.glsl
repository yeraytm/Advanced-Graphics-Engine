#ifdef DEFAULT

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

// THE TARGETS NEED TO BE SET FOR FORWARD SHADING
layout(location = 0) out vec3 gBufResult;
layout(location = 1) out vec3 gBufPosition;
layout(location = 2) out vec3 gBufNormal;
layout(location = 3) out vec3 gBufDepthLinear;

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
	vec3 albedo;
	vec3 specular;
	float shininess;
};
uniform Material uMaterial;

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vViewDir;

vec3 ComputeDirLight(Light light, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1;
float far = 100.0;
float LinearDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	vec3 result = vec3(0.0);

	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].type == 0)
			result += ComputeDirLight(uLights[i], vNormal, vViewDir);
		else if(uLights[i].type == 1)
			result += ComputePointLight(uLights[i], vNormal, vFragPos, vViewDir);
	}

	gBufResult = result;

	gBufPosition = vFragPos;

	gBufNormal = vNormal;

	float depth = LinearDepth(gl_FragCoord.z) / far;
	gBufDepthLinear = vec3(depth);
}

vec3 ComputeDirLight(Light light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * uMaterial.albedo;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * uMaterial.albedo;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);
	vec3 specular = light.specular * spec * uMaterial.specular;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient * uMaterial.albedo;
	ambient *= attenuation;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * uMaterial.albedo;
	diffuse *= attenuation;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);
	vec3 specular = light.specular * spec * uMaterial.specular;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif