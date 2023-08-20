#pragma once 

#include "core/Scene.hpp"


#include "core/Entity.hpp"

#include "components/SkyBox.hpp"

namespace lei3d {
    class Entity;
    class Model;
    class Shader;
    class SkyBox;

    class EmptyScene : public Scene {
    public:
        EmptyScene();
        ~EmptyScene();

        void OnLoad() override;
    };
}