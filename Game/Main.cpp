#include "Engine.h"
#include "Core/File.h"
#include "Framework/Scene.h"

using namespace nu;

int main(){

    SetWorkingDirectory("Assets");

    // INITIALIZATION
    Engine::Instance().Initialize();

    Scene scene;
	scene.Load("scenes/scene-skybox.json");

	auto& renderer = Engine::Instance().GetRenderer();

    // MAIN LOOP
    bool quit = false;
    while (!quit) {
        // UPDATE
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
			Engine::Instance().GetEditor().ProcessEvent(event);
            if (event.type == SDL_EVENT_QUIT) {quit = true;}
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE){quit = true;}
        }

        // ENGINE
        Engine::Instance().Update();
        float dt = Engine::Instance().GetTime().GetDeltaTime();
        float time = Engine::Instance().GetTime().GetTime();

        //SCENE
		scene.Update(dt);

        //Editor
		Engine::Instance().GetEditor().BeginFrame();
        Engine::Instance().GetEditor().Update(scene);

        // RENDER
        renderer.BeginFrame();

        scene.Draw(renderer);

		Engine::Instance().GetRenderer().EndRenderPass();
        Engine::Instance().GetEditor().Draw(renderer);

        renderer.EndFrame();
    }
    
    // SHUTDOWN
    Engine::Instance().Shutdown();    

    return 0;
}

