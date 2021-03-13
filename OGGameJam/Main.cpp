#include <SDL.h>
#include "World.hpp"
#include "SpriteBatch.hpp"
#include <glm\gtc\matrix_transform.hpp>
#include "BoxBattle.hpp"
#include "BoxParticles.hpp"

vec2 OutBorderDir(const vec2& a, const vec2& b, const vec2& pos, float len) {
	return glm::normalize(glm::normalize(pos - a) + glm::normalize(pos - b)) * len;
}

void PollEvents() {
	static World& world = GetWorld();
	static SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT: world.isRunning = false; break;
			case SDL_KEYDOWN:
				if (e.key.repeat == 0 && e.key.keysym.scancode == SDL_SCANCODE_R) {
					BoxBattle::Reset();
					ParticleSystem::Reset();
				}
				break;
			default: break;
		}
	}

	int x = 0, y = 0;
	Uint32 state = SDL_GetMouseState(&x, &y);
	vec2 windowsize = world.window->GetScreenSize();
	bool washeld = world.mouse.left.isheld;

	vec2 camerasize(world.camera.Width(), world.camera.Height());
	vec2 topleft(world.camera.left, world.camera.top);
	world.mouse.worldpos = topleft + (vec2(x, -y) / world.window->GetScreenSize()) * camerasize;

	if (SDL_GetMouseFocus() == world.window->GetSDLWindow())
		world.mouse.inFocus = true;
	else world.mouse.inFocus = false;

	world.mouse.left.isheld = SDL_BUTTON(SDL_BUTTON_LEFT) & state;
	if (!washeld && world.mouse.left.isheld)	world.mouse.left.waspressed = true;
	else										world.mouse.left.waspressed = false;
	world.mouse.right.isheld = SDL_BUTTON(SDL_BUTTON_RIGHT) & state;
	if (!washeld && world.mouse.right.isheld)	world.mouse.right.waspressed = true;
	else										world.mouse.right.waspressed = false;
}

int main(int argc, char** argv) {

	World& world = GetWorld();

	// init jobs
	std::array<cjs::worker_thread, workercount> workers;
	for (size_t i = 0; i < workers.size(); i++) {
		workers[i].attach_to(&(world.jobqueue));
	}

	// add the window to the world
	Window window("Box Battler", uvec2(1280, 720));
	world.window = &window;

	SDL_ShowCursor(SDL_DISABLE);

	// set frame rate
	world.timer.SetTargetFPS(60);

	// init the spritebatch and boxbattle
	SpriteBatch::Init();
	BoxBattle::Init();
	ParticleSystem::Init();

	// begin the game
	world.isRunning = true;
	while (world.isRunning) {
		// timer stuff
		world.timer.BeginFrame();

		// update 
		Timestep ts(world.timer.GetDelta());
		ParticleSystem::StartStep(ts);
		PollEvents();
		BoxBattle::Step(ts);
		OGJ_DEBUG_LOG("FPS: " + VTOS(ts.GetFPS()));

		// render
		window.ClearScreen(vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mat4 transform = glm::ortho(world.camera.left, world.camera.right, world.camera.bottom, world.camera.top);
		SpriteBatch::Begin(transform);

		// draw 
		BoxBattle::Draw();
		ParticleSystem::EndStep(); // end the step over here at the very latest
		ParticleSystem::Draw();

		if (world.mouse.inFocus) {
			const static bounds b0(0.23f, 0.23f);
			const static vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
			const static vec4 black(0.0f, 0.0f, 0.0f, 1.0f);
			vec2 targetpos = BoxBattle::GetSelectedPos();

			constexpr size_t count = 20;
			constexpr float angledelta = 270.0f / float(count);
			for (size_t i = 0; i < count; i++) {
				float angle = angledelta * float(i);
				SpriteBatch::DrawQuad(targetpos, b0, black, angle);
			}

			const static vertex verts[] = {
				{ vec2(0.0f,  0.0f),  white },  // top left
				{ vec2(0.15f, -0.3f), white },  // bottom left
				{ vec2(0.3f, -0.15f), white },  // top right
			};
			constexpr float thickness = 0.15f;
			const static vertex outlineverts[] = {
				{ verts[0].position + OutBorderDir(verts[1].position, verts[2].position, verts[0].position, thickness), black },
				{ verts[1].position + OutBorderDir(verts[0].position, verts[2].position, verts[1].position, thickness), black },
				{ verts[2].position + OutBorderDir(verts[0].position, verts[1].position, verts[2].position, thickness), black }
			};

			// draw cursor outline
			SpriteBatch::DrawVerts(world.mouse.worldpos + vec2(thickness, -thickness), outlineverts[0], outlineverts[1], outlineverts[2]);
			// draw cursor
			SpriteBatch::DrawVerts(world.mouse.worldpos + vec2(thickness, -thickness), verts[0], verts[1], verts[2]);

		}

		SpriteBatch::End();
		window.SwapBuffers();
		//world.timer.WaitForEndOfFrame();
		world.timer.EndFrame();
	}

	for (size_t i = 0; i < workers.size(); i++) {
		workers[i].attach_to(nullptr);
	}
	ParticleSystem::Exit();
	BoxBattle::Exit();
	SpriteBatch::Exit();
	return 0;
}