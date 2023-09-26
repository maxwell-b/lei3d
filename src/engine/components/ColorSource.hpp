#pragma once

#include "core/Component.hpp"
#include "logging/Log.hpp"

#include <string>
#include <vector>

namespace lei3d
{
	class ColorSource : public Component
	{
	public:
		float radius;
		bool active;

		ColorSource(Entity& entity);
		~ColorSource();

		void Init(float radius);

		void SetActive();
		void SetActive(bool active);

		float GetRadius();
		vec3 GetPosition();

	private:
	};


} // namespace lei3d