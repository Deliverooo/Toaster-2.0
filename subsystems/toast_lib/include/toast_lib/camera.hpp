#pragma once

#include "toast_lib.hpp"

#include "system_types.h"

#include "toast_math/math_matrix.hpp"

namespace toaster
{
	class TST_LIB_API Camera
	{
	public:
		Camera() = default;

		Camera(Dx::FXMMATRIX p_projection)
		{
			Dx::XMStoreFloat4x4(&m_projection, p_projection);
		}

		virtual ~Camera() = default;

		auto XM_CALLCONV setProjectionMatrix(Dx::FXMMATRIX p_projection) -> void
		{
			Dx::XMStoreFloat4x4(&m_projection, p_projection);
		}

		[[nodiscard]] auto getProjectionMatrix() const -> Dx::FXMMATRIX { return Dx::XMLoadFloat4x4(&m_projection); }
		[[nodiscard]] auto getProjectionMatrix4X4() const -> const Dx::XMFLOAT4X4 & { return m_projection; }

		auto setPerspective(float32 p_fov, float32 p_aspect, float32 p_z_near, float32 p_z_far) -> void
		{
			Dx::XMMATRIX proj{Dx::XMMatrixPerspectiveFovLH(p_fov, p_aspect, p_z_near, p_z_far)};
			Dx::XMStoreFloat4x4(&m_projection, proj);
			// m_projection.m[1][1] *= -1.0f;
		}

		auto setOrthographic(float32 p_width, float32 p_height, float32 p_z_near, float32 p_z_far) -> void
		{
			Dx::XMMATRIX proj{Dx::XMMatrixOrthographicLH(p_width, p_height, p_z_near, p_z_far)};
			Dx::XMStoreFloat4x4(&m_projection, proj);
		}

		auto setOrthographicFromSize(float32 p_size, float32 p_aspect, float32 p_z_near, float32 p_z_far) -> void
		{
			const float32 ortho_left{-p_size * p_aspect * 0.5f};
			const float32 ortho_right{p_size * p_aspect * 0.5f};
			const float32 ortho_bottom{-p_size * 0.5f};
			const float32 ortho_top{p_size * 0.5f};

			Dx::XMMATRIX proj{Dx::XMMatrixOrthographicOffCenterLH(ortho_left, ortho_right, ortho_bottom, ortho_top, p_z_near, p_z_far)};
			Dx::XMStoreFloat4x4(&m_projection, proj);
		}

	protected:
		Dx::XMFLOAT4X4 m_projection;
	};
}
