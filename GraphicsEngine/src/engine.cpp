//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "AssimpLoading.h"

#include "glad/glad.h"
#include "imgui-docking/imgui.h"

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

    // --- If a VAO wasn't found, create a new VAO for this mesh/program

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

    app->glState.version = "Version: " + std::string((const char*)glGetString(GL_VERSION));
    app->glState.renderer = "Renderer: " + std::string((const char*)glGetString(GL_RENDERER));
    app->glState.vendor = "Vendor: " + std::string((const char*)glGetString(GL_VENDOR));
    app->glState.glslVersion = "GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &app->glState.numExtensions);
    app->glState.extensions.reserve(app->glState.numExtensions);
    for (int i = 0; i < app->glState.numExtensions; ++i)
    {
        app->glState.extensions.emplace_back((const char*)glGetStringi(GL_EXTENSIONS, GLuint(i)));
    }

    app->camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    app->camera.front = glm::vec3(0.0f, 0.0f, -1.0f);
    app->camera.up = glm::vec3(0.0f, 1.0f, 0.0f);

    app->camera.speed = 1.5f;
    app->camera.sensitivity = 0.1f;

    app->camera.yaw = -90.0f;
    app->camera.pitch = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /*
    app->models.push_back(Model{});
    Model& model = app->models.back();
    u32 modelID = (u32)app->models.size() - 1u;

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

    app->meshProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/MeshShader.glsl", "TEXTURED_MESH");
    ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->meshProgramID];
    app->meshTextureLocation = glGetUniformLocation(texturedMeshProgram.handle, "u_Texture");

    Entity patrickEntity = Entity();
    patrickEntity.modelID = LoadModel(app, "Assets/Patrick/Patrick.obj", patrickEntity.model);
    app->entities.push_back(patrickEntity);

    app->numEntities = app->entities.size();
    app->mode = RenderMode::TexturedMesh;

    // MVP Matrices initialization & setup 
    app->model = glm::mat4(1.0f);
    app->model = glm::rotate(app->model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    app->view = glm::mat4(1.0f);

    app->projection = glm::mat4(1.0f);
    app->projection = glm::perspective(glm::radians(60.0f), float(app->displaySize.x) / float(app->displaySize.y), 0.1f, 100.0f);

    app->modelLoc = glGetUniformLocation(texturedMeshProgram.handle, "model");
    app->viewLoc = glGetUniformLocation(texturedMeshProgram.handle, "view");
    app->projectionLoc = glGetUniformLocation(texturedMeshProgram.handle, "projection");
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

        ImGui::Text(app->glState.version.c_str());
        ImGui::Text(app->glState.renderer.c_str());
        ImGui::Text(app->glState.vendor.c_str());
        ImGui::Text(app->glState.glslVersion.c_str());

        ImGui::Spacing();

        ImGui::Text("Extensions Supported:");
        for (int i = 0; i < app->glState.numExtensions; ++i)
            ImGui::Text(app->glState.extensions[i].c_str());

        ImGui::End();
    }

    if (app->debugInfo)
    {
        ImGui::Begin("App Info", &app->debugInfo);
        ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
        ImGui::Text("Frametime: %f", app->deltaTime);
        ImGui::Text("Time: %f", app->currentTime);
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;

    // Rotate around target
    //const float radius = 10.0f;
    //app->camera.position.x = sin(app->currentTime) * radius;
    //app->camera.position.z = cos(app->currentTime) * radius;
    //app->view = glm::lookAt(app->camera.position, app->camera.target, glm::vec3(0.0f, 1.0f, 0.0f));

    // Camera Rotation (Look Around)
    app->camera.yaw = glm::mod(app->camera.yaw + app->input.mouseDelta.x * app->camera.sensitivity, 360.0f); // Constrain yaw to only use values between 0-360 as float precision could be lost
    app->camera.pitch -= app->input.mouseDelta.y * app->camera.sensitivity;

    if (app->camera.pitch > 89.0f)
        app->camera.pitch = 89.0f;
    if (app->camera.pitch < -89.0f)
        app->camera.pitch = -89.0f;

    app->camera.front.x = glm::cos(glm::radians(app->camera.yaw)) * glm::cos(glm::radians(app->camera.pitch));
    app->camera.front.y = glm::sin(glm::radians(app->camera.pitch));
    app->camera.front.z = glm::sin(glm::radians(app->camera.yaw)) * glm::cos(glm::radians(app->camera.pitch));

    app->camera.front = glm::normalize(app->camera.front);

    // Camera Movement (Walk Around)
    float speed = app->camera.speed * app->deltaTime;
    if (app->input.keys[K_W] == BUTTON_PRESSED)
        app->camera.position += speed * app->camera.front;
    if (app->input.keys[K_S] == BUTTON_PRESSED)
        app->camera.position -= speed * app->camera.front;
    if (app->input.keys[K_A] == BUTTON_PRESSED)
        app->camera.position -= glm::normalize(glm::cross(app->camera.front, app->camera.up)) * speed;
    if (app->input.keys[K_D] == BUTTON_PRESSED)
        app->camera.position += glm::normalize(glm::cross(app->camera.front, app->camera.up)) * speed;

    app->view = glm::lookAt(app->camera.position, app->camera.position + app->camera.front, app->camera.up);

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
            //app->meshTextureLocation = glGetUniformLocation(shaderProgram.handle, "u_Texture");
            //shaderProgram.vertexLayout.attributes.clear();
            //shaderProgram.vertexLayout.attributes.shrink_to_fit();
            //InputShaderLayout(shaderProgram);
        }
    }
}

void Render(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (app->mode)
    {
    case RenderMode::TexturedMesh:
    {
        ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->meshProgramID];
        glUseProgram(texturedMeshProgram.handle);

        for (int i = 0; i < app->numEntities; ++i)
        {
            u32 numMeshes = app->entities[i].model.meshes.size();
            for (u32 meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
            {
                u32 vao = FindVAO(app->entities[i].model, meshIndex, texturedMeshProgram);
                glBindVertexArray(vao);

                u32 meshMaterialID = app->entities[i].model.materialIDs[meshIndex];
                Material& meshMaterial = app->materials[meshMaterialID];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
                glUniform1i(app->meshTextureLocation, 0);

                glUniformMatrix4fv(app->modelLoc, 1, GL_FALSE, glm::value_ptr(app->model));
                glUniformMatrix4fv(app->viewLoc, 1, GL_FALSE, glm::value_ptr(app->view));
                glUniformMatrix4fv(app->projectionLoc, 1, GL_FALSE, glm::value_ptr(app->projection));

                Mesh& mesh = app->entities[i].model.meshes[meshIndex];
                glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

                glBindVertexArray(0);
            }
        }
        glUseProgram(0);
    }
    break;

    case RenderMode::TexturedQuad:
    {
        ShaderProgram& programTexturedQuad = app->shaderPrograms[app->quadProgramID];
        glUseProgram(programTexturedQuad.handle);

        glBindVertexArray(app->vao.handle);

        glUniform1i(app->quadTextureLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        GLuint textureHandle = app->textures[app->diceTexIdx].handle;
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }
    break;

    case RenderMode::Count:
        break;

    default:
        break;
    }
}