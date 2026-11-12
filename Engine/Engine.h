#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/ParticleSystem.h"
#include "Core/GameTime.h"
#include "Input/Input.h"
#include "Audio/Audio.h"

#include "Core/Singleton.h"
#include "Editor/Editor.h"

namespace nu
{
	class Engine : public Singleton<Engine>
	{
	public:
		bool Initialize();
		void Shutdown();

		void Update();

		Input& GetInput() { return *m_input; }
		Renderer& GetRenderer() { return *m_renderer; }
		Audio& GetAudio() { return *m_audio; }
		Time& GetTime() { return m_time; }
		ParticleSystem& GetPS() { return *m_particleSystem; }
		Editor& GetEditor(){ return *m_editor; }
		

	private:
		friend class Singleton<Engine>;
		Engine() = default;

	private:
		std::unique_ptr<Input> m_input;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<Audio> m_audio;
		std::unique_ptr<ParticleSystem> m_particleSystem;
		std::unique_ptr<Editor > m_editor;

		Time m_time;
	};
}
