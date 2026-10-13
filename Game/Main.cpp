#include "Engine.h"
#include "Core/File.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

#include "Resources/ResourceManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace nu;
namespace nu {
    struct Color4 {
        float r, g, b, a;
    };
}

struct Vertex{
    Vector3 position;
    Color4 color;
};

struct vertexUniform
{
    glm::mat4 model;
};

vertexUniform uniform;

std::vector<Vertex> vertices = {
    Vertex{ Vector3{-1.0f, -1.0f, 0.0f}, Color4{1, 0, 0, 1}}, // Bottom-Left
    Vertex{  Vector3{0.8f, -0.5f, 0.0f}, Color4{0, 1, 0, 1}}, // Bottom-Right
    Vertex{  Vector3{0.0f,  0.2f, 0.0f}, Color4{0, 0, 1, 1}}, // Top-Middle
};

int main()
{
    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    auto vb = std::make_shared<VertexBuffer>();
    vb->Create<Vertex>(vertices, Engine::Instance().GetRenderer().GetGPUDevice());

    auto vshader = Resources().Get<nu::Shader>("shaders/position.vert", Engine::Instance().GetRenderer(), 1);
    auto fshader = Resources().Get<nu::Shader>("shaders/color.frag", Engine::Instance().GetRenderer(), 1);

    auto pipeline = std::make_shared<Pipeline>();
    pipeline->AddVertexBuffer(sizeof(Vertex));
    //position
    pipeline->AddVertexAttribute(
        0,
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        offsetof(Vertex, position));
    //color
    pipeline->AddVertexAttribute(
        1,
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
        offsetof(Vertex, color));

    pipeline->Create(*vshader.get(), *fshader.get(),
        Engine::Instance().GetRenderer().GetGPUDevice(),
        Engine::Instance().GetRenderer().GetWindow());

    float scale = 0.5f;
    Color4 color = { 0, 0, 0, 1 };

    // MAIN LOOP
    bool quit = false;
    while (!quit) 
    {
        // UPDATE
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_EVENT_QUIT) 
            {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)
            {
                quit = true;
            }
        }

        // ENGINE
        Engine::Instance().Update();
        float dt = Engine::Instance().GetTime().GetDeltaTime();
        float time = Engine::Instance().GetTime().GetTime();

		color.r = abs(sin(Engine::Instance().GetTime().GetTime())) * 0.5f + 0.5f;

        //uniform.model = glm::mat4(1.0f);

		auto mxt = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
		auto mxr = glm::rotate(glm::mat4(1.0f), time * 3, glm::vec3(0.0f, 0.0f, 1.0f));
		auto mxs = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));

		uniform.model = mxt * mxr * mxs;

        //uniform.model = glm::translate(uniform.model, glm::vec3(0.5f, 0.0f, 0.0f));
        //uniform.model = glm::rotate(uniform.model, time * 3, glm::vec3(0.0f, 0.0f, 1.0f));
        //uniform.model = glm::scale(uniform.model, glm::vec3(0.5f, 0.5f, 0.5f));

        // RENDER
        Engine::Instance().GetRenderer().BeginFrame();

        Engine::Instance().GetRenderer().SetPipeline(*pipeline);
        Engine::Instance().GetRenderer().SetVertexBuffer(*vb);
        Engine::Instance().GetRenderer().SetVertexUniform(0, &uniform, sizeof(uniform));
		Engine::Instance().GetRenderer().SetFragmentUniform(0, &color, sizeof(color));


        Engine::Instance().GetRenderer().Draw(vb->GetVertexCount());

        Engine::Instance().GetRenderer().EndFrame();
    }

    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}

