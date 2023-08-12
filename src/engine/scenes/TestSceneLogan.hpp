#pragma once 

#include "Scene.hpp"


#include "core/Entity.hpp"

#include "rendering/Shader.hpp"
#include "components/SkyBox.hpp"
#include "components/StaticCollider.hpp"
#include "components/CharacterController.hpp"

#include "physics/PhysicsWorld.hpp"

namespace lei3d {
    class Entity;
    class Model;
    class Shader;
    class SkyBox;
    class PhysicsWorld;

    class TestSceneLogan : public Scene {
    public:
        TestSceneLogan();
        ~TestSceneLogan();

        void OnLoad() override;
        void OnUpdate(float deltaTime) override;
        void OnPhysicsUpdate(float deltaTime) override;
    private:
        std::unique_ptr<Model> backpackModel;
    };
}