//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "AssimpLoading.h"
#include "Primitives.h"

#include "glad/glad.h"
#include "imgui-docking/imgui.h"

void Init(App* app)
{
    // CAMERA //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f), 45.0f, 1.0f, 100.0f);

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);
    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // DEFERRED SHADING //
    // Lighting Pass
    app->renderer.lightingPassShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::LIGHTING_PASS, "Assets/Shaders/LightingPass_Deferred.glsl", "DEFERRED_LIGHTING_PASS");

    // SCREEN-FILLING QUAD //
    app->renderer.screenQuad.VAO = CreateQuad();
    app->renderer.screenQuad.shaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::SCREEN_QUAD, "Assets/Shaders/Quad_Deferred.glsl", "SCREEN_QUAD");

    // SHADERS & UNIFORM TEXTURES //
    u32 defaultShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::DEFAULT, "Assets/Shaders/Default_Deferred.glsl", "DEFERRED_GEOMETRY_DEFAULT");
    app->renderer.lightCasterShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::LIGHT_CASTER, "Assets/Shaders/LightCaster.glsl", "LIGHT_CASTER");

    u32 texturedAlbShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALBEDO, "Assets/Shaders/GeometryPassAlb_Deferred.glsl", "DEFERRED_GEOMETRY_ALBEDO");
    Shader& texturedAlbShader = app->shaderPrograms[texturedAlbShaderID];
    texturedAlbShader.Bind();
    texturedAlbShader.SetUniform1i("uMaterial.albedo", 0);

    u32 texturedAlbSpecShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALB_SPEC, "Assets/Shaders/GeometryPassAlbSpec_Deferred.glsl", "DEFERRED_GEOMETRY_ALBEDO_SPECULAR");
    Shader& texturedAlbSpecShader = app->shaderPrograms[texturedAlbSpecShaderID];
    texturedAlbSpecShader.Bind();
    texturedAlbSpecShader.SetUniform1i("uMaterial.albedo", 0);
    texturedAlbSpecShader.SetUniform1i("uMaterial.specular", 1);

    app->renderer.skyboxCubeVAO = CreateSkyboxCube();
    app->renderer.skyboxShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/Skybox.glsl", "SKYBOX");

    Shader& equirectToCubemapShader = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/EquirectToCubemap.glsl", "EQUIRECT_TO_CUBEMAP")];
    Shader& irradianceConvShader = app->shaderPrograms[LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/Irradiance_Convolution.glsl", "IRRADIANCE_CONVOLUTION")];

    /*
    std::vector<std::string> cubemapFaces
    {
        "Assets/Skybox/right.jpg",
        "Assets/Skybox/left.jpg",
        "Assets/Skybox/top.jpg",
        "Assets/Skybox/bottom.jpg",
        "Assets/Skybox/front.jpg",
        "Assets/Skybox/back.jpg"
    };
    app->cubemapTextureID = LoadCubemap(cubemapFaces);
    */
    glm::uvec2 cubemapTextures = LoadCubemap(app->textures, "Assets/Skybox/little_paris_eiffel_tower_4k.hdr", equirectToCubemapShader, irradianceConvShader, app->renderer.skyboxCubeVAO);

    app->renderer.environmentMapHandle = cubemapTextures.x;
    app->renderer.irradianceMapHandle = cubemapTextures.y;

    app->renderer.ssaoShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/SSAO.glsl", "SSAO");
    app->renderer.ssaoBlurShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/SSAO_Blur.glsl", "SSAO_BLUR");

    app->renderer.Init(app);

    // MATERIALS //
    Material greyMaterial = {};
    greyMaterial.name = "Grey Material";
    greyMaterial.albedo = glm::vec3(0.4f, 0.4f, 0.4f);

    Material orangeMaterial = {};
    orangeMaterial.name = "Orange Material";
    orangeMaterial.albedo = glm::vec3(1.0f, 0.5f, 0.31f);
    orangeMaterial.reflective = glm::vec3(0.5f);

    Material blackMaterial = {};
    blackMaterial.name = "Black Material";
    blackMaterial.albedo = glm::vec3(0.1f, 0.1f, 0.1f);

    Material containerMat = {};
    containerMat.name = "Container Material";
    containerMat.albedoTextureID = LoadTexture2D(app->textures, "Assets/container_albedo.png");;
    containerMat.specularTextureID = LoadTexture2D(app->textures, "Assets/container_specular.png");

    // MODELS //
    Model* planeModel = CreatePrimitive(app, PrimitiveType::PLANE, greyMaterial);

    Model* sphereModel = CreatePrimitive(app, PrimitiveType::SPHERE, orangeMaterial);
    Model* sphereLowModel = CreatePrimitive(app, PrimitiveType::SPHERE, blackMaterial, 16, 16);

    Model* cubeModel = CreatePrimitive(app, PrimitiveType::CUBE, containerMat);
    Model* cubeModel2 = CreatePrimitive(app, PrimitiveType::CUBE, blackMaterial);

    Model* bunnyModel = LoadModel(app, "Assets/Models/Bunny/bunny.obj");
    Model* patrickModel = LoadModel(app, "Assets/Models/Patrick/patrick.obj");

    // ENTITIES //
    // Primitives
    Entity* planeEntity = CreateEntity(app, defaultShaderID, glm::vec3(0.0f, -3.4f, 0.0f), planeModel);
    planeEntity->Rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    planeEntity->Scale(35.0f);

    Entity* sphereEntity = CreateEntity(app, defaultShaderID, glm::vec3(0.0f, 1.0f, 7.0f), sphereModel);

    Entity* cubeEntity = CreateEntity(app, texturedAlbSpecShaderID, glm::vec3(5.0f, 0.0f, 7.0f), cubeModel);

    Entity* cubeEntity2 = CreateEntity(app, defaultShaderID, glm::vec3(-5.0f, 0.0f, 7.0f), cubeModel2);

    // 3D Models
    Entity* bunnyEntity = CreateEntity(app, defaultShaderID, glm::vec3(0.0f, -3.5f, 7.0f), bunnyModel);
    bunnyEntity->Scale(1.5f);

    CreateEntity(app, texturedAlbShaderID, glm::vec3(-6.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(0.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(6.0f, 0.0f, 0.0f), patrickModel);

    CreateEntity(app, texturedAlbShaderID, glm::vec3(-6.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(0.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(6.0f, 0.0f, -4.0f), patrickModel);

    CreateEntity(app, texturedAlbShaderID, glm::vec3(-6.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(0.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, texturedAlbShaderID, glm::vec3(6.0f, 0.0f, -8.0f), patrickModel);

    app->firstLightEntityID = app->entities.size();

    // LIGHTS //
    CreateDirectionalLight(app, glm::vec3(0.0f, -2.0f, 15.0f), glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.0f), cubeModel2, 0.5f);

    srand(14);
    for (unsigned int i = 0; i <= 8; i++)
    {
        // Random Position
        float xPos = static_cast<float>(((rand() % 100) / 70.0f) * 9.0f - 3.0f);
        float yPos = static_cast<float>(((rand() % 100) / 80.0f) * 6.0f + 6.0f);
        float zPos = static_cast<float>(((rand() % 100) / 25.0f) * 6.0f - 16.0f);

        // Random Color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        glm::vec3 color = glm::vec3(rColor, gColor, bColor);

        CreatePointLight(app, glm::vec3(xPos, yPos, zPos), color, sphereLowModel, 1.0f, 0.1f);
    }

    CreatePointLight(app, glm::vec3(-6.0f, 0.0f, 10.0f), glm::vec3(0.6f), sphereLowModel, 0.5f, 0.1f);
    CreatePointLight(app, glm::vec3(6.0f, 0.0f, 10.0f), glm::vec3(0.6f), sphereLowModel, 1.0f, 0.1f);

    // IMGUI SETTINGS //
    // ImGui Windows
    app->openGLGui.open = false;
    app->rendererOptions.open = true;
    app->sceneGui = false;
    app->performanceGui = false;

    // ImGui OpenGL Info
    app->openGLGui.version = "Version: " + std::string((const char*)glGetString(GL_VERSION));
    app->openGLGui.renderer = "Renderer: " + std::string((const char*)glGetString(GL_RENDERER));
    app->openGLGui.vendor = "Vendor: " + std::string((const char*)glGetString(GL_VENDOR));
    app->openGLGui.glslVersion = "GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &app->openGLGui.numExtensions);
    app->openGLGui.extensions.reserve(app->openGLGui.numExtensions);
    for (int i = 0; i < app->openGLGui.numExtensions; ++i)
    {
        app->openGLGui.extensions.emplace_back((const char*)glGetStringi(GL_EXTENSIONS, GLuint(i)));
    }

    // ImGui Render Target Selection Combo
    app->rendererOptions.renderTargets.push_back("FINAL COLOR");
    app->rendererOptions.renderTargets.push_back("DEPTH");
    app->rendererOptions.renderTargets.push_back("POSITION");
    app->rendererOptions.renderTargets.push_back("NORMAL");
    app->rendererOptions.renderTargets.push_back("ALBEDO");
    app->rendererOptions.renderTargets.push_back("SPECULAR");
    app->rendererOptions.renderTargets.push_back("REFLECTIVE + SHININESS");

    // ImGui Skybox Options
    app->rendererOptions.activeSkybox = true;
    app->rendererOptions.activeIrradiance = true;
    app->rendererOptions.activeReflection = false;
    app->rendererOptions.activeRefraction = false;

    // ImGui SSAO Options
    app->rendererOptions.activeSSAO = true;
    app->rendererOptions.activeRangeCheck = true;
    app->rendererOptions.activeSSAOBlur = true;
    app->rendererOptions.ssaoRadius = 0.5f;
    app->rendererOptions.ssaoBias = 0.025f;
    app->rendererOptions.ssaoPower = 1.0f;

    // ENGINE COUNT OF ENTITIES & LIGHTS //
    app->numEntities = app->entities.size();
    app->numLights = app->lights.size();

    // OPENGL GLOBAL STATE //
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void ImGuiRender(App* app)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Engine"))
        {
            ImGui::MenuItem("OpenGL", NULL, &app->openGLGui.open);
            ImGui::MenuItem("Renderer", NULL, &app->rendererOptions.open);
            ImGui::MenuItem("Scene", NULL, &app->sceneGui);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Performance", NULL, &app->performanceGui);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (app->openGLGui.open)
    {
        ImGui::Begin("OpenGL", &app->openGLGui.open);

        ImGui::Text(app->openGLGui.version.c_str());
        ImGui::Text(app->openGLGui.renderer.c_str());
        ImGui::Text(app->openGLGui.vendor.c_str());
        ImGui::Text(app->openGLGui.glslVersion.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Extensions Supported:");
        for (int i = 0; i < app->openGLGui.numExtensions; ++i)
            ImGui::Text(app->openGLGui.extensions[i].c_str());

        ImGui::End();
    }

    if (app->rendererOptions.open)
    {
        ImGui::Begin("Renderer", &app->rendererOptions.open);

        ImGui::Text("Rendering Mode:");
        ImGui::SameLine();
        ImGui::SetCursorPosY((ImGui::GetFontSize() * 1.5f + ImGui::GetStyle().FramePadding.y * 2.0f));
        if (ImGui::Button(app->forwardRendering ? "Switch to Deferred" : "Switch to Forward"))
        {
            app->forwardRendering = !app->forwardRendering;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Environment Mapping Options");
        if (ImGui::Checkbox("Skybox", &app->rendererOptions.activeSkybox))
        {
            app->rendererOptions.activeIrradiance = false;
            app->rendererOptions.activeReflection = false;
            app->rendererOptions.activeRefraction = false;
        }
        if (app->rendererOptions.activeSkybox)
        {
            ImGui::Checkbox("Irradiance", &app->rendererOptions.activeIrradiance);
            ImGui::Checkbox("Reflection", &app->rendererOptions.activeReflection);
            ImGui::Checkbox("Refraction", &app->rendererOptions.activeRefraction);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("SSAO Options");
        ImGui::Checkbox("SSAO", &app->rendererOptions.activeSSAO);
        if (app->rendererOptions.activeSSAO)
        {
            ImGui::Checkbox("Range Check", &app->rendererOptions.activeRangeCheck);
            ImGui::Checkbox("Blur", &app->rendererOptions.activeSSAOBlur);
            ImGui::DragFloat("Radius", &app->rendererOptions.ssaoRadius, 0.05f, 0.25f, 2.0f);
            ImGui::DragFloat("Bias", &app->rendererOptions.ssaoBias, 0.001f, 0.01f, 0.1f);
            ImGui::DragFloat("Power", &app->rendererOptions.ssaoPower, 0.1f, 1.0f, 10.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Render Target");
        static const char* preview = "FINAL COLOR";
        if (ImGui::BeginCombo("", preview))
        {
            for (int i = 0; i < app->rendererOptions.renderTargets.size(); ++i)
            {
                bool isSelected = (preview == app->rendererOptions.renderTargets[i]);
                if (ImGui::Selectable(app->rendererOptions.renderTargets[i], isSelected))
                {
                    preview = app->rendererOptions.renderTargets[i];
                    if (i == 0)
                        app->renderer.screenQuad.currentRenderTarget = app->renderer.screenQuad.FBO.colorAttachmentHandles[0];
                    else if (i == 1)
                        app->renderer.screenQuad.currentRenderTarget = app->renderer.GBuffer.depthAttachment;
                    else
                        app->renderer.screenQuad.currentRenderTarget = app->renderer.GBuffer.colorAttachmentHandles[i64(i) - 2];
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }

    if (app->sceneGui)
    {
        ImGui::Begin("Scene", &app->sceneGui);

        ImGui::Text("Camera");
        ImGui::DragFloat3("Position", &app->camera.position[0]);
        ImGui::DragFloat("Speed", &app->camera.speed, 1.0f, 1.0f, 10.0f);
        ImGui::DragFloat("Zoom", &app->camera.FOV, 1.0f, 5.0f, 45.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Lights");
        ImGui::DragFloat3("Directional Light Color", &app->lights[0].color[0], 0.05f, 0.0f, 1.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        for (u32 i = 1; i < app->numLights - 1; ++i)
            ImGui::DragFloat3(std::string("Point Light " + std::to_string(i) + " Color").c_str(), &app->lights[i].color[0], 0.05f, 0.0f, 1.0f);

        ImGui::End();
    }

    if (app->performanceGui)
    {
        ImGui::Begin("Performance", &app->performanceGui);
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
    
    app->camera.Update(app->input, app->displaySize, app->deltaTime);

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

    if (app->forwardRendering)
    {
        app->renderer.ForwardRender(app);
    }
    else
    {
        // DEFERRED SHADING: GEOMETRY PASS //
        app->renderer.GBuffer.Bind();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
                switch (shader.type)
                {
                case ShaderType::DEFAULT:
                {
                    // Material
                    shader.SetUniform3f("uMaterial.albedo", meshMaterial.albedo);
                    shader.SetUniform3f("uMaterial.specular", meshMaterial.specular);
                    shader.SetUniform3f("uMaterial.reflective", meshMaterial.reflective);
                    shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess);
                }
                break;
                case ShaderType::TEXTURED_ALBEDO:
                {
                    // Albedo Map
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                    // Material
                    shader.SetUniform3f("uMaterial.specular", meshMaterial.specular);
                    shader.SetUniform3f("uMaterial.reflective", meshMaterial.reflective);
                    shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess);
                }
                break;
                case ShaderType::TEXTURED_ALB_SPEC:
                {
                    // Albedo Map
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);

                    // Specular Map
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.specularTextureID].handle);

                    // Material
                    shader.SetUniform3f("uMaterial.reflective", meshMaterial.reflective);
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
        app->renderer.BindDefaultFramebuffer();

        if (app->rendererOptions.activeSSAO)
        {
            // SSAO //
            app->renderer.ssaoBuffer.Bind();
            glDisable(GL_BLEND);
            glClear(GL_COLOR_BUFFER_BIT);

            Shader& SSAOShader = app->shaderPrograms[app->renderer.ssaoShaderID];
            SSAOShader.Bind();

            SSAOShader.SetUniformMat4("uProjection", app->camera.GetProjectionMatrix(app->displaySize));
            SSAOShader.SetUniformMat4("uView", app->camera.GetViewMatrix(app->displaySize));
            SSAOShader.SetUniform2f("uDisplaySize", glm::vec2(app->displaySize.x, app->displaySize.y));

            SSAOShader.SetUniform1i("uSSAOptions.uRangeCheck", app->rendererOptions.activeRangeCheck);
            SSAOShader.SetUniform1f("uSSAOptions.uRadius", app->rendererOptions.ssaoRadius);
            SSAOShader.SetUniform1f("uSSAOptions.uBias", app->rendererOptions.ssaoBias);
            SSAOShader.SetUniform1f("uSSAOptions.uPower", app->rendererOptions.ssaoPower);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->renderer.GBuffer.colorAttachmentHandles[0]); // Position
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, app->renderer.GBuffer.colorAttachmentHandles[1]); // Normal
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, app->renderer.GBuffer.depthAttachment); // Depth
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, app->renderer.noiseTextureHandle); // SSAO Noise Texture

            glBindVertexArray(app->renderer.screenQuad.VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            glEnable(GL_BLEND);
            SSAOShader.Unbind();
            app->renderer.BindDefaultFramebuffer();

            // SSAO Blur
            app->renderer.ssaoBlurBuffer.Bind();
            glDisable(GL_BLEND);
            glClear(GL_COLOR_BUFFER_BIT);

            Shader& SSAOBlurShader = app->shaderPrograms[app->renderer.ssaoBlurShaderID];
            SSAOBlurShader.Bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->renderer.ssaoBuffer.colorAttachmentHandles[0]); // SSAO Color Texture

            glBindVertexArray(app->renderer.screenQuad.VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            glEnable(GL_BLEND);
            SSAOBlurShader.Unbind();
            app->renderer.BindDefaultFramebuffer();
        }

        // DEFERRED SHADING: LIGHTING PASS //
        app->renderer.screenQuad.FBO.Bind();

        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Shader& lightingPassShader = app->shaderPrograms[app->renderer.lightingPassShaderID];
        lightingPassShader.Bind();

        // Set the uniform textures from the G-Buffer
        for (u32 i = 0; i < app->renderer.GBuffer.colorAttachmentHandles.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, app->renderer.GBuffer.colorAttachmentHandles[i]);
        }

        // Environment Map
        glActiveTexture(GL_TEXTURE0 + app->renderer.GBuffer.colorAttachmentHandles.size());
        glBindTexture(GL_TEXTURE_CUBE_MAP, app->renderer.environmentMapHandle);

        // Irradiance Map
        glActiveTexture(GL_TEXTURE1 + app->renderer.GBuffer.colorAttachmentHandles.size());
        glBindTexture(GL_TEXTURE_CUBE_MAP, app->renderer.irradianceMapHandle);

        // SSAO Color
        glActiveTexture(GL_TEXTURE2 + app->renderer.GBuffer.colorAttachmentHandles.size());
        glBindTexture(GL_TEXTURE_2D, app->rendererOptions.activeSSAOBlur ? app->renderer.ssaoBlurBuffer.colorAttachmentHandles[0] : app->renderer.ssaoBuffer.colorAttachmentHandles[0]);

        lightingPassShader.SetUniform1i("uRendererOptions.uActiveIrradiance", app->rendererOptions.activeIrradiance);
        lightingPassShader.SetUniform1i("uRendererOptions.uActiveReflection", app->rendererOptions.activeReflection);
        lightingPassShader.SetUniform1i("uRendererOptions.uActiveRefraction", app->rendererOptions.activeRefraction);
        lightingPassShader.SetUniform1i("uRendererOptions.uActiveSSAO", app->rendererOptions.activeSSAO);

        glBindVertexArray(app->renderer.screenQuad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        lightingPassShader.Unbind();

        // SCREEN-FILLING QUAD //
        app->renderer.BindDefaultFramebuffer();

        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Shader& screenQuadShader = app->shaderPrograms[app->renderer.screenQuad.shaderID];
        screenQuadShader.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->renderer.screenQuad.currentRenderTarget);

        glBindVertexArray(app->renderer.screenQuad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        screenQuadShader.Unbind();

        // FORWARD SHADING: LIGHTS //
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, app->renderer.GBuffer.handle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, app->displaySize.x, app->displaySize.y, 0, 0, app->displaySize.x, app->displaySize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        app->renderer.BindDefaultFramebuffer();

        Shader& lightCasterShader = app->shaderPrograms[app->renderer.lightCasterShaderID];
        lightCasterShader.Bind();
        u32 lightID = 0;
        for (u32 i = app->firstLightEntityID; i < app->numEntities; ++i)
        {
            Entity& lightEntity = app->entities[i];

            glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO.handle, lightEntity.localParamOffset, lightEntity.localParamSize);

            Model* model = lightEntity.model;

            u32 vao = FindVAO(model, 0, lightCasterShader);
            glBindVertexArray(vao);

            lightCasterShader.SetUniform1ui("uLightID", lightID);

            Mesh& mesh = model->meshes[0];
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

            glBindVertexArray(0);
            lightID++;
        }
        lightCasterShader.Unbind();
    }

    if (app->rendererOptions.activeSkybox)
    {
        // SKYBOX //
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
        Shader& skyboxShader = app->shaderPrograms[app->renderer.skyboxShaderID];
        skyboxShader.Bind();
        glm::mat4 view = glm::mat4(glm::mat3(app->camera.GetViewMatrix(app->displaySize))); // remove translation from the view matrix
        skyboxShader.SetUniformMat4("uView", view);
        skyboxShader.SetUniformMat4("uProjection", app->camera.GetProjectionMatrix(app->displaySize));

        // Skybox Cube
        glBindVertexArray(app->renderer.skyboxCubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, app->renderer.environmentMapHandle);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }
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
        PushVec4(app->UBO, light.lightVector);
        PushVec3(app->UBO, light.color);
        PushFloat(app->UBO, light.constant);
    }
    app->globalParamSize = app->UBO.head - app->globalParamOffset;

    glm::mat4 VPMatrix = app->camera.GetProjectionMatrix(app->displaySize) * app->camera.GetViewMatrix(app->displaySize);
    // Local Parameters //
    for (u32 i = 0; i < app->numEntities; ++i)
    {
        AlignHead(app->UBO, app->uniformBufferOffsetAlignment);

        Entity& entity = app->entities[i];
        entity.localParamOffset = app->UBO.head;

        glm::mat4 modelMatrix = entity.GetModelMatrix();
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

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 color, Model* model, float constant, float scale)
{
    Entity lightEntity = Entity(app->renderer.lightCasterShaderID, position, model);
    lightEntity.Scale(scale);
    app->entities.push_back(lightEntity);

    Light light = { glm::vec4(position, 1.0f), color, constant };
    app->lights.push_back(light);
}

void CreateDirectionalLight(App* app, glm::vec3 entityPosition, glm::vec3 direction, glm::vec3 color, Model* model, float scale)
{
    Entity lightEntity = Entity(app->renderer.lightCasterShaderID, entityPosition, model);
    lightEntity.Scale(scale);
    app->entities.push_back(lightEntity);

    Light light = { glm::vec4(direction, 0.0f), color, 1.0f };
    app->lights.push_back(light);
}

float Lerp(float a, float b, float f)
{
    return a + f * (b - a);
}