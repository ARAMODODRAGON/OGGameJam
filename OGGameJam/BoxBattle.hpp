#ifndef BOXBATTLE_HPP
#define BOXBATTLE_HPP
#include "Core\Timer.hpp"
#include "SpriteBatch.hpp"
#include "World.hpp"
#include <glm\gtx\rotate_vector.hpp>
#include <array>
#include "General.hpp"

struct BoxPoints final {

	BoxPoints() : m_points{ vec2(0.0f) } { }
	BoxPoints(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
		: m_points{ a, b, c, d } { }

	vec2& operator[](const size_t index) {
		return m_points[index];
	}
	const vec2& operator[] (const size_t index) const {
		return m_points[index];
	}

	vec2 center() const {
		return glm::mix(glm::mix(m_points[0], m_points[1], 0.5f),
						glm::mix(m_points[2], m_points[3], 0.5f), 0.5f);
	}

	constexpr size_t size() { return 4; }
private:
	vec2 m_points[4];
};

struct BoxEntity {
	bool isAlive = true;
	vec4 color = vec4(1.0f);
	float lifetime = 0.0f;
	float colormix = 0.0f;

	vec2 position = vec2(0.0f);
	vec2 velocity = vec2(0.0f);
	float rotation = 0.0f;
	float angularvelocity = 0.0f;
	bounds box = bounds(-0.5f, -0.5f, 0.5f, 0.5f);

	bounds GetBounds() const {
		bounds b;
		b.min = position + box.min;
		b.max = position + box.max;
		return b;
	}

	BoxPoints GetBoxPoints() const {
		const float rad = -glm::radians(rotation);
		return {
			glm::rotate(box.min, rad) + position,
			glm::rotate(vec2(box.min.x, box.max.y), rad) + position,
			glm::rotate(box.max, rad) + position,
			glm::rotate(vec2(box.max.x, box.min.y), rad) + position
		};
	}

	vec2 UpAxis() const {
		return glm::normalize(glm::rotate(vec2(0.0f, 1.0f), glm::radians(-rotation)));
	}

	vec2 RightAxis() const {
		return glm::normalize(glm::rotate(vec2(1.0f, 0.0f), glm::radians(-rotation)));
	}
};

struct BoxBattle {

	static void Init();
	static void Reset();
	static void Exit();

	static void Step(Timestep ts);
	static void Draw();

	static vec2 GetSelectedPos();

};

#endif // !BOXBATTLE_HPP