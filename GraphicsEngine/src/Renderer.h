#pragma once

#include "platform.h"
#include "Shader.h"
#include "Framebuffer.h"

#include "glad/glad.h"

struct App;
class Shader;

struct ScreenQuad
{
	Framebuffer FBO;
	u32 VAO;
	u32 shaderID;
	u32 currentRenderTarget;
};

class Renderer
{
public:
	void Init(App* app);

	void ForwardRender(App* app);

	void DeferredRender(App* app);

	inline void BindDefaultFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

public:
	u32 lightCasterShaderID;

	// DEFERRED SHADING //
	Framebuffer GBuffer;
	u32 lightingPassShaderID;

	// SCREEN-FILLING QUAD //
	ScreenQuad screenQuad;

	// SKYBOX //
	u32 skyboxCubeVAO;
	u32 skyboxShaderID;
	u32 environmentMapHandle;
	u32 irradianceMapHandle;

	// SSAO //
	Framebuffer ssaoBuffer;
	Framebuffer ssaoBlurBuffer;
	std::vector<glm::vec3> ssaoKernel;
	u32 noiseTextureHandle;
	u32 ssaoShaderID;
	u32 ssaoBlurShaderID;
};