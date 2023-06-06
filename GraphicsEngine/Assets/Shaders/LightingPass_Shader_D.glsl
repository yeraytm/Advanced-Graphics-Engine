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
	// XYZ for position/direction and W for type
	vec4 lightVector;

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

in vec2 vTexCoord;

uniform sampler2D gBufPosition;
uniform sampler2D gBufNormal;
uniform sampler2D gBufAlbedo;
uniform sampler2D gBufSpecular;

uniform sampler2D ssaoColor;
//uniform samplerCube skybox;

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, float ambientOcclusion, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 albedo, float specularC, float ambientOcclusion, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 fragPos = texture(gBufPosition, vTexCoord).rgb;
	vec3 normal = texture(gBufNormal, vTexCoord).rgb;
	vec3 albedo = texture(gBufAlbedo, vTexCoord).rgb;
	float specularC = texture(gBufSpecular, vTexCoord).r;
	
	float ambientOcclusion = texture(ssaoColor, vTexCoord).r;

	vec3 viewDir = normalize(uViewPos - fragPos);

	vec3 result = vec3(0.0);

	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].lightVector.w == 0.0)
			result += ComputeDirLight(uLights[i], albedo, specularC, ambientOcclusion, normal, viewDir);
		else if(uLights[i].lightVector.w == 1.0)
			result += ComputePointLight(uLights[i], albedo, specularC, ambientOcclusion, normal, fragPos, viewDir);
	}

	//vec3 specularReflection = reflect(-viewDir, normalize(normal));
	//vec3 refr = refract(-viewDir, normalize(normal), 1.00/1.52);

	//result += texture(skybox, specularReflection).rgb;

	// Final Lighting Color write to G-Buffer
	FinalColor = vec4(result, 1.0);
}

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, float ambientOcclusion, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.lightVector.xyz);

	// Ambient
	vec3 ambient = light.ambient * albedo * ambientOcclusion;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * albedo;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = light.specular * spec * specularC;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 albedo, float specularC, float ambientOcclusion, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightPosition = light.lightVector.xyz;
	vec3 lightDir = normalize(lightPosition - fragPos);

	float distance = length(lightPosition - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient * albedo * attenuation * ambientOcclusion;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * albedo;
	diffuse *= attenuation;

	// Specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = light.specular * spec * specularC;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

#endif /////////////////////////////////////////////////////////////////

#endif