#ifdef SCREEN_QUAD

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

in vec2 vTexCoord;

uniform unsigned int targetBuffer;
uniform sampler2D gBufPosition;
uniform sampler2D gBufNormal;
uniform sampler2D gBufAlbedoSpec;
uniform sampler2D gBufDepth;
uniform sampler2D gBufDepthLinear;

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, vec3 normal, vec3 viewDir);
vec3 ComputePointLight(Light light, vec3 albedo, float specularC, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 fragPos = texture(gBufPosition, vTexCoord).rgb;
	vec3 normal = texture(gBufNormal, vTexCoord).rgb;
	vec3 albedo = texture(gBufAlbedoSpec, vTexCoord).rgb;
	float specularC = texture(gBufAlbedoSpec, vTexCoord).a;

	vec3 depth = texture(gBufDepth, vTexCoord).rgb;
	vec3 depthLinear = texture(gBufDepthLinear, vTexCoord).rgb;

	vec3 viewDir = normalize(uViewPos - fragPos);

	vec3 result = vec3(0.0);

	for(int i = 0; i < uNumLights; ++i)
	{
		if(uLights[i].type == 0)
			result += ComputeDirLight(uLights[i], albedo, specularC, normal, viewDir);
		else if(uLights[i].type == 1)
			result += ComputePointLight(uLights[i], albedo, specularC, normal, fragPos, viewDir);
	}
	
	switch(targetBuffer)
	{
		case 0: FragColor = vec4(result, 1.0); 			break; // Final Lighting
		case 1: FragColor = vec4(fragPos, 1.0); 		break; // Position
		case 2: FragColor = vec4(normal, 1.0); 			break; // Normal
		case 3: FragColor = vec4(albedo, 1.0); 			break; // Albedo
		case 4: FragColor = vec4(vec3(specularC), 1.0); break; // Specular
		case 5: FragColor = vec4(depth, 1.0); 			break; // Depth
		case 6: FragColor = vec4(depthLinear, 1.0); 	break; // Depth Linear
		default: FragColor = vec4(vec3(0.0), 1.0);
	}
}

vec3 ComputeDirLight(Light light, vec3 albedo, float specularC, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * albedo;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * albedo;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = light.specular * spec * specularC;

	return (ambient + diffuse + specular);
}

vec3 ComputePointLight(Light light, vec3 albedo, float specularC, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + 0.09 * distance + 0.032 * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient;
	ambient *= attenuation;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff;
	diffuse *= attenuation;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

	vec3 specular = light.specular * spec * specularC;
	specular *= attenuation;

	return (ambient + diffuse + specular) * albedo;
}

#endif /////////////////////////////////////////////////////////////////

#endif