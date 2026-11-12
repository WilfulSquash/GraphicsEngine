#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

namespace nu
{
	class Renderer;

	class IGUI
	{
	public:
		virtual void UpdateGui() = 0;
	};

	class Editor
	{
	public:
		bool Initialize(Renderer& renderer);
		void Shutdown();

		void Update();
		void ProcessEvent(const SDL_Event& event);

		void BeginFrame();
		void Draw(Renderer& renderer);

		void SetActive(bool active = true) { m_active = active; }
		bool GetActive() { return m_active; }

	private:
		bool m_active = true;
	};
}