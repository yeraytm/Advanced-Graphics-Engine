#ifdef DEFERRED_GEOMETRY_ALBEDO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

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
} vs_out;

void main()
{
	vs_out.TexCoord = aTexCoord;
	vs_out.FragPos = vec3(uModel * vec4(aPosition, 1.0));
	vs_out.Normal = normalize(vec3(uModel * vec4(aNormal, 0.0))); // As we will not perform non-uniform scale, we don't need a normal matrix for now

	gl_Position = uMVP * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec3 gBufPosition;
layout(location = 1) out vec3 gBufNormal;
layout(location = 2) out vec3 gBufAlbedo;
layout(location = 3) out vec3 gBufSpecular;
layout(location = 4) out vec3 gBufDepth;
layout(location = 5) out vec3 gBufDepthLinear;

struct Material
{
	sampler2D albedo;
	vec3 specular;
	float shininess;
};
uniform Material uMaterial;

in VS_OUT
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
} fs_in;

float near = 0.1;
float far = 100.0;
float LinearDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	gBufPosition = fs_in.FragPos;
	
	gBufNormal = fs_in.Normal;

	gBufAlbedo = texture(uMaterial.albedo, fs_in.TexCoord).rgb;
	gBufSpecular = uMaterial.specular;

	gBufDepth = vec3(gl_FragCoord.z);

	float depth = LinearDepth(gl_FragCoord.z) / far;
	gBufDepthLinear = vec3(depth);
}

#endif /////////////////////////////////////////////////////////////////

#endif