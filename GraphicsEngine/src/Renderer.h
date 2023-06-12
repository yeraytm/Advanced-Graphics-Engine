#pragma once

#include "platform.h"
#include "Shader.h"
#include "Framebuffer.h"

#include "glad/glad.h"

#include <array>

struct App;
class Shader;
struct Model;

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

	void GenerateKernelSamples(App* app);

	void GenerateKernelNoise();

private:
	inline void BindDefaultFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	u32 FindVAO(Model* model, u32 meshIndex, const Shader& shaderProgram);

public:
	u32 lightCasterShaderID;

	// Shaders specific to the rendering mode
	std::array<u32, 3> forwardShadersID;
	std::array<u32, 3> deferredShadersID;

	// DEFERRED RENDERING //
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
	std::vector<glm::vec3> ssaoNoise;
	u32 noiseTextureHandle;
	u32 ssaoShaderID;
	u32 ssaoBlurShaderID;
};