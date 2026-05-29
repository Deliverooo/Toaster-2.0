#include <iostream>

#include <glm/glm.hpp>

auto main(int32_t p_argc, char **p_argv) -> int32_t
{
	glm::vec2 v{67.0f};

	std::cout << v.x << std::endl;

	std::cin.get();
	return 0;
}
