//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Engine.h"
#include "AssimpLoading.h"
#include "Primitives.h"

#include "glad/glad.h"
#include "imgui-docking/imgui.h"

void Init(App* app)
{
    // OPENGL DEBUG //
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

    // IMGUI WINDOWS //
    app->openGLStatus = false;
    app->debugInfo = false;
    app->sceneInfo = false;

    // CAMERA & PROJECTION //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f), app->displaySize, 45.0f, 0.1f, 100.0f);

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);
    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // SCREEN-FILLING QUAD //
    app->screenQuad.VAO = CreateQuad();
    app->screenQuad.shaderHandle = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Quad_Shader_D.glsl", "SCREEN_QUAD")].handle;
    app->screenQuad.targetBuffer = 0;
    glUseProgram(app->screenQuad.shaderHandle);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "gBufPosition"), 0);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "gBufNormal"), 1);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "gBufAlbedoSpec"), 2);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "gBufDepth"), 3);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "gBufDepthLinear"), 4);

    // G-Buffer
    app->gBuffer.Generate();
    app->gBuffer.Bind();

    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize); // Position Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize); // Normal Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize); // Albedo + Specular Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize); // Depth Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize); // Depth Linearalized Color Buffer
    app->gBuffer.AttachDepthTexture(app->displaySize); // Depth Attachment

    app->gBuffer.SetColorBuffers();
    app->gBuffer.Unbind();

    //app->targets = { "FINAL", "POSITION", "NORMAL", "ALBEDO", "SPECULAR", "DEPTH", "DEPTH LINEAR" };

    // SHADERS & UNIFORM TEXTURES //
    app->defaultProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Default_Shader_D.glsl", "DEFAULT");
    app->lightProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Light_Shader.glsl", "LIGHT_CASTER");

    u32 texturedAlbProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/TexturedAlb_Shader_D.glsl", "TEXTURED_ALBEDO");
    glUseProgram(app->shaderPrograms[texturedAlbProgramID].handle);
    glUniform1i(glGetUniformLocation(app->shaderPrograms[texturedAlbProgramID].handle, "uMaterial.albedo"), 0);

    u32 texturedAlbSpecProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/TexturedAlbSpec_Shader_D.glsl", "TEXTURED_ALBEDO_SPECULAR");
    glUseProgram(app->shaderPrograms[texturedAlbSpecProgramID].handle);
    glUniform1i(glGetUniformLocation(app->shaderPrograms[texturedAlbSpecProgramID].handle, "uMaterial.albedo"), 0);
    glUniform1i(glGetUniformLocation(app->shaderPrograms[texturedAlbSpecProgramID].handle, "uMaterial.specular"), 1);

    // TEXTURES //
    u32 diceTexIdx = LoadTexture2D(app->textures, "Assets/dice.png");
    u32 whiteTexIdx = LoadTexture2D(app->textures, "Assets/color_white.png");
    u32 blackTexIdx = LoadTexture2D(app->textures, "Assets/color_black.png");
    u32 normalTexIdx = LoadTexture2D(app->textures, "Assets/color_normal.png");
    u32 magentaTexIdx = LoadTexture2D(app->textures, "Assets/color_magenta.png");

    u32 containerAlbedoTexID = LoadTexture2D(app->textures, "Assets/container_albedo.png");
    u32 containerSpecularID = LoadTexture2D(app->textures, "Assets/container_specular.png");

    // MATERIALS //
    Material defaultMat = {};
    defaultMat.type = MaterialType::DEFAULT;
    defaultMat.name = "Default Material";
    defaultMat.albedo = glm::vec3(1.0f, 0.5f, 0.31f);
    defaultMat.specular = glm::vec3(0.5f);
    defaultMat.shininess = 32.0f;

    Material defaultMat2 = {};
    defaultMat2.type = MaterialType::DEFAULT;
    defaultMat2.name = "Default Material 2";
    defaultMat2.albedo = glm::vec3(0.0f, 1.0f, 0.0f);
    defaultMat2.specular = glm::vec3(0.5f);
    defaultMat2.shininess = 32.0f;

    Material containerMat = {};
    containerMat.type = MaterialType::TEXTURED_ALB_SPEC;
    containerMat.name = "Container Material";
    containerMat.specular = glm::vec3(0.5f);
    containerMat.shininess = 32.0f;
    containerMat.albedoTextureID = containerAlbedoTexID;
    containerMat.specularTextureID = containerSpecularID;

    // MODELS //
    Model* planeModel = CreatePrimitive(app, PrimitiveType::PLANE, defaultMat);

    Model* sphereModel = CreatePrimitive(app, PrimitiveType::SPHERE, defaultMat);
    Model* sphereLowModel = CreatePrimitive(app, PrimitiveType::SPHERE, defaultMat2, 16, 16);

    Model* cubeModel = CreatePrimitive(app, PrimitiveType::CUBE, containerMat);
    Model* cubeModel2 = CreatePrimitive(app, PrimitiveType::CUBE, defaultMat2);

    Model* patrickModel = LoadModel(app, "Assets/Models/Patrick/patrick.obj");

    // ENTITIES //
    // Primitives
    Entity* planeEntity = CreateEntity(app, app->defaultProgramID, glm::vec3(0.0f, -5.0f, 0.0f), planeModel);
    planeEntity->modelMatrix = glm::rotate(planeEntity->modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    planeEntity->modelMatrix = glm::scale(planeEntity->modelMatrix, glm::vec3(25.0f));

    Entity* sphereEntity = CreateEntity(app, app->defaultProgramID, glm::vec3(0.0f, 0.0f, 3.0f), sphereModel);

    Entity* cubeEntity = CreateEntity(app, texturedAlbSpecProgramID, glm::vec3(5.0f, 0.0f, 2.0f), cubeModel);

    Entity* cubeEntity2 = CreateEntity(app, app->defaultProgramID, glm::vec3(-5.0f, 0.0f, 2.0f), cubeModel2);

    //glm::vec3 cubePositions[] = {
    //glm::vec3(0.0f,  0.0f,  0.0f),
    //glm::vec3(2.0f,  5.0f, -15.0f),
    //glm::vec3(-1.5f, -2.2f, -2.5f),
    //glm::vec3(-3.8f, -2.0f, -12.3f),
    //glm::vec3(2.4f, -0.4f, -3.5f),
    //glm::vec3(-1.7f,  3.0f, -7.5f),
    //glm::vec3(1.3f, -2.0f, -2.5f),
    //glm::vec3(1.5f,  2.0f, -2.5f),
    //glm::vec3(1.5f,  0.2f, -1.5f),
    //glm::vec3(-1.3f,  1.0f, -1.5f)
    //};
    //for (u32 i = 0; i < 10; ++i)
    //{
    //    Entity* cubeEntity = CreateEntity(app, geometryTexAlbProgramID, cubePositions[i], cubeModel);
    //    float angle = 20.0f * i;
    //    cubeEntity->modelMatrix = glm::rotate(cubeEntity->modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    //}

    // 3D Models
    Entity* patrickEntity = CreateEntity(app, texturedAlbProgramID, glm::vec3(0.0f, 0.0f, -5.0f), patrickModel);

    app->firstLightEntityID = app->entities.size();

    // LIGHTS //
    CreatePointLight(app, glm::vec3(3.0f, 1.0f, -2.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);
    CreatePointLight(app, glm::vec3(-5.0f, 1.0f, 0.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);

    CreatePointLight(app, glm::vec3(-6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 0.4f, sphereLowModel, 0.1f);
    CreatePointLight(app, glm::vec3(6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);

    //CreateDirectionalLight(app, glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.2f), glm::vec3(0.5f), glm::vec3(1.0f));

    // ENGINE COUNT OF ENTITIES & LIGHTS //
    app->numEntities = app->entities.size();
    app->numLights = app->lights.size();

    // OPENGL GLOBAL STATE //
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

        ImGui::Separator();
        ImGui::Text("Entities");
        for (int i = 0; i < app->numEntities; ++i)
            ImGui::DragFloat3(std::to_string(i).c_str(), &app->entities[i].position[0], 0.1f);
        
        //ImGui::Separator();
        //ImGui::Text("Render Target");
        //const char* preview = targets[app->framebuffer.currentAttachment];
        //if (ImGui::BeginCombo("Attachments", preview))
        //{
        //    for (int i = 0; i < app->framebuffer.colorAttachmentHandles.size(); ++i)
        //    {
        //        const bool isSelected = (app->framebuffer.currentAttachment == i);
        //        if (ImGui::Selectable(targets[i], isSelected))
        //            app->framebuffer.currentAttachment = app->framebuffer.colorAttachmentHandles[i];
        //    }
        //    ImGui::EndCombo();
        //}
        
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;
    
    app->camera.ProcessInput(app->input, app->deltaTime);

    if (app->input.keys[K_1] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 0;
    if (app->input.keys[K_2] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 1;
    if (app->input.keys[K_3] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 2;
    if (app->input.keys[K_4] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 3;
    if (app->input.keys[K_5] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 4;
    if (app->input.keys[K_6] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 5;
    if (app->input.keys[K_7] == BUTTON_PRESS)
        app->screenQuad.targetBuffer = 6;

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
        }
    }
}

void Render(App* app)
{
    UpdateUniformBuffer(app);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->UBO.handle, app->globalParamOffset, app->globalParamSize);

    app->gBuffer.Bind();

    // OPENGL SCENE STATE //
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (u32 i = 0; i < app->firstLightEntityID; ++i)
    {
        Entity& entity = app->entities[i];
        ShaderProgram& shader = app->shaderPrograms[entity.shaderID];
        Model* model = entity.model;

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO.handle, entity.localParamOffset, entity.localParamSize);

        glUseProgram(shader.handle);

        u32 numMeshes = model->meshes.size();
        for (u32 meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
        {
            u32 vao = FindVAO(model, meshIndex, shader);
            glBindVertexArray(vao);

            u32 meshMaterialID = model->materialIDs[meshIndex];
            Material& meshMaterial = app->materials[meshMaterialID];

            // Uniforms
            switch (meshMaterial.type)
            {
            case MaterialType::DEFAULT:
            {
                // Material
                glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.albedo"), 1, &meshMaterial.albedo[0]);
                glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.specular"), 1, &meshMaterial.specular[0]);
                glUniform1f(glGetUniformLocation(shader.handle, "uMaterial.shininess"), meshMaterial.shininess);
            }
            break;
            case MaterialType::TEXTURED_ALBEDO:
            {
                // Albedo Map
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                // Material
                glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.specular"), 1, &meshMaterial.specular[0]);
                glUniform1f(glGetUniformLocation(shader.handle, "uMaterial.shininess"), meshMaterial.shininess);
            }
            break;
            case MaterialType::TEXTURED_ALB_SPEC:
            {
                // Albedo Map
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                // Specular Map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.specularTextureID].handle);

                // Material
                glUniform1f(glGetUniformLocation(shader.handle, "uMaterial.shininess"), meshMaterial.shininess);
            }
            break;
            }

            Mesh& mesh = model->meshes[meshIndex];
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);
            glBindVertexArray(0);
        }
        glUseProgram(0);
    }

    app->gBuffer.Unbind();

    // OPENGL SCREEN-FILLING QUAD STATE //
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->screenQuad.shaderHandle);

    glBindVertexArray(app->screenQuad.VAO);

    for (u32 i = 0; i < app->gBuffer.colorAttachmentHandles.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, app->gBuffer.colorAttachmentHandles[i]);
    }

    glUniform1ui(glGetUniformLocation(app->screenQuad.shaderHandle, "targetBuffer"), app->screenQuad.targetBuffer);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    // LIGHTS RENDERING //
    ShaderProgram& lightShader = app->shaderPrograms[app->lightProgramID];
    glUseProgram(lightShader.handle);
    for (u32 i = app->firstLightEntityID; i < app->numEntities; ++i)
    {
        Entity& lightEntity = app->entities[i];

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO.handle, lightEntity.localParamOffset, lightEntity.localParamSize);

        Model* model = lightEntity.model;

        u32 vao = FindVAO(model, 0, lightShader);
        glBindVertexArray(vao);

        Mesh& mesh = model->meshes[0];
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

        glBindVertexArray(0);
    }
    glUseProgram(0);
}

u32 FindVAO(Model* model, u32 meshIndex, const ShaderProgram& shaderProgram)
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBHandle);

    // We have to link all vertex shader inputs attributes to attributes in the vertex buffer
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
    MapBuffer(app->UBO, GL_WRITE_ONLY);

    // Global Parameters //
    app->globalParamOffset = app->UBO.head;

    // Camera Position
    PushVec3(app->UBO, app->camera.position);

    // Lights
    PushUInt(app->UBO, app->numLights);
    for (u32 i = 0; i < app->numLights; ++i)
    {
        AlignHead(app->UBO, sizeof(glm::vec4));

        Light& light = app->lights[i];
        PushUInt(app->UBO, (u32)light.type);
        PushVec3(app->UBO, light.position);
        PushVec3(app->UBO, light.direction);
        PushVec3(app->UBO, light.ambient);
        PushVec3(app->UBO, light.diffuse);
        PushVec3(app->UBO, light.specular);
        PushFloat(app->UBO, light.constant);
    }
    app->globalParamSize = app->UBO.head - app->globalParamOffset;

    // Local Parameters //
    for (u32 i = 0; i < app->numEntities; ++i)
    {
        AlignHead(app->UBO, app->uniformBufferOffsetAlignment);

        Entity& entity = app->entities[i];
        entity.localParamOffset = app->UBO.head;

        glm::mat4& modelMatrix = entity.modelMatrix;
        PushMat4(app->UBO, modelMatrix);

        glm::mat4 MVP = app->camera.GetProjectionMatrix() * app->camera.GetViewMatrix() * modelMatrix;
        PushMat4(app->UBO, MVP);

        entity.localParamSize = app->UBO.head - entity.localParamOffset;
    }
    UnmapBuffer(app->UBO);
}

Entity* CreateEntity(App* app, u32 shaderID, glm::vec3 position, Model* model)
{
    Entity entity = Entity(shaderID, position, model);
    app->entities.push_back(entity);

    return &app->entities[app->entities.size() - 1u];
}

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, Model* model, float scale)
{
    Entity lightEntity = Entity(app->lightProgramID, position, model);
    lightEntity.modelMatrix = glm::scale(lightEntity.modelMatrix, glm::vec3(scale));
    app->entities.push_back(lightEntity);

    Light light = { LightType::POINT, position, glm::vec3(0.0f), ambient, diffuse, specular, constant };
    app->lights.push_back(light);
}

void CreateDirectionalLight(App* app, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
    Light light = { LightType::DIRECTIONAL, glm::vec3(0.0f), direction, ambient, diffuse, specular, 1.0f };
    app->lights.push_back(light);
}