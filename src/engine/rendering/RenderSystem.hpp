#pragma once

#include "core/Camera.hpp"
#include "core/Component.hpp"
#include "core/Scene.hpp"
#include "core/SceneView.hpp" 

#include "rendering/Shader.hpp"

namespace lei3d
{

	class ModelInstance;
	class SkyBox;

	class Scene;
	class SceneView;

	class RenderSystem
	{
	public:
		RenderSystem()
		{
		}

		~RenderSystem()
		{
		}

		void initialize(int width, int height);

		void draw(const Scene& scene, const SceneView& view);

	private:
		void lightingPass(const std::vector<ModelInstance*>& objects, const DirectionalLight* light, Camera& camera);
		void environmentPass(const SkyBox& skyBox, Camera& camera);
		void postprocessPass();

		void genShadowPass(const std::vector<ModelInstance*>& objects, DirectionalLight* light, Camera& camera);
		std::vector<glm::vec4> getFrustumCornersWS(const glm::mat4& projection, const glm::mat4& view);
		glm::mat4 getLightSpaceMatrix(DirectionalLight* light, float nearPlane, float farPlane, Camera& camera);
		std::vector<glm::mat4> getLightSpaceMatrices(DirectionalLight* light, Camera& camera);

		// offscreen render target objects
		unsigned int FBO;
		unsigned int rawTexture;
		unsigned int saturationMask;
		unsigned int depthStencilTexture;
		unsigned int finalTexture;

		// shadow resources
		unsigned int shadowFBO;
		unsigned int shadowResolution = 2048;
		unsigned int shadowDepth;

		unsigned int dummyVAO; // used to draw full-screen "quad"

		int scwidth, scheight;
		float frustum_fitting_factor = 10.f;	// TODO: make configurable through scenes?

		// shaders
		Shader forwardShader;
		Shader postprocessShader;
		Shader shadowCSMShader;
	};

} // namespace lei3d
