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
    // IMGUI WINDOWS //
    app->debugInfo = false;
    app->openGLStatus = false;
    app->sceneInfo = false;

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

    // OPENGL UNIFORM BUFFER //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);

    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // CAMERA & PROJECTION //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f));

    app->projection = glm::mat4(1.0f);
    app->projection = glm::perspective(glm::radians(45.0f), float(app->displaySize.x) / float(app->displaySize.y), 0.1f, 100.0f);

    // SHADERS & UNIFORM TEXTURE LOCATIONS //
    app->defaultProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/DefaultShader.glsl", "DEFAULT_MESH");
    app->lightProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/LightShader.glsl", "LIGHT_CASTER");

    u32 modelProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/ModelShader.glsl", "MODEL_MESH");
    app->meshTextureAlbedoLocation = glGetUniformLocation(app->shaderPrograms[modelProgramID].handle, "uMaterial.albedo");

    u32 cubeProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/CubeShader.glsl", "CUBE_MESH");
    app->cubeTextureAlbedoLocation = glGetUniformLocation(app->shaderPrograms[cubeProgramID].handle, "uMaterial.albedo");
    app->cubeTextureSpecularLocation = glGetUniformLocation(app->shaderPrograms[cubeProgramID].handle, "uMaterial.specular");

    //app->quadProgramID = LoadShaderProgram(app->shaderPrograms, "Assets/Shaders/QuadShader.glsl", "SCREEN_QUAD");
    //app->quadTextureLocation = glGetUniformLocation(app->shaderPrograms[app->quadProgramID].handle, "uScreenTexture");

    // TEXTURES //
    u32 diceTexIdx = LoadTexture2D(app->textures, "Assets/dice.png");
    u32 whiteTexIdx = LoadTexture2D(app->textures, "Assets/color_white.png");
    u32 blackTexIdx = LoadTexture2D(app->textures, "Assets/color_black.png");
    u32 normalTexIdx = LoadTexture2D(app->textures, "Assets/color_normal.png");
    u32 magentaTexIdx = LoadTexture2D(app->textures, "Assets/color_magenta.png");

    u32 containerAlbedoTexID = LoadTexture2D(app->textures, "Assets/Container/container_albedo.png");
    u32 containerSpecularID = LoadTexture2D(app->textures, "Assets/Container/container_specular.png");

    // MATERIALS //
    Material defaultMat = {};
    defaultMat.name = "Default Material";
    defaultMat.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
    defaultMat.specular = glm::vec3(0.5f);
    defaultMat.emissive = glm::vec3(0.0f);
    defaultMat.shininess = 32.0f;

    Material defaultMat2 = {};
    defaultMat2.name = "Default Material 2";
    defaultMat2.diffuse = glm::vec3(0.5f, 1.0f, 0.31f);
    defaultMat2.specular = glm::vec3(0.5f);
    defaultMat2.emissive = glm::vec3(0.0f);
    defaultMat2.shininess = 32.0f;

    Material containerMat = {};
    containerMat.name = "Default Material";
    containerMat.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
    containerMat.specular = glm::vec3(0.5f);
    containerMat.emissive = glm::vec3(0.0f);
    containerMat.shininess = 32.0f;
    containerMat.albedoTextureID = containerAlbedoTexID;
    containerMat.specularTextureID = containerSpecularID;

    // MODELS //
    Model* planeModel = CreatePrimitive(app, PrimitiveType::PLANE, defaultMat);

    Model* sphereModel = CreatePrimitive(app, PrimitiveType::SPHERE, defaultMat);
    Model* sphereLowModel = CreatePrimitive(app, PrimitiveType::SPHERE, defaultMat, 16, 32);

    Model* cubeModel = CreatePrimitive(app, PrimitiveType::CUBE, containerMat);
    Model* cubeModel2 = CreatePrimitive(app, PrimitiveType::CUBE, defaultMat2);

    Model* patrickModel = LoadModel(app, "Assets/Patrick/patrick.obj");

    // ENTITIES //
    // Primitives
    Entity* planeEntity = CreateEntity(app, EntityType::PRIMITIVE, app->defaultProgramID, glm::vec3(0.0f, -5.0f, 0.0f), planeModel);
    planeEntity->modelMatrix = glm::rotate(planeEntity->modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    planeEntity->modelMatrix = glm::scale(planeEntity->modelMatrix, glm::vec3(25.0f));

    Entity* sphereEntity = CreateEntity(app, EntityType::PRIMITIVE, app->defaultProgramID, glm::vec3(0.0f, 0.0f, 3.0f), sphereModel);

    Entity* cubeEntity = CreateEntity(app, EntityType::PRIMITIVE, app->defaultProgramID, glm::vec3(5.0f, 0.0f, 2.0f), cubeModel2);

    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    for (u32 i = 0; i < 10; ++i)
    {
        Entity* cubeEntity = CreateEntity(app, EntityType::PRIMITIVE_CUBE, cubeProgramID, cubePositions[i], cubeModel);
        float angle = 20.0f * i;
        cubeEntity->modelMatrix = glm::rotate(cubeEntity->modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    }

    // Models
    Entity* patrickEntity = CreateEntity(app, EntityType::MODEL, modelProgramID, glm::vec3(0.0f, 0.0f, -5.0f), patrickModel);

    // LIGHTS //
    CreatePointLight(app, glm::vec3(3.0f, 1.0f, -2.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);

    CreatePointLight(app, glm::vec3(-6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 0.1f, sphereLowModel, 0.1f);
    CreatePointLight(app, glm::vec3(6.0f, -4.5f, 10.0f), glm::vec3(0.2f), glm::vec3(0.6f), glm::vec3(1.0f), 1.0f, sphereLowModel, 0.1f);

    //CreateDirectionalLight(app, glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.2f), glm::vec3(0.5f), glm::vec3(1.0f));

    // ENGINE SETTINGS //
    app->numLights = app->lights.size();
    app->numEntities = app->entities.size();
    app->mode = RenderMode::TEXTURE_MESH;

    // OPENGL GLOBAL STATE //
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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

        ImGui::Separator();

        ImGui::Text("Entities");
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
        }
    }
}

void Render(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    UpdateUniformBuffer(app);

    switch (app->mode)
    {
    case RenderMode::TEXTURE_MESH:
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->UBO.handle, app->globalParamOffset, app->globalParamSize);

        for (u32 i = 0; i < app->numEntities; ++i)
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
                switch (entity.type)
                {
                case EntityType::MODEL:
                {
                    // Diffuse Map
                    glUniform1i(app->meshTextureAlbedoLocation, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                    // Material
                    glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.specular"), 1, &meshMaterial.specular[0]);
                    glUniform1f(glGetUniformLocation(shader.handle, "uMaterial.shininess"), meshMaterial.shininess);
                }
                break;
                case EntityType::PRIMITIVE:
                {
                    // Diffuse Map
                    //glUniform1i(app->cubeTextureAlbedoLocation, 0);
                    //glActiveTexture(GL_TEXTURE0);
                    //glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                    // Material
                    glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.diffuse"), 1, &meshMaterial.diffuse[0]);
                    glUniform3fv(glGetUniformLocation(shader.handle, "uMaterial.specular"), 1, &meshMaterial.specular[0]);
                    glUniform1f(glGetUniformLocation(shader.handle, "uMaterial.shininess"), meshMaterial.shininess);
                }
                break;
                case EntityType::PRIMITIVE_CUBE:
                {
                    // Diffuse Map
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
                    glUniform1i(app->cubeTextureAlbedoLocation, 0);

                    // Specular Map
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.specularTextureID].handle);
                    glUniform1i(app->cubeTextureSpecularLocation, 1);

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
    }
    break;

    case RenderMode::QUAD:
    {
        ShaderProgram& screenQuadProgram = app->shaderPrograms[app->quadProgramID];
        Model* model = app->quad.model;

        glUseProgram(screenQuadProgram.handle);

        u32 vao = FindVAO(model, 0, screenQuadProgram);
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

        glm::mat4 MVP = app->projection * app->camera.GetViewMatrix() * modelMatrix;
        PushMat4(app->UBO, MVP);

        entity.localParamSize = app->UBO.head - entity.localParamOffset;
    }
    UnmapBuffer(app->UBO);
}

Entity* CreateEntity(App* app, EntityType type, u32 shaderID, glm::vec3 position, Model* model)
{
    Entity entity = Entity(type, shaderID, position, model);
    app->entities.push_back(entity);

    return &app->entities[app->entities.size() - 1u];
}

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, Model* model, float scale)
{
    Entity lightEntity = Entity(EntityType::LIGHT, app->lightProgramID, position, model);
    lightEntity.modelMatrix = glm::scale(lightEntity.modelMatrix, glm::vec3(scale));
    app->entities.push_back(lightEntity);

    Light light = Light(LightType::POINT, position, glm::vec3(0.0f), ambient, diffuse, specular, constant);
    app->lights.push_back(light);
}

void CreateDirectionalLight(App* app, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
    Light light = Light(LightType::DIRECTIONAL, glm::vec3(0.0f), direction, ambient, diffuse, specular, 1.0f);
    app->lights.push_back(light);
}