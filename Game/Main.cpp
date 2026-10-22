#include "Engine.h"
#include "Core/File.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"
#include "Math/Transform.h"
#include "Renderer/Model.h"

#include "Resources/ResourceManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


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
    // Add this before main() or at the top of main(), after Color4 is defined
    nu::Color4 tint = { 1, 1, 1, 1 };
    

//Uniforms
struct VertexUniform
{
    glm::mat4 mvp;
    glm::mat4 model;
};

struct LightUniform {
    glm::vec3 lightDir;
    float padding1;
    glm::vec3 lightColor;
    float lightIntensity;
    glm::vec3 ambient;
    float padding2;
};

LightUniform lightUniform{
    .lightDir{0, -1, 0},
    .lightColor{1, 0, 1},
    .lightIntensity{1},
    .ambient{0.5f, 0.5f, 0.5f}
};

VertexUniform vertexUniform;
glm::vec3 cameraPos{ 0, 0, 5 };

std::vector<Vertex> vertices = {
    Vertex{ Vector3{-0.5f, -0.5f, 0.0f}, Color4{1, 0, 0, 1}}, // Bottom-Left
    Vertex{ Vector3{0.5f, -0.5f, 0.0f}, Color4{0, 1, 0, 1}}, // Bottom-Right
    Vertex{ Vector3{0.5f,  0.5f, 0.0f}, Color4{0, 0, 1, 1}}, // Top-Right
    Vertex{ Vector3{-0.5f,  0.5f, 0.0f}, Color4{0, 1, 1, 1}}, // Top-Left
};

std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};


int main()
{
    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    auto model = Resources().Get<Model>("models/spot.obj", Engine::Instance().GetRenderer());
    auto texture = Resources().Get<Texture>("textures/spot_diffuse.png", Engine::Instance().GetRenderer());

    auto vshader = Resources().Get<nu::Shader>("shaders/lit.vert", Engine::Instance().GetRenderer(), 2);
    auto fshader = Resources().Get<nu::Shader>("shaders/lit.frag", Engine::Instance().GetRenderer(), 1, 1);

    auto pipeline = std::make_shared<Pipeline>();

    pipeline->AddVertexBuffer(sizeof(Model::Vertex));
    pipeline->AddVertexAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, position));
    pipeline->AddVertexAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Model::Vertex, texcoord));
    pipeline->AddVertexAttribute(2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, normal));
    pipeline->AddVertexAttribute(3, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Model::Vertex, tangent));

    pipeline->Create(*vshader.get(), *fshader.get(),
        Engine::Instance().GetRenderer().GetGPUDevice(),
        Engine::Instance().GetRenderer().GetWindow());

    auto vb = std::make_shared<VertexBuffer>();
    vb->Create<Vertex>(vertices, Engine::Instance().GetRenderer());

	auto ib = std::make_shared<IndexBuffer>();
    ib->Create(static_cast<uint32_t>(indices.size()), indices.data(), Engine::Instance().GetRenderer());

    pipeline->Create(*vshader.get(), *fshader.get(),
        Engine::Instance().GetRenderer().GetGPUDevice(),
        Engine::Instance().GetRenderer().GetWindow());

    float scale = 0.5f;
    Color4 color = { 0, 0, 0, 1 };

	Transform transform;

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

		//color.r = sin(time * 10.0f) * 0.5f + 0.5f;

        glm::vec3 direction{ 0 };
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_W)) direction.y = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_A)) direction.x = +1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_S)) direction.y = +1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_D)) direction.x = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_E)) direction.z = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_Q)) direction.z = +1;

        cameraPos += direction * 4.0f * dt;


        //Transform
        transform.rotation.y = time * 100;

        //VIEW MATRIX
		auto view = glm::lookAtRH(cameraPos, cameraPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		//PROJECTION MATRIX
		float aspectRatio = (float)Engine::Instance().GetRenderer().GetWidth() / (float)Engine::Instance().GetRenderer().GetHeight();
		auto projection = glm::perspectiveRH(glm::radians(45.0f), aspectRatio, 0.01f, 20.0f);

        //MVP
        vertexUniform.model = transform.GetMatrix(); 
        vertexUniform.mvp = projection * view * vertexUniform.model;
        // RENDER
        Engine::Instance().GetRenderer().BeginFrame();

        Engine::Instance().GetRenderer().SetPipeline(*pipeline);
        Engine::Instance().GetRenderer().SetVertexUniform(0, &vertexUniform, sizeof(vertexUniform));
        Engine::Instance().GetRenderer().SetVertexUniform(1, &lightUniform, sizeof(lightUniform));
        Engine::Instance().GetRenderer().SetFragmentUniform(0, &tint, sizeof(tint));
        Engine::Instance().GetRenderer().SetFragmentTexture(0, *texture);

        model->Draw(Engine::Instance().GetRenderer());

        Engine::Instance().GetRenderer().EndFrame();
    }
    
    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}

