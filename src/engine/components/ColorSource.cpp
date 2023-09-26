#include "ColourSource.hpp"

#include "logging/LogGLM.hpp"
#include <array>

namespace lei3d
{
	ColorSource::ColorSource(Entity& entity)
		: Component(entity)
	{
	}

	ColorSource::~ColorSource()
	{
	}

	void ColorSource::Init(float new_radius)
	{
		radius = new_radius;
		active = false;
	}
	
	void SetActive()
	{
		SetActive(true);
	}
	void SetActive(bool new_active)
	{
		active = new_active;
	}
	// just so that calls are consistent
	float GetRadius()
	{
		return radius;
	}
	vec3 GetPosition()
	{
		return m_Entity.m_Transform.position;
	}
} // namespace lei3d