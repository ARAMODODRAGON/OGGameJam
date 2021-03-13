#include "Timer.hpp"
#include "Debugger.hpp"

Timer::Timer() : targetFPS(0), lastDelta(0.0) {
	SetTargetFPS(60);
	lastTime = currentTime = steady_clock::now();
}

Timer::~Timer() { }

Timestep Timer::GetTimestep() const {
	return Timestep(lastDelta);
}

void Timer::BeginFrame() {
	lastDelta = (currentTime - lastTime).count();
	// save the previous time point
	lastTime = currentTime;
}

void Timer::EndFrame() {
	currentTime = steady_clock::now();
}

void Timer::WaitForEndOfFrame() {
	// get new time
	time_point t = steady_clock::now();

	// loop as long as this frame is not done yet
	while ((t - lastTime).count() < secondsPerFrame) {
		t = steady_clock::now();
	}

	currentTime = t;
}

bool Timer::CheckIfFrameComplete() {
	// check if this frame is done yet
	if ((steady_clock::now() - lastTime).count() > secondsPerFrame) {
		// assume that the caller wants to update time
		currentTime = steady_clock::now();
		return true;
	}
	return false;
}

float Timer::GetFPS() const {
	return static_cast<float>(1.0 / lastDelta);
}

float Timer::GetDelta() const {
	return static_cast<float>(lastDelta);
}
