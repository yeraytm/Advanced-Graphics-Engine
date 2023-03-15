//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#define ASSERT(x) if (!(x)) __debugbreak();

#ifdef _DEBUG
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x
#endif

GLuint CreateProgramFromSource(std::string programSource, const char* shaderName)
{
    GLchar infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.c_str()
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.length()
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.c_str()
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.length()
    };

    GLCall(GLuint vshader = glCreateShader(GL_VERTEX_SHADER));
    GLCall(glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths));
    GLCall(glCompileShader(vshader));
    GLCall(glGetShaderiv(vshader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        GLCall(glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer));
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLCall(GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER));
    GLCall(glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths));
    GLCall(glCompileShader(fshader));
    GLCall(glGetShaderiv(fshader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        GLCall(glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer));
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLCall(GLuint programHandle = glCreateProgram());

    GLCall(glAttachShader(programHandle, vshader));
    GLCall(glAttachShader(programHandle, fshader));

    GLCall(glLinkProgram(programHandle));

    GLCall(glGetProgramiv(programHandle, GL_LINK_STATUS, &success));
    if (!success)
    {
        GLCall(glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer));
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLCall(glUseProgram(0));

    GLCall(glDetachShader(programHandle, vshader));
    GLCall(glDetachShader(programHandle, fshader));

    GLCall(glDeleteShader(vshader));
    GLCall(glDeleteShader(fshader));

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    std::string programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
        img.stride = img.size.x * img.nchannels;
    else
        ELOG("Could not open file %s", filename);

    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    GLenum dataType = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3:
            dataFormat = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            dataFormat = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    GLCall(glGenTextures(1, &texHandle));
    GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

    //GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(glGenerateMipmap(GL_TEXTURE_2D));

    GLCall(glBindTexture(GL_TEXTURE_2D, 0));

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void Init(App* app)
{
    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures

    app->glInfo.version = (const char*)glGetString(GL_VERSION);
    app->glInfo.renderer = (const char*)glGetString(GL_RENDERER);
    app->glInfo.vendor = (const char*)glGetString(GL_VENDOR);
    app->glInfo.glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    const Vertex vertices[] = {
        { vec3(-0.5, -0.5, 0.0),  vec2(0.0, 0.0) },
        { vec3( 0.5, -0.5, 0.0),  vec2(1.0, 0.0) },
        { vec3( 0.5,  0.5, 0.0),  vec2(1.0, 1.0) },
        { vec3(-0.5,  0.5, 0.0),  vec2(0.0, 1.0) }
    };

    const u16 indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLCall(glGenBuffers(1, &app->embeddedVertices));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLCall(glGenBuffers(1, &app->embeddedElements));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    GLCall(glGenVertexArrays(1, &app->vao));
    GLCall(glBindVertexArray(app->vao));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0));
    GLCall(glEnableVertexAttribArray(0));

    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)12));
    GLCall(glEnableVertexAttribArray(1));

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements));

    GLCall(glBindVertexArray(0));

    app->texturedGeometryProgramIdx = LoadProgram(app, "WorkingDir/shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
    GLCall(app->programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "u_Texture"));

    app->diceTexIdx = LoadTexture2D(app, "WorkingDir/dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "WorkingDir/color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "WorkingDir/color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "WorkingDir/color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "WorkingDir/color_magenta.png");

    app->mode = Mode_TexturedQuad;
}

void RenderImGui(App* app)
{
    ImGui::Begin("Info");

    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);

    if (ImGui::Button("Show OpenGL Info"))
    {
        std::cout << "[INFO] OpenGL Version: " << app->glInfo.version << std::endl;
        std::cout << "[INFO] OpenGL Vendor: " << app->glInfo.renderer << std::endl;
        std::cout << "[INFO] OpenGL Renderer: " << app->glInfo.vendor << std::endl;
        std::cout << "[INFO] OpenGL GLSL Version: " << app->glInfo.glslVersion << std::endl;
    }

    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
        {
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            // - set the viewport
            // - set the blending state
            // - bind the texture into unit 0
            // - bind the program 
            //   (...and make its texture sample from unit 0)
            // - bind the vao
            // - glDrawElements() !!!

            GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
            GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            GLCall(glViewport(0, 0, app->displaySize.x, app->displaySize.y));

            Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
            GLCall(glUseProgram(programTexturedGeometry.handle));
            GLCall(glBindVertexArray(app->vao));

            GLCall(glEnable(GL_BLEND));
            GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

            GLCall(glUniform1i(app->programUniformTexture, 0));
            GLCall(glActiveTexture(GL_TEXTURE0));
            GLuint textureHandle = app->textures[app->diceTexIdx].handle;
            GLCall(glBindTexture(GL_TEXTURE_2D, textureHandle));

            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            GLCall(glBindVertexArray(0));
            GLCall(glUseProgram(0));
        }
        break;

        default:
            break;
    }
}

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum errorCode = glGetError())
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << "[ERROR] OpenGL (Code " << errorCode << ": " << error << "): " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}