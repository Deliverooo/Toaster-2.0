#include <iostream>


#include "toast_lib/core_basic.hpp"
#include "toast_lib/math/trig.hpp"

auto main(int32_t p_argc, char **p_argv) -> int32_t
{
	tsm::float4x4 tsm_mat{1.0f};
	tsm_mat = tsm::inverse(tsm_mat);

	tsm::float4 tsm_vec{1.0f};

	LOG_INFO("{}\n", tsm_mat);
	LOG_INFO("{}\n", tsm_vec);

	LOG_INFO("{}\n", tsm_mat * tsm_vec);

	std::cin.get();
	return 0;
}
