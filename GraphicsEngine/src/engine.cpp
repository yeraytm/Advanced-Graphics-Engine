//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "AssimpLoading.h"

#include "imgui-docking/imgui.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

GLuint CreateShaderProgram(String programSource, const char* shaderName)
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
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadShaderProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    ShaderProgram program = {};
    program.handle = CreateShaderProgram(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->shaderPrograms.push_back(program);

    return app->shaderPrograms.size() - 1;
}

void InputShaderLayout(ShaderProgram& shaderProgram)
{
    shaderProgram.vertexLayout.attributes.clear();

    char* attributeName;
    GLint activeAttributes, attributeNameMaxLength;
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);

    attributeName = new char[attributeNameMaxLength++];

    for (u32 i = 0; i < activeAttributes; ++i)
    {
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(shaderProgram.handle, i, attributeNameMaxLength + 1, NULL, &attributeSize, &attributeType, attributeName);

        u8 attributeLocation = glGetAttribLocation(shaderProgram.handle, attributeName);

        u8 componentCount = 1;
        switch (attributeType)
        {
        case GL_FLOAT: componentCount = 1; break;
        case GL_FLOAT_VEC2: componentCount = 2; break;
        case GL_FLOAT_VEC3: componentCount = 3; break;
        case GL_FLOAT_VEC4: componentCount = 4; break;
        default:
            break;
        }

        shaderProgram.vertexLayout.attributes.push_back({ attributeLocation, componentCount });
    }
    delete[] attributeName;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

u32 CreateTexture2DFromImage(Image image)
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

    u32 texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

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
        return UINT32_MAX;
}

u32 FindVAO(Model& model, u32 meshIndex, const ShaderProgram& shaderProgram)
{
    Mesh& mesh = model.meshes[meshIndex];

    // Try Finding a VAO for this mesh/program
    for (u32 i = 0; i < (u32)mesh.VAOs.size(); ++i)
    {
        if (mesh.VAOs[i].shaderProgramHandle == shaderProgram.handle)
            return mesh.VAOs[i].handle;
    }

    u32 vaoHandle = 0;

    // Create new VAO for this mesh/program
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, model.VBHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBHandle);

    // We have to link all vertex inputs attributes to attributes in the vertex buffer
    for (u32 i = 0; i < shaderProgram.vertexLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        const std::vector<VertexBufferAttribute>& attributes = mesh.VBLayout.attributes;
        for (u32 j = 0; j < attributes.size(); ++j)
        {
            if (shaderProgram.vertexLayout.attributes[i].location == attributes[j].location)
            {
                const u32 index = attributes[j].location;
                const u32 nComp = attributes[j].componentCount;
                const u32 offset = attributes[j].offset + mesh.vertexOffset;
                const u32 stride = mesh.VBLayout.stride;

                glVertexAttribPointer(index, nComp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }
        assert(attributeWasLinked); // The mesh should provide an attribute for each vertex inputs
    }

    glBindVertexArray(0);

    // Store the VAO handle in the list of VAOs for this mesh
    VAO vao = { vaoHandle, shaderProgram.handle };
    mesh.VAOs.push_back(vao);

    return vaoHandle;
}

void Init(App* app)
{
    app->debugInfo = false;
    app->openGLStatus = false;

    app->glInfo.version = "Version: " + std::string((const char*)glGetString(GL_VERSION));
    app->glInfo.renderer = "Renderer: " + std::string((const char*)glGetString(GL_RENDERER));
    app->glInfo.vendor = "Vendor: " + std::string((const char*)glGetString(GL_VENDOR));
    app->glInfo.glslVersion = "GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    app->glInfo.extensions.reserve(numExtensions);
    for (int i = 0; i < numExtensions; ++i)
    {
        app->glInfo.extensions.emplace_back((const char*)glGetStringi(GL_EXTENSIONS, GLuint(i)));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
    const Vertex vertices[] = {
        { vec3(-0.5, -0.5, 0.0), vec2(0.0, 0.0) },
        { vec3(0.5, -0.5, 0.0), vec2(1.0, 0.0) },
        { vec3(0.5,  0.5, 0.0), vec2(1.0, 1.0) },
        { vec3(-0.5,  0.5, 0.0), vec2(0.0, 1.0) }
    };

    const u32 indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &app->vao.handle);
    glGenBuffers(1, &app->VBO);
    glGenBuffers(1, &app->EBO);

    glBindVertexArray(app->vao.handle);

    glBindBuffer(GL_ARRAY_BUFFER, app->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));
    glEnableVertexAttribArray(1);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    app->texturedQuadProgramID = LoadShaderProgram(app, "Assets/Shaders/QuadShader.glsl", "TEXTURED_QUAD");
    ShaderProgram& texturedQuadProgram = app->shaderPrograms[app->texturedQuadProgramID];
    app->programUniformTexture = glGetUniformLocation(texturedQuadProgram.handle, "u_Texture");

    app->diceTexIdx = LoadTexture2D(app, "Assets/dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "Assets/color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "Assets/color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "Assets/color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "Assets/color_magenta.png");
    */

    app->texturedMeshProgramID = LoadShaderProgram(app, "Assets/Shaders/MeshShader.glsl", "TEXTURED_MESH");
    ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->texturedMeshProgramID];
    app->programUniformTexture = glGetUniformLocation(texturedMeshProgram.handle, "u_Texture");
    InputShaderLayout(texturedMeshProgram);

    app->modelID = LoadModel(app, "Assets/Patrick/Patrick.obj");

    app->mode = Mode_TexturedMesh;
}

void ImGuiRender(App* app)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("About"))
        {
            ImGui::MenuItem("OpenGL", NULL, &app->openGLStatus);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Info", NULL, &app->debugInfo);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (app->openGLStatus)
    {
        ImGui::Begin("OpenGL", &app->openGLStatus);

        ImGui::Text(app->glInfo.version.c_str());
        ImGui::Text(app->glInfo.renderer.c_str());
        ImGui::Text(app->glInfo.vendor.c_str());
        ImGui::Text(app->glInfo.glslVersion.c_str());

        ImGui::End();
    }

    if (app->debugInfo)
    {
        ImGui::Begin("App Info", &app->debugInfo);
        ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
        ImGui::Text("frametime: %f", app->deltaTime);
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;

    for (u32 i = 0; i < app->shaderPrograms.size(); ++i)
    {
        ShaderProgram& shaderProgram = app->shaderPrograms[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(shaderProgram.filepath.c_str());
        if (currentTimestamp > shaderProgram.lastWriteTimestamp)
        {
            glDeleteProgram(shaderProgram.handle);
            String shaderProgramSrc = ReadTextFile(shaderProgram.filepath.c_str());
            const char* shaderProgramName = shaderProgram.programName.c_str();
            shaderProgram.handle = CreateShaderProgram(shaderProgramSrc, shaderProgramName);
            shaderProgram.lastWriteTimestamp = currentTimestamp;
            app->programUniformTexture = glGetUniformLocation(shaderProgram.handle, "u_Texture");
            InputShaderLayout(shaderProgram);
        }
    }
}

void Render(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (app->mode)
    {
    case Mode_TexturedQuad:
    {
        ShaderProgram& programTexturedQuad = app->shaderPrograms[app->texturedQuadProgramID];
        glUseProgram(programTexturedQuad.handle);

        glBindVertexArray(app->vao.handle);

        glUniform1i(app->programUniformTexture, 0);
        glActiveTexture(GL_TEXTURE0);
        GLuint textureHandle = app->textures[app->diceTexIdx].handle;
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }
    break;

    case Mode_TexturedMesh:
    {
        ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->texturedMeshProgramID];
        glUseProgram(texturedMeshProgram.handle);

        Model& model = app->models[app->modelID];

        for (u32 meshIndex = 0; meshIndex < model.meshes.size(); ++meshIndex)
        {
            u32 vao = FindVAO(model, meshIndex, texturedMeshProgram);
            glBindVertexArray(vao);

            u32 meshMaterialID = model.materialIDs[meshIndex];
            Material& meshMaterial = app->materials[meshMaterialID];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
            glUniform1i(app->programUniformTexture, 0);

            Mesh& mesh = model.meshes[meshIndex];
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

            glBindVertexArray(0);
        }
        glUseProgram(0);
    }
    break;

    case Mode_Count:
        break;

    default:
        break;
    }
}