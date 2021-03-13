#ifndef BOX_PARTICLES_HPP
#define BOX_PARTICLES_HPP
#include "General.hpp"
#include "Core\Timer.hpp"
#include "SpriteBatch.hpp"

struct Particle {
	bool isAlive = true;
	float lifetime = 0.0f;
	vec2 position = vec2(0.0f);
	vec2 velocity = vec2(0.0f);
	bounds box = bounds::MakeFromArea(0.01f);

	float maxlifetime = 10.0f;
	float colormixoffset = 0.0f;
	float rotation = 0.0f;

};

struct ParticleSystem {

	static void Init();
	static void Reset();
	static void Exit();

	static void StartStep(Timestep ts);
	static void EndStep();
	//static void Step(Timestep ts);
	static void Draw();

	static void Shoot(const vec2& position, const vec2& velocity, const float maxlifetime);

	static void BoxMerge(const bounds& boxA, const float rotA, const float colormixA, 
						 const bounds& boxB, const float rotB, const float colormixB, 
						 const bounds& newbox, const float speed);
	static void BoxExplode(const bounds& box, const float rotation, const float colormix, const vec2& velocity, const float spacing = 0.13f);

};

#endif // !BOX_PARTICLES_HPP