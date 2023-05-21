//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
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
    app->performanceStatus = false;
    app->sceneStatus = true;

    // CAMERA & PROJECTION //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f), 45.0f, 0.1f, 100.0f);

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);
    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // DEFERRED SHADING //
    // G-Buffer
    app->GBuffer.Generate();
    app->GBuffer.Bind();
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize);   // Position Color Buffer
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize);   // Normal Color Buffer
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Albedo Color Buffer
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Specular Color Buffer
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Depth Color Buffer
    app->GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Depth Linear Color Buffer
    app->GBuffer.AttachDepthTexture(app->displaySize);                                  // Depth Attachment
    app->GBuffer.SetColorBuffers(); // Set color buffers with glDrawBuffers
    BindDefaultFramebuffer();

    // Lighting Pass
    app->lightPassShaderHandle = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/LightingPass_Shader_D.glsl", "DEFERRED_LIGHTING_PASS")].handle;
    glUseProgram(app->lightPassShaderHandle);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufPosition"),   0);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufNormal"),     1);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufAlbedo"),     2);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufSpecular"),   3);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufDepth"),      4);
    glUniform1i(glGetUniformLocation(app->lightPassShaderHandle, "gBufDepthLinear"),5);

    // Screen-Filling Quad
    app->screenQuad.FBO.Generate();
    app->screenQuad.FBO.Bind();
    app->screenQuad.FBO.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize); // Final Color Buffer
    app->screenQuad.FBO.SetColorBuffers(); // Set color buffers with glDrawBuffers
    BindDefaultFramebuffer();

    app->screenQuad.VAO = CreateQuad();
    app->screenQuad.shaderHandle = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Quad_Shader_D.glsl", "SCREEN_QUAD")].handle;
    glUseProgram(app->screenQuad.shaderHandle);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "uRenderTarget"), 0);

    // Map for Render Target Selection
    app->screenQuad.currentRenderTarget =   app->screenQuad.FBO.colorAttachmentHandles[0];
    app->renderTargets.push_back("FINAL COLOR");
    app->renderTargets.push_back("POSITION");
    app->renderTargets.push_back("NORMAL");
    app->renderTargets.push_back("ALBEDO");
    app->renderTargets.push_back("SPECULAR");
    app->renderTargets.push_back("DEPTH");
    app->renderTargets.push_back("DEPTH LINEAR");

    // SHADERS & UNIFORM TEXTURES //
    app->defaultProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Default_Shader_D.glsl", "DEFERRED_GEOMETRY_DEFAULT");
    app->lightCasterProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/LightCaster_Shader.glsl", "LIGHT_CASTER");

    u32 texturedAlbProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/GeometryPassAlb_Shader_D.glsl", "DEFERRED_GEOMETRY_ALBEDO");
    Shader& texturedAlbProgram = app->shaderPrograms[texturedAlbProgramID];
    texturedAlbProgram.Bind();
    texturedAlbProgram.SetUniform1i("uMaterial.albedo", 0);

    u32 texturedAlbSpecProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/GeometryPassAlbSpec_Shader_D.glsl", "DEFERRED_GEOMETRY_ALBEDO_SPECULAR");
    Shader& texturedAlbSpecProgram = app->shaderPrograms[texturedAlbSpecProgramID];
    texturedAlbSpecProgram.Bind();
    texturedAlbSpecProgram.SetUniform1i("uMaterial.albedo", 0);
    texturedAlbSpecProgram.SetUniform1i("uMaterial.specular", 1);

    // TEXTURES //
    u32 diceTexIdx = LoadTexture2D(app->textures, "Assets/dice.png");
    u32 whiteTexIdx = LoadTexture2D(app->textures, "Assets/color_white.png");
    u32 blackTexIdx = LoadTexture2D(app->textures, "Assets/color_black.png");
    u32 normalTexIdx = LoadTexture2D(app->textures, "Assets/color_normal.png");
    u32 magentaTexIdx = LoadTexture2D(app->textures, "Assets/color_magenta.png");

    u32 containerAlbedoTexID = LoadTexture2D(app->textures, "Assets/container_albedo.png");
    u32 containerSpecularID = LoadTexture2D(app->textures, "Assets/container_specular.png");

    // MATERIALS //
    Material greyMaterial = {};
    greyMaterial.type = MaterialType::DEFAULT;
    greyMaterial.name = "Grey Material";
    greyMaterial.albedo = glm::vec3(0.4f, 0.4f, 0.4f);
    greyMaterial.specular = glm::vec3(0.5f);
    greyMaterial.shininess = 32.0f;

    Material orangeMaterial = {};
    orangeMaterial.type = MaterialType::DEFAULT;
    orangeMaterial.name = "Orange Material";
    orangeMaterial.albedo = glm::vec3(1.0f, 0.5f, 0.31f);
    orangeMaterial.specular = glm::vec3(0.5f);
    orangeMaterial.shininess = 32.0f;

    Material blackMaterial = {};
    blackMaterial.type = MaterialType::DEFAULT;
    blackMaterial.name = "Black Material";
    blackMaterial.albedo = glm::vec3(0.1f, 0.1f, 0.1f);
    blackMaterial.specular = glm::vec3(0.5f);
    blackMaterial.shininess = 32.0f;

    Material containerMat = {};
    containerMat.type = MaterialType::TEXTURED_ALB_SPEC;
    containerMat.name = "Container Material";
    containerMat.shininess = 32.0f;
    containerMat.albedoTextureID = containerAlbedoTexID;
    containerMat.specularTextureID = containerSpecularID;

    // MODELS //
    Model* planeModel = CreatePrimitive(app, PrimitiveType::PLANE, greyMaterial);

    Model* sphereModel = CreatePrimitive(app, PrimitiveType::SPHERE, orangeMaterial);
    Model* sphereLowModel = CreatePrimitive(app, PrimitiveType::SPHERE, blackMaterial, 16, 16);

    Model* cubeModel = CreatePrimitive(app, PrimitiveType::CUBE, containerMat);
    Model* cubeModel2 = CreatePrimitive(app, PrimitiveType::CUBE, blackMaterial);

    Model* patrickModel = LoadModel(app, "Assets/Models/Patrick/patrick.obj");

    // ENTITIES //
    // Primitives
    Entity* planeEntity = CreateEntity(app, app->defaultProgramID, glm::vec3(0.0f, -5.0f, 0.0f), planeModel);
    planeEntity->modelMatrix = glm::rotate(planeEntity->modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    planeEntity->modelMatrix = glm::scale(planeEntity->modelMatrix, glm::vec3(35.0f));

    Entity* sphereEntity = CreateEntity(app, app->defaultProgramID, glm::vec3(0.0f, 0.0f, 7.0f), sphereModel);

    Entity* cubeEntity = CreateEntity(app, texturedAlbSpecProgramID, glm::vec3(5.0f, 0.0f, 7.0f), cubeModel);

    Entity* cubeEntity2 = CreateEntity(app, app->defaultProgramID, glm::vec3(-5.0f, 0.0f, 7.0f), cubeModel2);

    // 3D Models
    CreateEntity(app, texturedAlbProgramID, glm::vec3(-6.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(0.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(6.0f, 0.0f, 0.0f), patrickModel);

    CreateEntity(app, texturedAlbProgramID, glm::vec3(-6.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(0.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(6.0f, 0.0f, -4.0f), patrickModel);

    CreateEntity(app, texturedAlbProgramID, glm::vec3(-6.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(0.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, texturedAlbProgramID, glm::vec3(6.0f, 0.0f, -8.0f), patrickModel);

    app->firstLightEntityID = app->entities.size();

    // LIGHTS //
    // Random positions for point lights
    srand(13);
    for (unsigned int i = 0; i < 8; i++)
    {
        float xPos = static_cast<float>(((rand() % 100) / 70.0f) * 6.0f - 3.0f);
        float yPos = static_cast<float>(((rand() % 100) / 80.0f) * 6.0f - 4.0f);
        float zPos = static_cast<float>(((rand() % 100) / 25.0f) * 6.0f - 16.0f);
        CreatePointLight(app, glm::vec3(xPos, yPos, zPos), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);
    }

    CreatePointLight(app, glm::vec3(-6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 0.5f, sphereLowModel, 0.1f);
    CreatePointLight(app, glm::vec3(6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);

    CreateDirectionalLight(app, glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.1f), glm::vec3(0.3f), glm::vec3(0.5f));

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
        if (ImGui::BeginMenu("Engine"))
        {
            ImGui::MenuItem("OpenGL", NULL, &app->openGLStatus);
            ImGui::MenuItem("Scene", NULL, &app->sceneStatus);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Performance", NULL, &app->performanceStatus);
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
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Extensions Supported:");
        for (int i = 0; i < app->glState.numExtensions; ++i)
            ImGui::Text(app->glState.extensions[i].c_str());

        ImGui::End();
    }

    if (app->sceneStatus)
    {
        ImGui::Begin("Scene", &app->sceneStatus);

        ImGui::Text("Camera");
        ImGui::DragFloat3("Position", &app->camera.position[0]);
        ImGui::DragFloat("Speed", &app->camera.speed);
        ImGui::DragFloat("Zoom", &app->camera.FOV, 1.0f, 5.0f, 45.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Directional Light");
        ImGui::DragFloat3("Direction", &app->lights[10].direction[0], 0.1f, -1.0f, 1.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Render Target");
        static const char* preview = "FINAL COLOR";
        if (ImGui::BeginCombo("", preview))
        {
            for (int i = 0; i < app->renderTargets.size(); ++i)
            {
                bool isSelected = (preview == app->renderTargets[i]);
                if (ImGui::Selectable(app->renderTargets[i], isSelected))
                {
                    preview = app->renderTargets[i];
                    if (i == 0)
                        app->screenQuad.currentRenderTarget = app->screenQuad.FBO.colorAttachmentHandles[0];
                    else if (i >= 1)
                        app->screenQuad.currentRenderTarget = app->GBuffer.colorAttachmentHandles[i64(i) - 1];
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();
    }

    if (app->performanceStatus)
    {
        ImGui::Begin("Performance", &app->performanceStatus);
        ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
        ImGui::Text("Frametime: %f", app->deltaTime);
        ImGui::Text("Time: %f", app->currentTime);
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;
    
    app->camera.ProcessInput(app->input, app->displaySize, app->deltaTime);

    for (u32 i = 0; i < app->shaderPrograms.size(); ++i)
    {
        Shader& shaderProgram = app->shaderPrograms[i];
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

    // DEFERRED SHADING: GEOMETRY PASS //
    app->GBuffer.Bind();

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
        Shader& shader = app->shaderPrograms[entity.shaderID];
        Model* model = entity.model;

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO.handle, entity.localParamOffset, entity.localParamSize);

        shader.Bind();

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
                shader.SetUniform3f("uMaterial.albedo", meshMaterial.albedo);
                shader.SetUniform3f("uMaterial.specular", meshMaterial.specular);
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess);
            }
            break;
            case MaterialType::TEXTURED_ALBEDO:
            {
                // Albedo Map
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                // Material
                shader.SetUniform3f("uMaterial.specular", meshMaterial.specular);
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess);
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
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess);
            }
            break;
            }

            Mesh& mesh = model->meshes[meshIndex];
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);
            glBindVertexArray(0);
        }
        shader.Unbind();
    }
    BindDefaultFramebuffer();

    // DEFERRED SHADING: LIGHTING PASS //
    app->screenQuad.FBO.Bind();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->lightPassShaderHandle);

    glBindVertexArray(app->screenQuad.VAO);

    // Set the uniform textures from the G-Buffer
    for (u32 i = 0; i < app->GBuffer.colorAttachmentHandles.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, app->GBuffer.colorAttachmentHandles[i]);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    // SCREEN-FILLING QUAD //
    BindDefaultFramebuffer();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->screenQuad.shaderHandle);

    glBindVertexArray(app->screenQuad.VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->screenQuad.currentRenderTarget);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    // FORWARD SHADING: LIGHTS //
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, app->GBuffer.handle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, app->displaySize.x, app->displaySize.y, 0, 0, app->displaySize.x, app->displaySize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    BindDefaultFramebuffer();

    Shader& lightShader = app->shaderPrograms[app->lightCasterProgramID];
    lightShader.Bind();
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
    lightShader.Unbind();
}

u32 FindVAO(Model* model, u32 meshIndex, const Shader& shaderProgram)
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

    glm::mat4 VPMatrix = app->camera.GetViewProjectionMatrix(app->displaySize);
    // Local Parameters //
    for (u32 i = 0; i < app->numEntities; ++i)
    {
        AlignHead(app->UBO, app->uniformBufferOffsetAlignment);

        Entity& entity = app->entities[i];
        entity.localParamOffset = app->UBO.head;

        const glm::mat4& modelMatrix = entity.modelMatrix;
        PushMat4(app->UBO, modelMatrix);

        glm::mat4 MVP = VPMatrix * modelMatrix;
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
    Entity lightEntity = Entity(app->lightCasterProgramID, position, model);
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