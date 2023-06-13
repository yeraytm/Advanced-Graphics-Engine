//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "Shader.h"
#include "AssimpLoading.h"
#include "Primitives.h"

#include "Timer.h"

#include "glad/glad.h"
#include "imgui-docking/imgui.h"

void Init(App* app)
{
    // RENDERING MODE //
    app->rendererOptions.forwardRendering = false;

    // IMGUI SETTINGS //
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
    app->rendererOptions.ssaoKernelSize = 64;
    app->rendererOptions.ssaoNoiseSize = 16;

    // CAMERA //
    app->camera = Camera(glm::vec3(0.0f, 3.0f, 20.0f), 45.0f, 1.0f, 100.0f);

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferOffsetAlignment);
    app->UBO = CreateConstantBuffer(maxUniformBlockSize);

    // SCREEN-FILLING QUAD //
    app->renderer.screenQuad.VAO = CreateQuad();
    app->renderer.screenQuad.shaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::SCREEN_QUAD, "Assets/Shaders/Quad_Deferred.glsl", "SCREEN_QUAD");

    app->renderer.lightCasterShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::LIGHT_CASTER, "Assets/Shaders/LightCaster.glsl", "LIGHT_CASTER");

    // DEFERRED RENDERING: Lighting Pass //
    app->renderer.lightingPassShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::LIGHTING_PASS, "Assets/Shaders/LightingPass_Deferred.glsl", "DEFERRED_LIGHTING_PASS");

    // SHADERS FORWARD //
    app->renderer.forwardShadersID[0] = LoadShaderProgram(app->shaderPrograms, ShaderType::DEFAULT, "Assets/Shaders/Forward/Default_Forward.glsl", "FORWARD_DEFAULT");
    Shader& defaultShaderF = app->shaderPrograms[app->renderer.forwardShadersID[0]];
    defaultShaderF.Bind();
    defaultShaderF.SetUniform1i("uEnvironmentMap", 0);
    defaultShaderF.SetUniform1i("uIrradianceMap", 1);

    app->renderer.forwardShadersID[1] = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALBEDO, "Assets/Shaders/Forward/Albedo_Forward.glsl", "FORWARD_ALBEDO");
    Shader& texturedAlbShaderF = app->shaderPrograms[app->renderer.forwardShadersID[1]];
    texturedAlbShaderF.Bind();
    texturedAlbShaderF.SetUniform1i("uMaterial.albedo", 0);
    texturedAlbShaderF.SetUniform1i("uEnvironmentMap", 1);
    texturedAlbShaderF.SetUniform1i("uIrradianceMap", 2);

    app->renderer.forwardShadersID[2] = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALB_SPEC, "Assets/Shaders/Forward/AlbedoSpecular_Forward.glsl", "FORWARD_ALBEDO_SPECULAR");
    Shader& texturedAlbSpecShaderF = app->shaderPrograms[app->renderer.forwardShadersID[2]];
    texturedAlbSpecShaderF.Bind();
    texturedAlbSpecShaderF.SetUniform1i("uMaterial.albedo", 0);
    texturedAlbSpecShaderF.SetUniform1i("uMaterial.specular", 1);
    texturedAlbSpecShaderF.SetUniform1i("uEnvironmentMap", 2);
    texturedAlbSpecShaderF.SetUniform1i("uIrradianceMap", 3);

    // SHADERS DEFERRED //
    app->renderer.deferredShadersID[0] = LoadShaderProgram(app->shaderPrograms, ShaderType::DEFAULT, "Assets/Shaders/Default_Deferred.glsl", "DEFERRED_GEOMETRY_DEFAULT");

    app->renderer.deferredShadersID[1] = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALBEDO, "Assets/Shaders/GeometryPassAlb_Deferred.glsl", "DEFERRED_GEOMETRY_ALBEDO");
    Shader& texturedAlbShaderD = app->shaderPrograms[app->renderer.deferredShadersID[1]];
    texturedAlbShaderD.Bind();
    texturedAlbShaderD.SetUniform1i("uMaterial.albedo", 0);

    app->renderer.deferredShadersID[2] = LoadShaderProgram(app->shaderPrograms, ShaderType::TEXTURED_ALB_SPEC, "Assets/Shaders/GeometryPassAlbSpec_Deferred.glsl", "DEFERRED_GEOMETRY_ALBEDO_SPECULAR");
    Shader& texturedAlbSpecShaderD = app->shaderPrograms[app->renderer.deferredShadersID[2]];
    texturedAlbSpecShaderD.Bind();
    texturedAlbSpecShaderD.SetUniform1i("uMaterial.albedo", 0);
    texturedAlbSpecShaderD.SetUniform1i("uMaterial.specular", 1);

    // SKYBOX //
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
    glm::uvec2 cubemapTextures = LoadCubemap(app->textures, "Assets/Skybox/lilienstein_4k.hdr", equirectToCubemapShader, irradianceConvShader, app->renderer.skyboxCubeVAO);

    app->renderer.environmentMapHandle = cubemapTextures.x;
    app->renderer.irradianceMapHandle = cubemapTextures.y;

    // SSAO //
    app->renderer.ssaoShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/SSAO.glsl", "SSAO");
    app->renderer.ssaoBlurShaderID = LoadShaderProgram(app->shaderPrograms, ShaderType::OTHER, "Assets/Shaders/SSAO_Blur.glsl", "SSAO_BLUR");

    // MATERIALS //
    Material greyMaterial = {};
    greyMaterial.name = "Grey Material";
    greyMaterial.albedo = glm::vec3(0.4f, 0.4f, 0.4f);
    greyMaterial.reflective = glm::vec3(0.8f);

    Material orangeMaterial = {};
    orangeMaterial.name = "Orange Material";
    orangeMaterial.albedo = glm::vec3(1.0f, 0.5f, 0.31f);
    orangeMaterial.reflective = glm::vec3(0.5f);

    Material blueMaterial = {};
    blueMaterial.name = "Blue Material";
    blueMaterial.albedo = glm::vec3(0.0f, 0.0f, 0.6f);
    blueMaterial.reflective = glm::vec3(0.5f);

    Material blackMaterial = {};
    blackMaterial.name = "Black Material";
    blackMaterial.albedo = glm::vec3(0.1f, 0.1f, 0.1f);
    blackMaterial.reflective = glm::vec3(0.5f);

    Material containerMat = {};
    containerMat.name = "Container Material";
    containerMat.albedoTextureID = LoadTexture2D(app->textures, "Assets/container_albedo.png");;
    containerMat.specularTextureID = LoadTexture2D(app->textures, "Assets/container_specular.png");

    // MODELS //
    Model* planePrimitive = CreatePrimitive(app, PrimitiveType::PLANE, greyMaterial);

    Model* spherePrimitive1 = CreatePrimitive(app, PrimitiveType::SPHERE, orangeMaterial);
    Model* spherePrimitive2 = CreatePrimitive(app, PrimitiveType::SPHERE, blueMaterial);
    Model* sphereLowPrimitive = CreatePrimitive(app, PrimitiveType::SPHERE, blackMaterial, 16, 16);

    Model* containerPrimitive = CreatePrimitive(app, PrimitiveType::CUBE, containerMat);
    Model* cubePrimitive = CreatePrimitive(app, PrimitiveType::CUBE, blackMaterial);

    Model* bunnyModel = LoadModel(app, "Assets/Models/Bunny/bunny.obj");
    Model* patrickModel = LoadModel(app, "Assets/Models/Patrick/patrick.obj");
    Model* backpackModel = LoadModel(app, "Assets/Models/Backpack/backpack.obj", false);

    // ENTITIES //
    // Primitives
    Entity* planeEntity = CreateEntity(app, app->renderer.deferredShadersID[0], glm::vec3(0.0f, -3.4f, 0.0f), planePrimitive);
    planeEntity->Rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    planeEntity->Scale(35.0f);

    Entity* sphereEntity1 = CreateEntity(app, app->renderer.deferredShadersID[0], glm::vec3(10.0f, 1.0f, -4.0f), spherePrimitive1);
    Entity* sphereEntity2 = CreateEntity(app, app->renderer.deferredShadersID[0], glm::vec3(-10.0f, 1.0f, -4.0f), spherePrimitive2);
    Entity* containerEntity = CreateEntity(app, app->renderer.deferredShadersID[2], glm::vec3(-7.0f, 0.0f, 7.0f), containerPrimitive);
    Entity* cubeEntity = CreateEntity(app, app->renderer.deferredShadersID[0], glm::vec3(-11.0f, 0.0f, 7.0f), cubePrimitive);

    // 3D Models
    Entity* bunnyEntity = CreateEntity(app, app->renderer.deferredShadersID[0], glm::vec3(7.0f, -3.5f, 7.0f), bunnyModel);
    bunnyEntity->Scale(1.5f);

    Entity* backpackEntity = CreateEntity(app, app->renderer.deferredShadersID[2], glm::vec3(0.0f, 1.0f, 7.0f), backpackModel);

    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(-6.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(0.0f, 0.0f, 0.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(6.0f, 0.0f, 0.0f), patrickModel);

    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(-6.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(0.0f, 0.0f, -4.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(6.0f, 0.0f, -4.0f), patrickModel);

    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(-6.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(0.0f, 0.0f, -8.0f), patrickModel);
    CreateEntity(app, app->renderer.deferredShadersID[1], glm::vec3(6.0f, 0.0f, -8.0f), patrickModel);

    app->firstLightEntityID = app->entities.size();

    // LIGHTS //
    CreateDirectionalLight(app, glm::vec3(0.0f, -2.0f, 15.0f), glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.0f), cubePrimitive, 0.5f);

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

        CreatePointLight(app, glm::vec3(xPos, yPos, zPos), color, sphereLowPrimitive, 1.0f, 0.1f);
    }

    CreatePointLight(app, glm::vec3(-6.0f, 1.0f, 14.0f), glm::vec3(0.9f, 0.0f, 0.0f), sphereLowPrimitive, 0.5f, 0.1f);
    CreatePointLight(app, glm::vec3(6.0f, 1.0f, 14.0f), glm::vec3(0.0f, 0.9f, 0.0f), sphereLowPrimitive, 1.0f, 0.1f);

    // ENGINE COUNT OF ENTITIES & LIGHTS //
    app->numEntities = app->entities.size();
    app->numLights = app->lights.size();

    // RENDERER INIT //
    app->renderer.Init(app);

    // OPENGL GLOBAL STATE //
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void ImGuiRender(App* app)
{
    static bool oGLStatusWindow = false;
    static bool rendererWindow = true;
    static bool sceneWindow = true;
    static bool performanceWindow = true;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Engine"))
        {
            ImGui::MenuItem("OpenGL", NULL, &oGLStatusWindow);
            ImGui::MenuItem("Renderer", NULL, &rendererWindow);
            ImGui::MenuItem("Scene", NULL, &sceneWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Performance", NULL, &performanceWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (oGLStatusWindow)
    {
        ImGui::Begin("OpenGL", &oGLStatusWindow);

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

    if (rendererWindow)
    {
        ImGui::Begin("Renderer", &rendererWindow);

        ImGui::Text("Rendering Mode:");
        ImGui::SameLine();
        ImGui::SetCursorPosY((ImGui::GetFontSize() * 1.5f + ImGui::GetStyle().FramePadding.y * 2.0f));
        const char* options[] = { "FORWARD", "DEFERRED" };
        static const char* preview = options[1];
        if (ImGui::BeginCombo("##1", preview))
        {
            for (u32 i = 0; i < 2; ++i)
            {
                bool isSelected = (preview == options[i]);
                if (ImGui::Selectable(options[i], isSelected))
                {
                    preview = options[i];
                    app->rendererOptions.forwardRendering = i == 0;

                    for (u32 i = 0; i < app->numEntities; ++i)
                    {
                        Entity& entity = app->entities[i];
                        switch (app->shaderPrograms[entity.shaderID].type)
                        {
                        case ShaderType::DEFAULT:
                            entity.shaderID = app->rendererOptions.forwardRendering ? app->renderer.forwardShadersID[0] : app->renderer.deferredShadersID[0];
                            break;
                        case ShaderType::TEXTURED_ALBEDO:
                            entity.shaderID = app->rendererOptions.forwardRendering ? app->renderer.forwardShadersID[1] : app->renderer.deferredShadersID[1];
                            break;
                        case ShaderType::TEXTURED_ALB_SPEC:
                            entity.shaderID = app->rendererOptions.forwardRendering ? app->renderer.forwardShadersID[2] : app->renderer.deferredShadersID[2];
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            ImGui::EndCombo();
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

        if (!app->rendererOptions.forwardRendering)
        {
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

                if (ImGui::DragInt("Kernel Size", &app->rendererOptions.ssaoKernelSize, 4.0f, 4, 64))
                {
                    app->renderer.ssaoKernel.clear();
                    app->renderer.GenerateKernelSamples(app->shaderPrograms[app->renderer.ssaoShaderID], app->rendererOptions.ssaoKernelSize);
                }
                
                static int increment = 2;
                if (ImGui::SliderInt("Noise Size Scale", &increment, 1, 8))
                {
                    app->rendererOptions.ssaoNoiseSize = (int)glm::pow(2 * increment, 2);
                    
                    glDeleteTextures(1, &app->renderer.noiseTextureHandle);
                    app->renderer.ssaoNoise.clear();
                    app->renderer.GenerateKernelNoise(app->rendererOptions.ssaoNoiseSize);
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("G-Buffer Render Target");
            static const char* preview = "FINAL COLOR";
            if (ImGui::BeginCombo("##2", preview))
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
        }

        ImGui::End();
    }

    if (sceneWindow)
    {
        ImGui::Begin("Scene", &sceneWindow);

        ImGui::Text("Camera");
        ImGui::DragFloat3("Position", &app->camera.position[0]);
        ImGui::DragFloat("Speed", &app->camera.speed, 1.0f, 1.0f, 10.0f);
        ImGui::DragFloat("Zoom", &app->camera.FOV, 1.0f, 5.0f, 45.0f);
        ImGui::Checkbox("Free Camera", &app->camera.freeCamera);
        if (!app->camera.freeCamera)
        {
            ImGui::DragFloat("Radius", &app->camera.m_Radius, 1.0f, 1.0f, 40.0f);
            ImGui::Checkbox("Auto Rotate", &app->camera.autoRotate);
            if(app->camera.autoRotate)
                ImGui::DragFloat("RotationSpeed", &app->camera.m_RotationSpeed, 0.1f, 0.1f, 1.0f,"%.1f");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Lights");
        ImGui::Spacing();
        ImGui::Text("Directional Light");
        ImGui::SameLine();
        ImGui::ColorEdit3("##3", &app->lights[0].color[0]);
        ImGui::Spacing();

        ImGui::Text("Point Lights");
        ImGui::Spacing();
        if (ImGui::Button("Turn Off All Point Lights"))
        {
            for (u32 i = 1; i < app->numLights; i++)
                app->lights[i].color = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        ImGui::Spacing();
        for (u32 i = 1; i < app->numLights; ++i)
        {
            ImGui::Text(std::string("Light " + std::to_string(i)).c_str());
            ImGui::SameLine();
            ImGui::ColorEdit3(std::string("##" + std::to_string(i + 3)).c_str(), &app->lights[i].color[0]);
        }
        ImGui::End();
    }

    if (performanceWindow)
    {
        ImGui::Begin("Performance", &performanceWindow);
        ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
        ImGui::Text("Frametime (s): %f", app->deltaTime);
        ImGui::Text("Render Loop (ms): %f", app->renderTime);
        ImGui::Text("Time (s): %f", app->currentTime);
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle input keyboard/mouse here
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESS)
        app->isRunning = false;
    
    app->camera.Update(app->input, app->displaySize, app->deltaTime, float(app->currentTime));

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

    Timer timer(&app->renderTime);

    if (app->rendererOptions.forwardRendering)
        app->renderer.ForwardRender(app);
    else
        app->renderer.DeferredRender(app);

    // SKYBOX //
    if (app->rendererOptions.activeSkybox)
    {
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