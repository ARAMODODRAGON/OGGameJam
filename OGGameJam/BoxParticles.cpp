#include "BoxParticles.hpp"
#include "World.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <array>

namespace {

	vector<Particle> particles;
	bounds particlebounds(0.1f, 0.1f);
	constexpr float dragcoef = 0.994f;

	Timestep timestep;

	using functype = std::function<void(size_t, Particle&)>;
	using guard = std::lock_guard<std::mutex>;
	std::mutex spawnslock;

	struct ParticleJob : cjs::ijob {
		size_t begin = 0;
		size_t end = 0;

		void execute() override;
	};

	struct SpawnJob : cjs::ijob {
		std::atomic_bool complete = false;
		size_t count;
		functype callable;

		void execute() override;
	};

	std::array<ParticleJob, workercount> jobs;
	std::vector<std::shared_ptr<SpawnJob>> spawns;
	cjs::fence particlefence;

}

static void Step(Timestep ts, size_t index, Particle& p);
static void DoSpawn(size_t count, functype callable);

static void Step(Timestep ts, size_t index, Particle& p) {
	if (!p.isAlive) return;
	bounds camera = GetWorld().camera;

	p.position += p.velocity * ts;
	p.velocity *= dragcoef;
	p.lifetime += ts;

	if (p.position.x < camera.left)
		p.position.x += camera.Width();
	else if (p.position.x > camera.right)
		p.position.x -= camera.Width();

	if (p.position.y < camera.bottom)
		p.position.y += camera.Height();
	else if (p.position.y > camera.top)
		p.position.y -= camera.Height();

	if (p.lifetime > p.maxlifetime)
		p.isAlive = false;
}

//static void Loop() {
//	while (!shouldquit) {
//		// wait
//		while (!beginstep) {
//			if (shouldquit) return;
//		}
//
//		// step
//		for (size_t i = 0; i < particles.size(); i++) {
//			Step(timestep, i, particles[i]);
//		}
//
//		// want to do this at least once
//		do {
//			guard _(spawnslock);
//			for (auto [count, func] : schedualedspawns) {
//				DoSpawn(count, func);
//			}
//			schedualedspawns.clear();
//		} while (!shouldendstep);
//
//		// make sure the other thread can continue
//		shouldendstep = false;
//		endstep = true;
//		beginstep = false;
//	}
//}

static void DoSpawn(size_t count, functype callable) {
	// spawn using existing particles
	for (size_t i = 0; i < particles.size() && count > 0; i++) {
		if (!particles[i].isAlive) {
			new (&particles[i]) Particle();
			--count;
			callable(i, particles[i]);
		}
	}
	// reserve more memory as needed
	if (count > (particles.capacity() - particles.size())) {
		particles.reserve(particles.capacity() * 2);
	}
	// emplace some more as needed
	while (count > 0) {
		particles.emplace_back();
		callable(particles.size() - 1, particles[particles.size() - 1]);
		--count;
	}
}

void SpawnJob::execute() {
	guard _(spawnslock);
	// spawn using existing particles
	for (size_t i = 0; i < particles.size() && count > 0; i++) {
		if (!particles[i].isAlive) {
			new (&particles[i]) Particle();
			--count;
			callable(i, particles[i]);
		}
	}
	//// reserve more memory as needed
	//if (count > (particles.capacity() - particles.size())) {
	//	particles.reserve(particles.capacity() * 2);
	//}
	//// emplace some more as needed
	//while (count > 0) {
	//	particles.emplace_back();
	//	callable(particles.size() - 1, particles[particles.size() - 1]);
	//	--count;
	//}
	complete = true;
}

void ParticleJob::execute() {
	for (size_t i = begin; i < end; i++) {
		Step(timestep, i, particles[i]);
	}
}

template<typename Callable>
static void Spawn(size_t count, Callable callable) {
	//guard _(spawnslock);
	//schedualedspawns.emplace_back(count, callable);
	for (size_t i = 0; i < spawns.size(); i++) {
		if (spawns[i]->complete) {
			spawns[i]->complete = false;
			spawns[i]->callable = callable;
			spawns[i]->count = count;
			GetWorld().jobqueue.submit(spawns[i].get());
			return;
		}
	}
	std::shared_ptr<SpawnJob> spjob(new SpawnJob());
	spjob->complete = false;
	spjob->callable = callable;
	spjob->count = count;
	GetWorld().jobqueue.submit(spjob.get());
	spawns.push_back(spjob);
}

void ParticleSystem::Init() {
	particles.clear();
	particles.resize(2000);
	for (size_t i = 0; i < particles.size(); i++) {
		particles[i].isAlive = false;
	}
}

void ParticleSystem::Reset() {
	auto& world = GetWorld();
	particlefence.await_and_resume();
	for (size_t i = 0; i < particles.size(); i++) {
		particles[i].isAlive = false;
	}
	spawns.clear();
	world.jobqueue.submit(&particlefence);
}

void ParticleSystem::Exit() {
	particles.clear();
	spawns.clear();
}

void ParticleSystem::StartStep(Timestep ts) {
	auto& world = GetWorld();

	world.jobqueue.submit(&particlefence);
	particlefence.await_and_resume();

	timestep = ts;

	size_t range = particles.size() / jobs.size();
	for (size_t i = 0; i < jobs.size(); i++) {
		jobs[i].begin = range * i;
		if ((i + 1) != jobs.size())
			jobs[i].end = jobs[i].begin + range;
		else
			jobs[i].end = particles.size();
		world.jobqueue.submit(&(jobs[i]));
	}
	world.jobqueue.submit(&particlefence);
}

void ParticleSystem::EndStep() {
	auto& world = GetWorld();
	particlefence.await_and_resume();
}

void ParticleSystem::Draw() {
	for (size_t i = 0; i < particles.size(); i++) {
		if (!particles[i].isAlive) continue;
		SpriteBatch::DrawQuad(particles[i].position, particles[i].box,
							  ColorMix(particles[i].lifetime + particles[i].colormixoffset),
							  -glm::radians(particles[i].rotation));
	}
}

void ParticleSystem::Shoot(const vec2& position, const vec2& velocity, const float maxlifetime) {
	Spawn(1, [position, velocity, maxlifetime](size_t index, Particle& p) {
		p.position = position;
		p.velocity = velocity;
		p.maxlifetime = maxlifetime;
	});
}

vec2 RotateAround(const vec2& pos, const vec2& around, const float rad) {
	return glm::rotate(pos - around, rad) + around;
}

void ParticleSystem::BoxMerge(const bounds& boxA, const float rotA, const float colormixA,
							  const bounds& boxB, const float rotB, const float colormixB,
							  const bounds& newbox, const float speed) {
	vec2 targetpos = newbox.Center();
	vec2 velA = glm::normalize(targetpos - boxA.Center()) * speed;
	vec2 velB = glm::normalize(targetpos - boxB.Center()) * speed;
	BoxExplode(boxA, rotA, colormixA, velA, 0.13f);
	BoxExplode(boxB, rotB, colormixB, velB, 0.13f);
}

void ParticleSystem::BoxExplode(const bounds& box, const float rotation, const float colormix, const vec2& velocity, const float spacing) {

	vec2 bsize = box.Size();
	vec2 pos = box.Center();
	vec2 psize = particlebounds.Size();
	vec2 extrasize = psize + vec2(spacing);
	size_t widthcount = glm::min(static_cast<size_t>(bsize.x / extrasize.x), 20u);
	size_t heightcount = glm::min(static_cast<size_t>(bsize.x / extrasize.y), 20u);
	size_t totalcount = widthcount * heightcount;
	float rad = glm::radians(-rotation);
	vec2 startpos(box.left + (extrasize.x * 0.5f), box.bottom + (extrasize.y * 0.5f));
	//float scalar = glm::length(velocity) * (1.0f / 60.0f);

	Spawn(totalcount, [startpos, widthcount, pos, extrasize, rad, velocity, colormix](size_t index, Particle& p) {
		size_t xpos = index / widthcount;
		size_t ypos = index % widthcount;
		p.position = RotateAround(vec2(xpos * extrasize.x, ypos * extrasize.y) + startpos, pos, rad);
		p.velocity = velocity * RandomRange(0.2f, 0.4f) + (p.position - pos) * RandomRange(0.5f, 5.0f);
		p.colormixoffset = colormix + RandomRange(0.0f, 0.2f);
		p.maxlifetime = RandomRange(2.0f, 4.0f);
		p.rotation = -glm::degrees(rad);
		p.box = bounds::MakeFromArea(RandomRange(0.005f, 0.05f));
	});

}
