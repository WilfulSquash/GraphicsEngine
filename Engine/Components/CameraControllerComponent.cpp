#include "pch.h"
#include "CameraControllerComponent.h"
#include "Framework/Actor.h"
#include "Engine.h"
#include "Core/Factory.h"

namespace nu {
	FACTORY_REGISTER(CameraControllerComponent)

	void nu::CameraControllerComponent::Update(float dt)
	{
        /*
        glm::vec3 direction{ 0 };
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_W)) direction.y = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_A)) direction.x = +1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_S)) direction.y = +1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_D)) direction.x = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_E)) direction.z = -1;
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_Q)) direction.z = +1;

        m_owner->SetPosition(m_owner->GetTransform().position + direction * 4.0f * dt);
        */

        // get camera rotation
        if (Engine::Instance().GetInput().GetButtonDown(Input::MouseButton::Right))
        {
            glm::vec2 axis = Engine::Instance().GetInput().GetMouseDelta() * m_sensitivity;

            m_yaw -= axis.x;
            m_pitch -= axis.y;
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
        }

        //owner->transform.SetRotation(glm::vec3(pitch, yaw, 0.0f));

        //glm::quat q = glm::quat_cast(glm::eulerAngleYXZ(glm::radians(m_yaw), glm::radians(m_pitch), 0.0f));
        //m_owner->GetTransform().SetRotationQuat(q);

        glm::vec3 direction{ 0 };
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_A)) direction += m_owner->GetTransform().Right();
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_D)) direction -= m_owner->GetTransform().Right();
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_W)) direction += m_owner->GetTransform().Up();
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_S)) direction -= m_owner->GetTransform().Up();
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_E)) direction += m_owner->GetTransform().Forward();
        if (Engine::Instance().GetInput().GetKeyDown(SDL_SCANCODE_Q)) direction -= m_owner->GetTransform().Forward();;

        if (glm::length2(direction) > 0.0f) {
            direction = glm::normalize(direction);
        }
        m_owner->SetPosition(m_owner->GetTransform().position + direction * 4.0f * dt);

	}
    void CameraControllerComponent::Read(const json::value_t& value){

		JSON_READ_NAME(value, "speed", m_speed);
    }
}