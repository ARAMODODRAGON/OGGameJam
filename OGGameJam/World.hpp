#ifndef WORLD_HPP
#define WORLD_HPP
#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Regions.hpp"
#include "cjs\cjs.hpp"

constexpr float camsize = 10.0f;
constexpr float camheight = 2.0f;
constexpr float camwidth = camheight * (16.0f / 9.0f);
constexpr size_t workercount = 3;

// singletons can go here, call GetWorld() to get it
struct World {
	bool isRunning = false;
	Window* window = nullptr;
	Timer timer;
	bounds camera = bounds(camwidth * camsize, camheight * camsize);

	struct {
		vec2 worldpos = vec2(0.0f);
		struct {
			bool isheld = false;
			bool waspressed = false;
		} right;
		struct {
			bool isheld = false;
			bool waspressed = false;
		} left;
		bool inFocus = false;
	} mouse;

	cjs::work_queue jobqueue;

};

inline World& GetWorld() {
	static World world;
	return world;
}

#endif // !WORLD_HPP