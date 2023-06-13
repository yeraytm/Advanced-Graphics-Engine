#ifdef DEFERRED_LIGHTING_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;

	gl_Position = vec4(aPosition, 0.0, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FinalColor;

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

in vec2 vTexCoord;

uniform sampler2D gBufPosition;
uniform sampler2D gBufNormal;
uniform sampler2D gBufAlbedo;
uniform sampler2D gBufSpecular;
uniform sampler2D gBufReflShini;

uniform samplerCube uEnvironmentMap;
uniform samplerCube uIrradianceMap;
uniform sampler2D uSSAOColor;

struct RendererOptions
{
	bool uActiveIrradiance;
	bool uActiveReflection;
	bool uActiveRefraction;
	bool uActiveSSAO;
};
uniform RendererOptions uRendererOptions;

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, float shininess, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 albedo, float specularC,  float shininess, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 fragPos = texture(gBufPosition, vTexCoord).rgb;

	vec3 normal = texture(gBufNormal, vTexCoord).rgb;

	vec3 albedo = texture(gBufAlbedo, vTexCoord).rgb;

	float specularC = texture(gBufSpecular, vTexCoord).r;

	// Reflective + Shininess G-Buffer color texture
	vec4 reflectiveShininess = texture(gBufReflShini, vTexCoord);
	vec3 reflective = reflectiveShininess.rgb;
	float shininess = reflectiveShininess.a * 256.0;

	vec3 irradiance = vec3(0.0);
	if(uRendererOptions.uActiveIrradiance)
		irradiance = texture(uIrradianceMap, normal).rgb;

	float ambientOcclusion = 1.0;
	if(uRendererOptions.uActiveSSAO)
		ambientOcclusion = texture(uSSAOColor, vTexCoord).r;

	vec3 viewDir = normalize(uViewPos - fragPos);

	// Ambient
	vec3 result = albedo * ambientOcclusion * irradiance;
	
	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].lightVector.w == 0.0)
			result += ComputeDirLight(uLights[i], albedo, specularC, shininess, normal, viewDir);
		else if(uLights[i].lightVector.w == 1.0)
			result += ComputePointLight(uLights[i], albedo, specularC, shininess, normal, fragPos, viewDir);
	}

	if(uRendererOptions.uActiveReflection)
	{
		vec3 specularReflection = reflect(-viewDir, normal);
		result += texture(uEnvironmentMap, specularReflection).rgb * reflective;
	}

	if(uRendererOptions.uActiveRefraction)
	{
		vec3 refraction = refract(-viewDir, normal, 1.00/1.52);
		result += texture(uEnvironmentMap, refraction).rgb;
	}

	// Final Lighting Color write to G-Buffer
	FinalColor = vec4(result, 1.0);
}

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, float shininess, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.lightVector.xyz);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * albedo;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular = light.color * spec * specularC;

	return (diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 albedo, float specularC, float shininess, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightPosition = light.lightVector.xyz;
	vec3 lightDir = normalize(lightPosition - fragPos);

	float distance = length(lightPosition - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * albedo;
	diffuse *= attenuation;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular = light.color * spec * specularC;
	specular *= attenuation;

	return (diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif