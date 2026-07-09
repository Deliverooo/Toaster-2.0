#pragma once

#include "toast_scene.hpp"

namespace toaster::scene
{
	struct TST_SCENE_API DirectionalLight
	{
		Dx::XMFLOAT3 direction{0.0f, 0.0f, 0.0f};
		char         _padd[4];
		tsm::float3  radiance{1.0f, 1.0f, 1.0f};
		float32      multiplier{1.0f};
	};

	struct TST_SCENE_API PointLight
	{
		Dx::XMFLOAT4 position{0.0f, 0.0f, 0.0f, 1.0f};
		tsm::float4  radianceIntensity{1.0f, 1.0f, 1.0f, 1.0f};
	};

	struct TST_SCENE_API LightEnvironment
	{
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight>       pointLights;
	};
}
