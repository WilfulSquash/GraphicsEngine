#pragma once
#include "RendererComponent.h"

namespace nu{
	class LightComponent : public RendererComponent{
	public:
		CLASS_PROTOTYPE(LightComponent)

		void Draw(const Renderer& renderer) override;

	private:
	};
}
