#pragma once
#include "Serialization/Json.h"
#include "Editor/Editor.h"
#include <string>
#include <memory>

#define CLASS_PROTOTYPE(classname) \
	virtual std::unique_ptr<Object> Clone() const { return std::make_unique<classname>(*this); } \
	virtual std::string GetClassName() const {return #classname;}

namespace nu
{
	class Object : public IGUI
	{
	public:
		Object() = default;
		virtual ~Object() = default;

		CLASS_PROTOTYPE(Object)

		const std::string& GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		bool IsActive() const { return m_active; }
		void SetActive(bool active = true) { m_active = active; }

		virtual void Read(const json::value_t& value)
		{
			JSON_READ_NAME(value, "name", m_name);
			JSON_READ_NAME(value, "active", m_active);
		}

		void UpdateGui() override {
			ImGui::Text("%s", m_name.c_str());
		}

	protected:
		std::string m_name;
		bool m_active = true;
	};
}