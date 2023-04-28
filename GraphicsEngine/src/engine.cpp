//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "AssimpLoading.h"
#include "BufferManagement.h"
#include "Primitives.h"

#include "glad/glad.h"
#include "imgui-docking/imgui.h"

u32 FindVAO(Model* model, u32 meshIndex, const ShaderProgram& shaderProgram, bool hasIndices)
{
    Mesh& mesh = model->meshes[meshIndex];

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

    glBindBuffer(GL_ARRAY_BUFFER, model->VBHandle);
    if (hasIndices)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBHandle);

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

void UpdateUniformBuffer(App* app)
{
    glBindBuffer(GL_UNIFORM_BUFFER, app->UBO);
    u8* bufferData = (u8*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    u32 bufferHead = 0;

    for (u32 i = 0; i < app->numEntities; ++i)
    {
        bufferHead = Align(bufferHead, app->uniformBufferOffsetAlignment);
        app->entities[i].localParamOffset = bufferHead;

        memcpy(bufferData + bufferHead, &app->entities[i].modelMatrix[0][0], sizeof(glm::mat4));
        bufferHead += sizeof(glm::mat4);

        glm::mat4 MVP = app->projection * app->camera.GetViewMatrix() * app->entities[i].modelMatrix;
        memcpy(bufferData + bufferHead, &MVP, sizeof(glm::mat4));
        bufferHead += sizeof(glm::mat4);

        app->entities[i].localParamSize = bufferHead - app->entities[i].localParamOffset;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Init(App* app)
{
    app->debugInfo = false;
    app->openGLStatus = false;
    app->sceneInfo = false;

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

    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);

    glGenBuffers(1, &app->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, app->UBO);
    glBufferData(GL_UNIFORM_BUFFER, maxUniformBlockSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Camera setup
    app->camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));

    // Projection Matrix initialization & setup
    app->projection = glm::mat4(1.0f);
    app->projection = glm::perspective(glm::radians(45.0f), float(app->displaySize.x) / float(app->displaySize.y), 0.1f, 100.0f);

    app->quadProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/QuadShader.glsl", "TEXTURED_QUAD");
    ShaderProgram& texturedQuadProgram = app->shaderPrograms[app->quadProgramID];
    app->quadTextureLocation = glGetUniformLocation(texturedQuadProgram.handle, "uTexture");

    app->meshProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/MeshShader.glsl", "TEXTURED_MESH");
    ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->meshProgramID];
    app->meshTextureLocation = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    app->diceTexIdx = LoadTexture2D(app->textures, "Assets/dice.png");
    app->whiteTexIdx = LoadTexture2D(app->textures, "Assets/color_white.png");
    app->blackTexIdx = LoadTexture2D(app->textures, "Assets/color_black.png");
    app->normalTexIdx = LoadTexture2D(app->textures, "Assets/color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app->textures, "Assets/color_magenta.png");

    Model* patrickModel = new Model();
    u32 patrickModelID = LoadModel(app, "Assets/Patrick/Patrick.obj", patrickModel);

    Model* cubeModel = new Model();
    u32 cubeModelID = CreateCube(app, cubeModel);

    Entity patrickEntity = Entity(glm::vec3(0.0f));
    patrickEntity.model = patrickModel;
    patrickEntity.modelID = patrickModelID;
    app->entities.push_back(patrickEntity);

    Entity patrickClone1 = Entity(glm::vec3(-6.0f, 0.0f, 0.0f));
    patrickClone1.model = patrickModel;
    patrickClone1.modelID = patrickModelID;
    app->entities.push_back(patrickClone1);

    Entity patrickClone2 = Entity(glm::vec3(6.0f, 0.0f, 0.0f));
    patrickClone2.model = patrickModel;
    patrickClone2.modelID = patrickModelID;
    app->entities.push_back(patrickClone2);

    Entity cubeEntity = Entity(glm::vec3(0.0f, 0.0f, 3.0f), false);
    cubeEntity.model = cubeModel;
    cubeEntity.modelID = cubeModelID;
    app->entities.push_back(cubeEntity);



    //app->quad = Entity(glm::vec3(0.0f, 0.0f, 3.0f));
    //app->quad.modelID = CreateQuad(app, app->quad.model);
    //app->entities.push_back(quad);

    /////////////////////////////////////////////////////

    app->numEntities = app->entities.size();
    app->mode = RenderMode::TEXTURE_MESH;

    /////////////////////////////////////////////////////

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);
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
            ImGui::MenuItem("Debug Info", NULL, &app->debugInfo);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene"))
        {
            ImGui::MenuItem("Scene Info", NULL, &app->sceneInfo);
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

    if (app->sceneInfo)
    {
        ImGui::Begin("Scene Info", &app->sceneInfo);

        ImGui::Text("Camera");

        ImGui::DragFloat3("Position", &app->camera.position[0]);
        ImGui::DragFloat("Speed", &app->camera.speed);
        //glm::vec2 yawPitch = glm::vec2(app->camera.yaw, app->camera.pitch);
        //ImGui::DragFloat2("Yaw / Pitch", &yawPitch[0]);

        //ImGui::Separator();

        //ImGui::Text("Entities");
        //for (int i = 0; i < app->numEntities; ++i)
        //{
        //    if (ImGui::DragFloat3(std::to_string(i).c_str(), &app->entities[i].position[0], 0.1f))
        //    {
        //        // Move to the corresponding position
        //    }
        //}
        
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;

    if (app->input.keys[K_LSHIFT] == BUTTON_PRESS)
        app->camera.speed *= 3.0f;
    else if (app->input.keys[K_LSHIFT] == BUTTON_RELEASE)
        app->camera.speed = 2.0f;

    if (app->input.keys[K_W] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_FORWARD, app->deltaTime);
    if (app->input.keys[K_S] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_BACKWARD, app->deltaTime);
    if (app->input.keys[K_A] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_LEFT, app->deltaTime);
    if (app->input.keys[K_D] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_RIGHT, app->deltaTime);
    if (app->input.keys[K_Q] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_UP, app->deltaTime);
    if (app->input.keys[K_E] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CameraDirection::CAMERA_DOWN, app->deltaTime);

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

    UpdateUniformBuffer(app);

    ShaderProgram& texturedMeshProgram = app->shaderPrograms[app->meshProgramID];
    ShaderProgram& texturedQuadProgram = app->shaderPrograms[app->quadProgramID];

    switch (app->mode)
    {
    case RenderMode::TEXTURE_MESH:
    {
        for (u32 i = 0; i < app->numEntities; ++i)
        {
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO, app->entities[i].localParamOffset, app->entities[i].localParamSize);

            glUseProgram(texturedMeshProgram.handle);

            Model* model = app->entities[i].model;
            u32 numMeshes = model->meshes.size();
            for (u32 meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
            {
                u32 vao = FindVAO(model, meshIndex, texturedMeshProgram, app->entities[i].hasIndices);

                glBindVertexArray(vao);

                u32 meshMaterialID = model->materialIDs[meshIndex];
                Material& meshMaterial = app->materials[meshMaterialID];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
                glUniform1i(app->meshTextureLocation, 0);

                Mesh& mesh = model->meshes[meshIndex];

                if (app->entities[i].hasIndices)
                    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);
                else
                    glDrawArrays(GL_TRIANGLES, 0, 36); // Check for different primitives without indices, the only one supported by now is the cube (quad is indexed tho)

                glBindVertexArray(0);
            }

            glUseProgram(0);
        }
    }
    break;

    case RenderMode::QUAD:
    {
        glUseProgram(texturedQuadProgram.handle);

        Model* model = app->quad.model;

        u32 vao = FindVAO(model, 0, texturedQuadProgram, app->quad.hasIndices);
        glBindVertexArray(vao);

        u32 meshMaterialID = model->materialIDs[0];
        Material& meshMaterial = app->materials[meshMaterialID];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
        glUniform1i(app->quadTextureLocation, 0);

        Mesh& mesh = model->meshes[0];
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

        glBindVertexArray(0);

        glUseProgram(0);
    }
    break;
    }
}