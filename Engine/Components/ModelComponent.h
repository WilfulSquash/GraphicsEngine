#pragma once
#include "RendererComponent.h"

namespace nu{
	class ModelComponent : public RendererComponent{
	public:
		CLASS_PROTOTYPE(ModelComponent)

		void Draw(const Renderer& renderer) override;

	private:
	};
}
