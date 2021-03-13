#include "BoxBattle.hpp"
#include <glm\gtx\norm.hpp>
#include "BoxParticles.hpp"

namespace {

	uint32 selectedentity = -1;
	uint32 laterfocus = -1;
	vec2 relselectpos;
	vec2 lastmousepos;
	vector<BoxEntity> entities;
	vector<BoxEntity> laterentities;
	constexpr float dragcoef = 0.994f;

}

// entity functions
void UpdateBox(uint32 index, Timestep ts);
void Explode(uint32 index);

void Select(uint32 index) {
	selectedentity = index;
	auto& world = GetWorld();
	auto& ent = entities[index];
	float rad = glm::radians(ent.rotation);
	relselectpos = glm::rotate(ent.box.Clamp(glm::rotate(world.mouse.worldpos - ent.position, rad)), -rad);
	lastmousepos = world.mouse.worldpos;
	ent.velocity = vec2(0.0f);
	ent.angularvelocity = 0.0f;
}
void Deselect() {
	if (selectedentity != -1) {
		selectedentity = -1;
	}
}

void Collide(uint32 e0index, uint32 e1index);

void Destroy(uint32 index) {
	if (selectedentity == index) {
		selectedentity = -1;
	}
	entities[index].isAlive = false;
}

uint32 Create() {
	for (size_t i = 0; i < entities.size(); i++) {
		if (!entities[i].isAlive) {
			new (&entities[i]) BoxEntity();
			return i;
		}
	}
	entities.push_back(BoxEntity());
	return entities.size() - 1;
}

void AddLater(BoxEntity& e, bool shouldlaterfocus = false) {
	laterentities.push_back(std::move(e));
	if (shouldlaterfocus) laterfocus = laterentities.size() - 1;
}

void DoAddLater() {
	for (size_t i = 0; i < laterentities.size(); i++) {
		uint32 ent = Create();
		entities[ent] = std::move(laterentities[i]);
		if (laterfocus == i) {
			laterfocus = -1;
			Select(ent);
		}
	}
	laterentities.clear();
}

void BoxBattle::Init() {
	//auto& ent = entities[Create()];
	//ent.velocity = vec2(1.5f, 1.5f);
	//ent.rotation = 45.0f;
	//ent.type = BoxEntity::EXPLOSIVE;
	//ent.box = bounds(5.0f, 5.0f);
	//entities.push_back(ent);

	for (size_t i = 0; i < 2; i++) {
		auto& ent = entities[Create()];
		ent.box = bounds(5.0f, 5.0f);
		if (i == 0) ent.position = vec2(-5.0f, 0.0f);
		else ent.position = vec2(5.0f, 0.0f);
	}

}

void BoxBattle::Reset() {
	Deselect();
	entities.clear();
	laterentities.clear();
	Init();
}

void BoxBattle::Exit() {
	entities.clear();
}

void BoxBattle::Step(Timestep ts) {
	World& world = GetWorld();
	bounds& camera = world.camera;

	DoAddLater();

	for (size_t i = 0; i < entities.size(); i++) {
		auto& ent = entities[i];
		if (!ent.isAlive) continue;
		ent.lifetime += ts;

		bool losefocus = true;

		if (ent.position.x < camera.left)
			ent.position.x += camera.Width();
		else if (ent.position.x > camera.right)
			ent.position.x -= camera.Width();
		else losefocus = false;

		if (losefocus && selectedentity == i) Deselect();
		losefocus = true;

		if (ent.position.y < camera.bottom)
			ent.position.y += camera.Height();
		else if (ent.position.y > camera.top)
			ent.position.y -= camera.Height();
		else losefocus = false;

		if (losefocus && selectedentity == i) Deselect();


		if (world.mouse.left.waspressed && (selectedentity == -1)) {
			bounds b = ent.GetBounds();
			if (b.Contains(world.mouse.worldpos, ent.rotation)) {
				Select(i);
			}
		}

		UpdateBox(i, ts);

	}

	for (size_t i = 0; i < entities.size(); i++) {
		if (!entities[i].isAlive) continue;
		for (size_t j = i + 1; j < entities.size(); j++) {
			if (entities[j].isAlive) {
				Collide(i, j);
			}
		}
		auto& ent = entities[i];

		ent.velocity *= dragcoef;
		ent.angularvelocity *= dragcoef;
		ent.position += ent.velocity * ts;
		ent.rotation += ent.angularvelocity * ts;

		if (selectedentity == i) {
			// rotate around relative pos
			relselectpos = glm::rotate(relselectpos, glm::radians(-ent.angularvelocity * ts));
		}

	}

	if (world.mouse.left.isheld && (selectedentity != -1)) {
		auto& ent = entities[selectedentity];
		if (!ent.isAlive) {
			Deselect();
			return;
		}

		float mousedelta = glm::length(lastmousepos - world.mouse.worldpos) * 10.0f;
		lastmousepos = world.mouse.worldpos;

		// calculate linear velocity
		vec2 targetpos = world.mouse.worldpos - relselectpos;
		vec2 dir = (targetpos - ent.position) * 0.2f;
		ent.velocity += dir + dir * mousedelta;

		// calculate angular velocity
		const float force = glm::dot(glm::normalize(vec2(-relselectpos.y, relselectpos.x)), dir);
		const float radius = glm::length(relselectpos);
		vec2 halfsize(ent.box.Width() * 0.5f, ent.box.Height() * 0.5f);
		const float amount = glm::length((world.mouse.worldpos - ent.position) / halfsize) * 0.5f;
		if (fabs(force) > 0.000001f)
			ent.angularvelocity += radius * -force * 100.0f * amount;

	} else {
		Deselect();
	}
}

void BoxBattle::Draw() {
	World& world = GetWorld();
	bounds& camera = world.camera;

	DoAddLater();

	for (size_t i = 0; i < entities.size(); i++) {
		auto& ent = entities[i];
		if (!ent.isAlive) continue;
		bounds b = ent.GetBounds();
		vec2 half(b.Width() * 0.5f, b.Height() * 0.5f);
		b.min -= half;
		b.max += half;

		SpriteBatch::DrawQuad(ent.position, ent.box, ent.color, ent.rotation);

		if (b.left < camera.left) {
			vec4 color = ent.color;
			vec2 pos = ent.position;
			pos.x += camera.Width();
			color.a = 0.2f;
			SpriteBatch::DrawQuad(pos, ent.box, color, ent.rotation);
		} else if (b.right > camera.right) {
			vec4 color = ent.color;
			bounds b0 = b;
			vec2 pos = ent.position;
			pos.x -= camera.Width();
			color.a = 0.2f;
			SpriteBatch::DrawQuad(pos, ent.box, color, ent.rotation);
		}

		if (b.bottom < camera.bottom) {
			vec4 color = ent.color;
			vec2 pos = ent.position;
			pos.y += camera.Height();
			color.a = 0.2f;
			SpriteBatch::DrawQuad(pos, ent.box, color, ent.rotation);
		} else if (b.top > camera.top) {
			vec4 color = ent.color;
			vec2 pos = ent.position;
			pos.y -= camera.Height();
			color.a = 0.2f;
			SpriteBatch::DrawQuad(pos, ent.box, color, ent.rotation);
		}

	}
}

vec2 BoxBattle::GetSelectedPos() {
	if (selectedentity != -1)
		return relselectpos + entities[selectedentity].position;
	return vec2(INFINITY);
}

void UpdateBox(uint32 index, Timestep ts) {
	//const static vec4 colorA = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const static vec4 colorB = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float maxspeed = 60.0f;
	float maxspeedinverse = 1.0f / maxspeed;

	float len = glm::length(entities[index].velocity);
	float delta = len * maxspeedinverse;
	float area = entities[index].box.Area();

	entities[index].colormix += ts * delta;
	vec4 colorA = ColorMix(entities[index].colormix);

	entities[index].color = colorA;
	//if (len > maxspeed) entities[index].color = colorA;
	//else if (len < 9.0f) entities[index].color = colorB;
	//else entities[index].color = glm::mix(colorB, colorA, (len - 9.0f) / (maxspeed - 9.0f));

	if (area > 1.0f && delta > (2.0f - (1.0f / 25.0f) * area)) {
		Explode(index);
	}

}

void Explode(uint32 index) {
	bounds box = bounds::MakeFromArea(entities[index].box.Area() * 0.25f);
	vec2 quart(box.Width(), box.Height());
	for (size_t i = 0; i < 4; i++) {
		uint32 nindex = Create();
		auto& nent = entities[nindex];

		vec2 offset = glm::rotate(quart, glm::radians(entities[index].rotation + 90.0f * float(i))) * 1.25f;
		nent.position = entities[index].position + offset * 1.12f;
		nent.rotation = entities[index].rotation;

		offset = glm::normalize(offset);
		nent.velocity = (entities[index].velocity + offset * glm::length(entities[index].velocity) * 0.1f) * 0.5f;
		nent.angularvelocity = entities[index].angularvelocity * glm::dot(glm::normalize(entities[index].velocity), offset);
		nent.box = box;

	}

	ParticleSystem::BoxExplode(entities[index].GetBounds(), entities[index].rotation, entities[index].colormix, entities[index].velocity);
	Destroy(index);
}

std::tuple<float, float> GetBoxRange(BoxPoints& points, const vec2& axis) {
	float min = FLT_MAX;
	float max = -FLT_MAX;
	for (size_t i = 0; i < points.size(); i++) {
		float val = glm::dot(points[i], axis);
		min = glm::min(val, min);
		max = glm::max(val, max);
	}
	return { min, max };
}

bool NearZero(float a) {
	return fabs(a) < 0.001f;
}

float BoxOverlapAxis(BoxPoints& p0, BoxPoints& p1, const vec2& axis) {
	auto [min0, max0] = GetBoxRange(p0, axis);
	auto [min1, max1] = GetBoxRange(p1, axis);
	if (min0 < max1 && min0 > min1) return glm::mix(max1, min1, 0.5f) - min0;
	if (min1 < max0 && min1 > min0) return glm::mix(max0, min0, 0.5f) - min1;
	return 0.0f;
}

void Collide(uint32 e0index, uint32 e1index) {
	BoxEntity& e0 = entities[e0index];
	BoxEntity& e1 = entities[e1index];

	if (fabs(glm::length(e0.velocity)) > 10.0f || fabs(glm::length(e1.velocity)) > 10.0f) return;

	auto p0 = e0.GetBoxPoints();
	auto p1 = e1.GetBoxPoints();

	vec2 up1 = e1.UpAxis();
	float a = BoxOverlapAxis(p0, p1, up1);
	if (NearZero(a)) return;

	vec2 right1 = e1.RightAxis();
	float b = BoxOverlapAxis(p0, p1, right1);
	if (NearZero(b)) return;

	vec2 up0 = e0.UpAxis();
	float c = BoxOverlapAxis(p0, p1, up0);
	if (NearZero(c)) return;

	vec2 right0 = e0.RightAxis();
	float d = BoxOverlapAxis(p0, p1, right0);
	if (NearZero(d)) return;

	// colliding
	BoxEntity ent;
	ent.isAlive = true;
	ent.color = glm::mix(e0.color, e1.color, 0.5f);
	ent.position = glm::mix(e0.position, e1.position, 0.5f);
	ent.velocity = glm::mix(e0.velocity, e1.position, 0.5f);
	ent.box = bounds::MakeFromArea(e0.box.Area() + e1.box.Area());
	//ent.box.min *= 0.75f;
	//ent.box.max *= 0.75f;
	ent.rotation = glm::mix(e0.rotation, e1.rotation, 0.5f);
	ent.angularvelocity = glm::mix(e0.angularvelocity, e1.angularvelocity, 0.5f);
	ent.colormix = glm::mix(e0.colormix, e1.colormix, 0.5f);
	AddLater(ent, (e0index == selectedentity) || (e1index == selectedentity));
	//SpriteBatch::DrawQuad(glm::mix(e0.position, ent.position, 0.5f), ent.box, ent.color * 0.5f, glm::mix(e0.rotation, ent.rotation, 0.5f));
	//SpriteBatch::DrawQuad(glm::mix(e1.position, ent.position, 0.5f), ent.box, ent.color * 0.5f, glm::mix(e1.rotation, ent.rotation, 0.5f));
	ParticleSystem::BoxMerge(entities[e0index].GetBounds(), entities[e0index].rotation, entities[e0index].colormix,
							 entities[e1index].GetBounds(), entities[e1index].rotation, entities[e1index].colormix,
							 ent.GetBounds(), glm::length(ent.velocity));
	Destroy(e0index);
	Destroy(e1index);
}
