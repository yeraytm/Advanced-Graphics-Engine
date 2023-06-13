#ifdef FORWARD_ALBEDO_SPECULAR

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

struct Light
{
	vec4 lightVector; // XYZ for position/direction and W for type
	vec3 color;
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

out VS_OUT
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
	vec3 ViewDir;
} vs_out;

void main()
{
	vs_out.TexCoord = aTexCoord;
	vs_out.FragPos = vec3(uModel * vec4(aPosition, 1.0));
	vs_out.Normal = normalize(vec3(uModel * vec4(aNormal, 0.0))); // As we will not perform non-uniform scale, we don't need a normal matrix for now
	vs_out.ViewDir = normalize(uViewPos - vs_out.FragPos);

	gl_Position = uMVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

struct Light
{
	vec4 lightVector; // XYZ for position/direction and W for type
	vec3 color;
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
	vec3 reflective;
	float shininess;
};
uniform Material uMaterial;

in VS_OUT
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
	vec3 ViewDir;
} fs_in;

uniform samplerCube uEnvironmentMap;
uniform samplerCube uIrradianceMap;

struct RendererOptions
{
	bool uActiveIrradiance;
	bool uActiveReflection;
	bool uActiveRefraction;
};
uniform RendererOptions uRendererOptions;

vec3 ComputeDirLight(Light light, vec3 albedo, vec3 specular, vec3 irradiance, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 albedo, vec3 specular, vec3 irradiance, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 albedo = texture(uMaterial.albedo, fs_in.TexCoord).rgb;
	vec3 specularC = texture(uMaterial.specular, fs_in.TexCoord).rgb;
	
	vec3 irradiance = vec3(0.0);
	if(uRendererOptions.uActiveIrradiance)
		irradiance = texture(uIrradianceMap, fs_in.Normal).rgb;

	vec3 result = vec3(0.0);
	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].lightVector.w == 0.0)
			result += ComputeDirLight(uLights[i], albedo, specularC, irradiance, fs_in.Normal, fs_in.ViewDir);
		else if(uLights[i].lightVector.w == 1.0)
			result += ComputePointLight(uLights[i], albedo, specularC, irradiance, fs_in.Normal, fs_in.FragPos, fs_in.ViewDir);
	}

	if(uRendererOptions.uActiveReflection)
	{
		vec3 specularReflection = reflect(-fs_in.ViewDir, fs_in.Normal);
		result += texture(uEnvironmentMap, specularReflection).rgb * uMaterial.reflective;
	}

	if(uRendererOptions.uActiveRefraction)
	{
		vec3 refraction = refract(-fs_in.ViewDir, fs_in.Normal, 1.00/1.52);
		result += texture(uEnvironmentMap, refraction).rgb;
	}

	FragColor = vec4(result, 1.0);
}

vec3 ComputeDirLight(Light light, vec3 albedo, vec3 specularC, vec3 irradiance, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.lightVector.xyz);

	// Ambient
	vec3 ambient = albedo * irradiance;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * albedo;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
	vec3 specular = light.color * spec * specularC;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 albedo, vec3 specularC, vec3 irradiance, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightPosition = light.lightVector.xyz;
	vec3 lightDir = normalize(lightPosition - fragPos);
	
	float distance = length(lightPosition - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = albedo * attenuation * irradiance;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * albedo;
	diffuse *= attenuation;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
	vec3 specular = light.color * spec * specularC;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif