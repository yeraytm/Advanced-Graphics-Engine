//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"

#include "glad/glad.h"

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Vertex
{
    vec3 position;
    vec2 uv;
};

struct Image
{
    void* pixels;
    ivec2 size;
    int nchannels;
    int stride;
};

struct Texture
{
    u32 handle;
    std::string filepath;
};

struct ShaderProgram
{
    u32 handle;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp; // What is this for?
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct GLInfo
{
    bool openGLStatus;
    const char* version;
    const char* vendor;
    const char* renderer;
    const char* glslVersion;
    std::string extensions;
};

struct App
{
    // Loop
    f32 deltaTime;
    bool isRunning;

    // Input
    Input input;

    GLInfo glInfo;

    ivec2 displaySize;

    std::vector<Texture> textures;
    std::vector<ShaderProgram> shaderPrograms;

    // Program indices
    u32 texturedGeometryProgramIdx;

    // Texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    u32 VBO;
    u32 EBO;

    // Location of the texture uniform in the textured quad shader
    u32 programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    u32 VAO;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);