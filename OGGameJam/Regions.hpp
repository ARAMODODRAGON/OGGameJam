#ifndef REGIONS_HPP
#define REGIONS_HPP
#include "General.hpp"
#include <glm\gtx\vector_angle.hpp>

struct rect {
	union {
		struct { vec2 position, size; };
		struct { float x, y, w, h; };
	};
	rect() : position(0.0f), size(0.0f) { }
	rect(float x_, float y_, float w_, float h_)
		: x(x_), y(y_), w(w_), h(h_) { }
};

struct bounds {
	union {
		struct { vec2 min, max; };
		struct { float left, bottom, right, top; };
	};
	bounds() : min(0.0f), max(0.0f) { }
	bounds(float width, float height)
		: left(-width * 0.5f), bottom(-height * 0.5f), right(width * 0.5f), top(height * 0.5f) { }
	bounds(const vec2& min_, const vec2& max_)
		: min(min_), max(max_) { }
	bounds(float left_, float bottom_, float right_, float top_)
		: left(left_), bottom(bottom_), right(right_), top(top_) { }

	float Width() const {
		return right - left;
	}

	float Height() const {
		return top - bottom;
	}

	float Area() const {
		return Width() * Height();
	}

	vec2 Size() const {
		return vec2(Width(), Height());
	}

	vec2 Center() const {
		return vec2(right + left, top + bottom) * 0.5f;
	}

	bool Contains(const bounds& other) {
		if (other.left < this->left || other.right > this->right) return false;
		if (other.bottom < this->bottom || other.top > this->top) return false;
		return true;
	}

	bool Contains(const vec2& p) {
		if (p.x < left || p.x > right) return false;
		if (p.y < bottom || p.y > top) return false;
		return true;
	}

	vec2 Clamp(const vec2& p) {
		return vec2(glm::clamp(p.x, left, right), glm::clamp(p.y, bottom, top));
	}

	bool Contains(const vec2& p, const float rotation) {
		const vec2 center = glm::mix(min, max, 0.5f);
		vec2 pos = glm::rotate(p - center, glm::radians(rotation)) + center;
		if (pos.x < left || pos.x > right) return false;
		if (pos.y < bottom || pos.y > top) return false;
		return true;
	}

	static bounds MakeFromArea(const float area) {
		float sidelen = sqrtf(area);
		return bounds(sidelen, sidelen);
	}

	static bool Intersects(const bounds& b0, const bounds& b1) {
		if (b0.left > b1.right || b0.right < b1.left) return false;
		if (b0.bottom > b1.top || b0.top < b1.bottom) return false;
		return true;
	}

};

#endif // !REGIONS_HPP