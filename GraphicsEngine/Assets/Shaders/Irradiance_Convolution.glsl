#ifdef IRRADIANCE_CONVOLUTION

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPos;

out vec3 vWorldPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vWorldPos = aPos;
    gl_Position = uProjection * uView * vec4(aPos,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 FragColor;

in vec3 vWorldPos;

uniform samplerCube uEnvironmentMap;

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(vWorldPos);

    vec3 irradiance = vec3(0.0);

    // Calculate tangent space from origin point
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(uEnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
	FragColor = vec4(irradiance, 1.0);
}

#endif /////////////////////////////////////////////////////////////////

#endif