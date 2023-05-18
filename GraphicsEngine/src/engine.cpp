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
    app->debugInfo = false;
    app->sceneInfo = false;

    // CAMERA & PROJECTION //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f), 45.0f, 0.1f, 100.0f);

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);
    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // DEFERRED SHADING //
    // G-Buffer
    app->gBuffer.Generate();
    app->gBuffer.Bind();

    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize);   // Position Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize);   // Normal Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Albedo Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Specular Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Depth Color Buffer
    app->gBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);    // Depth Linear Color Buffer
    app->gBuffer.AttachDepthTexture(app->displaySize);                                  // Depth Attachment

    app->gBuffer.SetColorBuffers(); // Set color buffers with glDrawBuffers
    app->gBuffer.Unbind();

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
    app->gBuffer.SetColorBuffers(); // Set color buffers with glDrawBuffers
    app->gBuffer.Unbind();

    app->screenQuad.VAO = CreateQuad();
    app->screenQuad.shaderHandle = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Quad_Shader_D.glsl", "SCREEN_QUAD")].handle;
    app->screenQuad.renderTarget = app->screenQuad.FBO.colorAttachmentHandles[0];
    glUseProgram(app->screenQuad.shaderHandle);
    glUniform1i(glGetUniformLocation(app->screenQuad.shaderHandle, "uRenderTarget"), 0);

    // SHADERS & UNIFORM TEXTURES //
    app->defaultProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/Default_Shader_D.glsl", "DEFERRED_GEOMETRY_DEFAULT");
    app->lightCasterProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/LightCaster_Shader.glsl", "LIGHT_CASTER");

    u32 texturedAlbProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/GeometryPassAlb_Shader_D.glsl", "DEFERRED_GEOMETRY_ALBEDO");
    app->shaderPrograms[texturedAlbProgramID].Bind();
    glUniform1i(glGetUniformLocation(app->shaderPrograms[texturedAlbProgramID].handle, "uMaterial.albedo"), 0);

    u32 texturedAlbSpecProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/GeometryPassAlbSpec_Shader_D.glsl", "DEFERRED_GEOMETRY_ALBEDO_SPECULAR");
    app->shaderPrograms[texturedAlbSpecProgramID].Bind();
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
        ImGui::DragFloat("FOV", &app->camera.FOV, 1.0f, 5.0f, 120.0f);

        ImGui::Separator();
        ImGui::Text("Entities");
        for (u32 i = 0; i < app->numEntities; ++i)
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
    
    app->camera.ProcessInput(app->input, app->displaySize, app->deltaTime);

    if (app->input.keys[K_1] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->screenQuad.FBO.colorAttachmentHandles[0];
    if (app->input.keys[K_2] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[0];
    if (app->input.keys[K_3] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[1];
    if (app->input.keys[K_4] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[2];
    if (app->input.keys[K_5] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[3];
    if (app->input.keys[K_6] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[4];
    if (app->input.keys[K_7] == BUTTON_PRESS)
        app->screenQuad.renderTarget = app->gBuffer.colorAttachmentHandles[5];

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
    app->gBuffer.Bind();

    // Scene State
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
        shader.Unbind();
    }

    app->gBuffer.Unbind();

    // DEFERRED SHADING: LIGHTING PASS //
    app->screenQuad.FBO.Bind();
    glDisable(GL_DEPTH_TEST);
    glUseProgram(app->lightPassShaderHandle);

    glBindVertexArray(app->screenQuad.VAO);

    // Set the uniform textures from the G-Buffer
    for (u32 i = 0; i < app->gBuffer.colorAttachmentHandles.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, app->gBuffer.colorAttachmentHandles[i]);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    // SCREEN-FILLING QUAD //
    app->screenQuad.FBO.Unbind();
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(app->screenQuad.shaderHandle);

    glBindVertexArray(app->screenQuad.VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->screenQuad.renderTarget);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    glUseProgram(0);

    // LIGHTS FORWARD RENDERING //
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

    // Local Parameters //
    for (u32 i = 0; i < app->numEntities; ++i)
    {
        AlignHead(app->UBO, app->uniformBufferOffsetAlignment);

        Entity& entity = app->entities[i];
        entity.localParamOffset = app->UBO.head;

        const glm::mat4& modelMatrix = entity.modelMatrix;
        PushMat4(app->UBO, modelMatrix);

        glm::mat4 MVP = app->camera.GetViewProjectionMatrix(app->displaySize) * modelMatrix;
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