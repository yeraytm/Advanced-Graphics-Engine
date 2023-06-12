#include "Renderer.h"

#include "engine.h"
#include "Entity.h"

#include <random>

void Renderer::Init(App* app)
{
    // SCREEN QUAD //
    Shader& screenQuadShader = app->shaderPrograms[screenQuad.shaderID];
    screenQuadShader.Bind();
    screenQuadShader.SetUniform1i("uRenderTarget", 0);

    screenQuad.FBO.Generate();
    screenQuad.FBO.Bind();
    screenQuad.FBO.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize); // Final Color Buffer
    screenQuad.FBO.SetColorBuffers(); // Set color buffers with glDrawBuffers
    BindDefaultFramebuffer();

    screenQuad.currentRenderTarget = screenQuad.FBO.colorAttachmentHandles[0];

    // SKYBOX //
    Shader& skyboxShader = app->shaderPrograms[skyboxShaderID];
    skyboxShader.Bind();
    skyboxShader.SetUniform1i("uEnvironmentMap", 0);

    // DEFERRED RENDERING //
    GBuffer.Generate();
    GBuffer.Bind();
    GBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize, true); // Position Color Buffer
    GBuffer.AttachColorTexture(FBAttachmentType::COLOR_FLOAT, app->displaySize);       // Normal Color Buffer
    GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);        // Albedo Color Buffer
    GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);        // Specular Color Buffer
    GBuffer.AttachColorTexture(FBAttachmentType::COLOR_BYTE, app->displaySize);        // Reflective + Shininess Color Buffer
    GBuffer.AttachDepthTexture(app->displaySize);                                      // Depth Attachment
    GBuffer.SetColorBuffers(); // Set color buffers with glDrawBuffers
    BindDefaultFramebuffer();

    Shader& lightingPassShader = app->shaderPrograms[lightingPassShaderID];
    lightingPassShader.Bind();
    lightingPassShader.SetUniform1i("gBufPosition", 0);
    lightingPassShader.SetUniform1i("gBufNormal", 1);
    lightingPassShader.SetUniform1i("gBufAlbedo", 2);
    lightingPassShader.SetUniform1i("gBufSpecular", 3);
    lightingPassShader.SetUniform1i("gBufReflShini", 4);
    lightingPassShader.SetUniform1i("uEnvironmentMap", 5);
    lightingPassShader.SetUniform1i("uIrradianceMap", 6);
    lightingPassShader.SetUniform1i("uSSAOColor", 7);

    // SSAO //
    ssaoBuffer.Generate();
    ssaoBuffer.Bind();
    ssaoBuffer.AttachColorTexture(FBAttachmentType::COLOR_R, app->displaySize, true);
    ssaoBuffer.SetColorBuffers();
    BindDefaultFramebuffer();

    ssaoBlurBuffer.Generate();
    ssaoBlurBuffer.Bind();
    ssaoBlurBuffer.AttachColorTexture(FBAttachmentType::COLOR_R, app->displaySize, true);
    ssaoBlurBuffer.SetColorBuffers();
    BindDefaultFramebuffer();

    GenerateKernelSamples(app);
    GenerateKernelNoise();

    Shader& SSAOShader = app->shaderPrograms[ssaoShaderID];
    SSAOShader.Bind();
    SSAOShader.SetUniform1i("gBufPosition", 0);
    SSAOShader.SetUniform1i("gBufNormal", 1);
    SSAOShader.SetUniform1i("gBufDepth", 2);
    SSAOShader.SetUniform1i("uNoiseTexture", 3);
    for (u32 i = 0; i < 64; ++i)
        SSAOShader.SetUniform3f("uSamples[" + std::to_string(i) + "]", ssaoKernel[i]);

    Shader& SSAOBlurShader = app->shaderPrograms[ssaoBlurShaderID];
    SSAOBlurShader.Bind();
    SSAOBlurShader.SetUniform1i("uSSAOColor", 0);
}

void Renderer::ForwardRender(App* app)
{
    BindDefaultFramebuffer();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (u32 i = 0; i < app->numEntities; ++i)
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
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess * 256.0f);

                // Environment Map
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);

                // Irradiance Map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapHandle);

                shader.SetUniform1i("uRendererOptions.uActiveIrradiance", app->rendererOptions.activeIrradiance);
                shader.SetUniform1i("uRendererOptions.uActiveReflection", app->rendererOptions.activeReflection);
                shader.SetUniform1i("uRendererOptions.uActiveRefraction", app->rendererOptions.activeRefraction);
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
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess * 256.0f);

                // Environment Map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);

                // Irradiance Map
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapHandle);

                shader.SetUniform1i("uRendererOptions.uActiveIrradiance", app->rendererOptions.activeIrradiance);
                shader.SetUniform1i("uRendererOptions.uActiveReflection", app->rendererOptions.activeReflection);
                shader.SetUniform1i("uRendererOptions.uActiveRefraction", app->rendererOptions.activeRefraction);
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
                shader.SetUniform1f("uMaterial.shininess", meshMaterial.shininess * 256.0f);

                // Environment Map
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);

                // Irradiance Map
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapHandle);

                shader.SetUniform1i("uRendererOptions.uActiveIrradiance", app->rendererOptions.activeIrradiance);
                shader.SetUniform1i("uRendererOptions.uActiveReflection", app->rendererOptions.activeReflection);
                shader.SetUniform1i("uRendererOptions.uActiveRefraction", app->rendererOptions.activeRefraction);
            }
            break;
            case ShaderType::LIGHT_CASTER:
            {
                shader.SetUniform3f("uLightColor", app->lights[i - app->firstLightEntityID].color);
            }
            break;
            }

            Mesh& mesh = model->meshes[meshIndex];
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);
            glBindVertexArray(0);
        }
        shader.Unbind();
    }
}

void Renderer::DeferredRender(App* app)
{
    // DEFERRED RENDERING: GEOMETRY PASS //
    GBuffer.Bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
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

    BindDefaultFramebuffer();

    if (app->rendererOptions.activeSSAO)
    {
        // SSAO //
        ssaoBuffer.Bind();
        glDisable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT);

        Shader& SSAOShader = app->shaderPrograms[ssaoShaderID];
        SSAOShader.Bind();

        SSAOShader.SetUniformMat4("uProjection", app->camera.GetProjectionMatrix(app->displaySize));
        SSAOShader.SetUniformMat4("uView", app->camera.GetViewMatrix(app->displaySize));
        SSAOShader.SetUniform2f("uDisplaySize", glm::vec2(app->displaySize.x, app->displaySize.y));

        SSAOShader.SetUniform1i("uSSAOptions.uRangeCheck", app->rendererOptions.activeRangeCheck);
        SSAOShader.SetUniform1f("uSSAOptions.uRadius", app->rendererOptions.ssaoRadius);
        SSAOShader.SetUniform1f("uSSAOptions.uBias", app->rendererOptions.ssaoBias);
        SSAOShader.SetUniform1f("uSSAOptions.uPower", app->rendererOptions.ssaoPower);
        SSAOShader.SetUniform1i("uSSAOptions.uKernelSize", app->rendererOptions.ssaoKernelSize);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GBuffer.colorAttachmentHandles[0]); // Position
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, GBuffer.colorAttachmentHandles[1]); // Normal
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, GBuffer.depthAttachment); // Depth
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, noiseTextureHandle); // SSAO Noise Texture

        glBindVertexArray(screenQuad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        glEnable(GL_BLEND);
        SSAOShader.Unbind();

        BindDefaultFramebuffer();

        if (app->rendererOptions.activeSSAOBlur)
        {
            // SSAO Blur
            ssaoBlurBuffer.Bind();
            glDisable(GL_BLEND);
            glClear(GL_COLOR_BUFFER_BIT);

            Shader& SSAOBlurShader = app->shaderPrograms[ssaoBlurShaderID];
            SSAOBlurShader.Bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoBuffer.colorAttachmentHandles[0]); // SSAO Color Texture

            glBindVertexArray(screenQuad.VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            glEnable(GL_BLEND);
            SSAOBlurShader.Unbind();

            BindDefaultFramebuffer();
        }
    }

    // DEFERRED RENDERING: LIGHTING PASS //
    screenQuad.FBO.Bind();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader& lightingPassShader = app->shaderPrograms[lightingPassShaderID];
    lightingPassShader.Bind();

    // Set the uniform textures from the G-Buffer
    for (u32 i = 0; i < GBuffer.colorAttachmentHandles.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, GBuffer.colorAttachmentHandles[i]);
    }

    // Environment Map
    glActiveTexture(GL_TEXTURE0 + GBuffer.colorAttachmentHandles.size());
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);

    // Irradiance Map
    glActiveTexture(GL_TEXTURE1 + GBuffer.colorAttachmentHandles.size());
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapHandle);

    // SSAO Color
    glActiveTexture(GL_TEXTURE2 + GBuffer.colorAttachmentHandles.size());
    glBindTexture(GL_TEXTURE_2D, app->rendererOptions.activeSSAOBlur ? ssaoBlurBuffer.colorAttachmentHandles[0] : ssaoBuffer.colorAttachmentHandles[0]);

    lightingPassShader.SetUniform1i("uRendererOptions.uActiveIrradiance", app->rendererOptions.activeIrradiance);
    lightingPassShader.SetUniform1i("uRendererOptions.uActiveReflection", app->rendererOptions.activeReflection);
    lightingPassShader.SetUniform1i("uRendererOptions.uActiveRefraction", app->rendererOptions.activeRefraction);
    lightingPassShader.SetUniform1i("uRendererOptions.uActiveSSAO", app->rendererOptions.activeSSAO);

    glBindVertexArray(screenQuad.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    lightingPassShader.Unbind();

    // SCREEN-FILLING QUAD //
    BindDefaultFramebuffer();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader& screenQuadShader = app->shaderPrograms[screenQuad.shaderID];
    screenQuadShader.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenQuad.currentRenderTarget);

    glBindVertexArray(screenQuad.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
    screenQuadShader.Unbind();

    // RENDER LIGHTS USIGN FORWARD RENDERING //
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, GBuffer.handle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, app->displaySize.x, app->displaySize.y, 0, 0, app->displaySize.x, app->displaySize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    BindDefaultFramebuffer();

    Shader& lightCasterShader = app->shaderPrograms[lightCasterShaderID];
    lightCasterShader.Bind();
    u32 lightID = 0;
    for (u32 i = app->firstLightEntityID; i < app->numEntities; ++i)
    {
        Entity& lightEntity = app->entities[i];

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->UBO.handle, lightEntity.localParamOffset, lightEntity.localParamSize);

        Model* model = lightEntity.model;

        u32 vao = FindVAO(model, 0, lightCasterShader);
        glBindVertexArray(vao);

        lightCasterShader.SetUniform3f("uLightColor", app->lights[lightID].color);

        Mesh& mesh = model->meshes[0];
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

        glBindVertexArray(0);
        lightID++;
    }
    lightCasterShader.Unbind();
}

void Renderer::GenerateKernelSamples(App* app)
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    ssaoKernel.reserve(64);
    for (u32 i = 0; i < 64; i++)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator)
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        float scale = float(i) / 64.0f;
        scale = Lerp(0.1f, 1.0f, scale * scale);

        sample *= scale;
        ssaoKernel.emplace_back(sample);
    }

    Shader& SSAOShader = app->shaderPrograms[ssaoShaderID];
    SSAOShader.Bind();
    for (u32 i = 0; i < 64; ++i)
        SSAOShader.SetUniform3f("uSamples[" + std::to_string(i) + "]", ssaoKernel[i]);
}

void Renderer::GenerateKernelNoise()
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    ssaoNoise.reserve(16);
    for (u32 i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f
        );
        ssaoNoise.emplace_back(glm::normalize(noise));
    }

    glGenTextures(1, &noiseTextureHandle);
    glBindTexture(GL_TEXTURE_2D, noiseTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

u32 Renderer::FindVAO(Model* model, u32 meshIndex, const Shader& shaderProgram)
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