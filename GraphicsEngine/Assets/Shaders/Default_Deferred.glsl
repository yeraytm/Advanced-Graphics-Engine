#ifdef DEFERRED_GEOMETRY_DEFAULT

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

layout(location = 0) out vec4 gBufPosition;
layout(location = 1) out vec4 gBufNormal;
layout(location = 2) out vec4 gBufAlbedo;
layout(location = 3) out vec4 gBufSpecular;
layout(location = 4) out vec4 gBufReflShini;

struct Material
{
	vec3 albedo;
	vec3 specular;
	vec3 reflective;
	float shininess;
};
uniform Material uMaterial;

in VS_OUT
{
	vec2 TexCoord;
	vec3 FragPos;
	vec3 Normal;
} fs_in;

void main()
{
	gBufPosition = vec4(fs_in.FragPos, 1.0);

	gBufNormal = vec4(fs_in.Normal, 1.0);

	gBufAlbedo = vec4(uMaterial.albedo, 1.0);

	gBufSpecular = vec4(uMaterial.specular, 1.0);

	gBufReflShini = vec4(uMaterial.reflective, uMaterial.shininess);
}

#endif /////////////////////////////////////////////////////////////////

#endif