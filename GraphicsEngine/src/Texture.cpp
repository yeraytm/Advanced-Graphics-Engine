#include "Texture.h"

#include "Shader.h"

#include "glad/glad.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

Image LoadImage(const char* filename, bool isFlipped)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(isFlipped);

    if (stbi_is_hdr(filename))
    {
        img.isHDR = true;
        img.pixels = stbi_loadf(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    }
    else
    {
        img.isHDR = false;
        img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    }

    if (img.pixels)
        img.stride = img.size.x * img.nchannels;
    else
        ELOG("Could not open file %s\n", filename);

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
        ELOG("LoadTexture2D() - Unsupported number of channels\n");
    }

    u32 texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, image.isHDR ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (image.isHDR)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image.size.x, image.size.y, 0, GL_RGB, GL_FLOAT, image.pixels);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);

    if (!image.isHDR)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(std::vector<Texture>& textures, const char* filepath, bool isFlipped)
{
    for (u32 texIdx = 0; texIdx < textures.size(); ++texIdx)
        if (textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath, isFlipped);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = textures.size();
        textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
        return UINT32_MAX;
}

glm::uvec2 LoadCubemap(std::vector<Texture>& textures, const char* filepath, Shader& equirectToCubemapShader, Shader& irradianceConvShader, u32 skyboxCubeVAO)
{
    // Matrices needed to generate cubemap faces
    glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    // Framebuffer to capture cubemap faces
    u32 cubemapFBO, cubemapRBO;
    glGenFramebuffers(1, &cubemapFBO);
    glGenRenderbuffers(1, &cubemapRBO);

    // CUBEMAP SKYBOX TEXTURE //
    glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, cubemapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cubemapRBO);

    Texture& hdrTexture = textures[LoadTexture2D(textures, filepath, true)];

    u32 environmentMapHandle;
    glGenTextures(1, &environmentMapHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);
    for (u32 i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    equirectToCubemapShader.Bind();
    equirectToCubemapShader.SetUniform1i("uEquirectangularMap", 0);
    equirectToCubemapShader.SetUniformMat4("uProjection", captureProj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture.handle);

    // Configure viewport to the dimensions of each face we want to capture
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
    for (u32 i = 0; i < 6; ++i)
    {
        equirectToCubemapShader.SetUniformMat4("uView", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentMapHandle, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(skyboxCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    equirectToCubemapShader.Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // IRRADIANCE CUBEMAP TEXTURE //
    u32 irradianceMapHandle;
    glGenTextures(1, &irradianceMapHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapHandle);
    for (u32 i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, cubemapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradianceConvShader.Bind();
    irradianceConvShader.SetUniform1i("uEnvironmentMap", 0);
    irradianceConvShader.SetUniformMat4("uProjection", captureProj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapHandle);

    // Configure viewport to the dimensions of each face we want to capture
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);
    for (u32 i = 0; i < 6; ++i)
    {
        irradianceConvShader.SetUniformMat4("uView", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMapHandle, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(skyboxCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    irradianceConvShader.Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return glm::uvec2(environmentMapHandle, irradianceMapHandle);
}

u32 LoadCubemap(std::vector<std::string>& faces)
{
    u32 cubemapTexHandle;
    glGenTextures(1, &cubemapTexHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexHandle);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        Image image = LoadImage(faces[i].c_str(), false);
        if (image.pixels)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image.size.x, image.size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, image.pixels);
            FreeImage(image);
        }
        else
        {
            std::string log = "Cubemap texture failed to load at path: " + faces[i];
            ELOG(log.c_str());
            FreeImage(image);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return cubemapTexHandle;
}
