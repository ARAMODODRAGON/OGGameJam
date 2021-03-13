#ifndef GENERAL_HPP
#define GENERAL_HPP

#include <glm\glm.hpp>
using glm::uvec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#include <inttypes.h>
using uint32 = uint32_t;
using int32 = int32_t;

#include <vector>
using std::vector;
#include <string>
using std::string;

#include "Core\Debugger.hpp"

inline vec4 ColorMix(const float mix) {
	const static vec4 colors[] = {
		vec4(1.0f, 0.0f, 0.0f, 1.0f),
		vec4(1.0f, 0.5f, 0.0f, 1.0f),
		vec4(1.0f, 1.0f, 0.0f, 1.0f),
		vec4(0.5f, 1.0f, 0.0f, 1.0f),
		vec4(0.0f, 1.0f, 0.0f, 1.0f),
		vec4(0.0f, 1.0f, 0.5f, 1.0f),
		vec4(0.0f, 1.0f, 1.0f, 1.0f),
		vec4(0.0f, 0.5f, 1.0f, 1.0f),
		vec4(0.0f, 0.0f, 1.0f, 1.0f),
		vec4(0.5f, 0.0f, 1.0f, 1.0f),
		vec4(1.0f, 0.0f, 1.0f, 1.0f),
		vec4(1.0f, 0.0f, 0.5f, 1.0f)
	};
	constexpr size_t colors_count = sizeof(colors) / sizeof(colors[0]);
	constexpr float colors_countf = static_cast<float>(colors_count);

	const float mixpos = mix * colors_countf;
	const size_t index = static_cast<size_t>(mixpos) % colors_count;
	const float mixval = mixpos - glm::floor(mixpos);
	return glm::mix(colors[index], colors[(index + 1) % colors_count], mixval);
}

inline float RandomRange(const float min, const float max) {
	constexpr float invmaxrand = 1.0f / static_cast<float>(RAND_MAX);
	return (static_cast<float>(rand()) * invmaxrand) * (max - min) + min;
}

#endif // !GENERAL_HPP