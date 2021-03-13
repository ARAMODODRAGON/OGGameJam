#include "SpriteBatch.hpp"
#include <glm\gtc\matrix_transform.hpp>
#include "Core/Shader.hpp"
#include <glew.h>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtc\matrix_transform.hpp>

#pragma region Shader

static const char* spriteshadersource = R""(
#type vertex
#version 450 core
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;

uniform mat4 u_transform;

out vec4 v_color;

void main() {
	v_color = a_color;
	gl_Position = u_transform * vec4(a_position, 0.0, 1.0);
}

#type fragment
#version 450 core
out vec4 out_fragcolor;

in vec4 v_color;

void main() {
	out_fragcolor = v_color;
}

)"";

#pragma endregion

namespace {

	bool isDrawing = false;
	vector<vertex> verticies;
	uint32 VBO = -1, VAO = -1;
	uint32 bufferSize = 0;
	uint32 shader = -1;
	uint32 transformLoc = -1;

}

void SpriteBatch::Init() {
	verticies.reserve(100);

	// load shader
	shader = LoadShaderSource(spriteshadersource);
	transformLoc = glGetUniformLocation(shader, "u_transform");

	// create our VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// create our VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

	// set the attributes

	// set the position to location 0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)offsetof(vertex, position));

	// set the color to location 1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)offsetof(vertex, color));

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteBatch::Exit() {
	glDeleteProgram(shader);
}

void SpriteBatch::Begin(const vec2& screensize) {
	mat4 transform = glm::ortho(0.0f, screensize.x, screensize.y, 0.0f);
	Begin(transform);
}

void SpriteBatch::Begin(const rect& region) {
	mat4 transform = glm::ortho(region.x, region.x + region.w, region.y + region.h, region.y);
	Begin(transform);
}

void SpriteBatch::Begin(const mat4& transform) {
	if (isDrawing) End();
	isDrawing = true;

	// use shader
	glUseProgram(shader);

	// bind vertex array and buffer
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// set uniform data
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &(transform[0].x));

}

void SpriteBatch::End() {
	if (verticies.size() == 0)
		return;
	const uint32 bytes = (sizeof(vertex) * verticies.size());

	// resize the buffer if needed
	if (bufferSize < bytes) {
		if (bufferSize == 0) bufferSize = bytes;
		else bufferSize *= 2;
		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);
	}

	// update the vertex data
	glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, verticies.data());

	// draw
	glDrawArrays(GL_TRIANGLES, 0, verticies.size());

	// clear out vector
	verticies.clear();

	// we are no longer drawing
	isDrawing = false;
}

void SpriteBatch::DrawQuad(const rect& position, const vec4& color) {

	const vec2 min = position.position;
	const vec2 max = position.position + position.size;

	// create verticies
	vertex verts[4];

	/* bottom left  */ verts[0].position = min;
	/* top left     */ verts[1].position = vec2(min.x, max.y);
	/* top right    */ verts[2].position = max;
	/* bottom right */ verts[3].position = vec2(max.x, min.y);

	// set color
	verts[0].color = verts[1].color
		= verts[2].color = verts[3].color = color;

	// submit our verticies to be drawn;
	DrawVerts(verts[0], verts[1], verts[2]);
	DrawVerts(verts[0], verts[2], verts[3]);

	// finish
}

void SpriteBatch::DrawQuad(const bounds& position, const vec4& color) {

	// create verticies
	vertex verts[4];

	/* bottom left  */ verts[0].position = position.min;
	/* top left     */ verts[1].position = vec2(position.min.x, position.max.y);
	/* top right    */ verts[2].position = position.max;
	/* bottom right */ verts[3].position = vec2(position.max.x, position.min.y);

	// set color
	verts[0].color = verts[1].color
		= verts[2].color = verts[3].color = color;

	// submit our verticies to be drawn;
	DrawVerts(verts[0], verts[1], verts[2]);
	DrawVerts(verts[0], verts[2], verts[3]);

	// finish
}

void SpriteBatch::DrawQuad(const vec2& position, const bounds& box, const vec4& color, const float rotation) {

	// create verticies
	vertex verts[4];

	/* bottom left  */ verts[0].position = box.min;
	/* top left     */ verts[1].position = vec2(box.min.x, box.max.y);
	/* top right    */ verts[2].position = box.max;
	/* bottom right */ verts[3].position = vec2(box.max.x, box.min.y);

	const float rad = -glm::radians(rotation);
	for (size_t i = 0; i < 4; i++) {
		verts[i].position = glm::rotate(verts[i].position, rad) + position;
	}

	// set color
	verts[0].color = verts[1].color
		= verts[2].color = verts[3].color = color;

	// submit our verticies to be drawn;
	DrawVerts(verts[0], verts[1], verts[2]);
	DrawVerts(verts[0], verts[2], verts[3]);

	// finish
}

void SpriteBatch::DrawVerts(const vertex& sv0, const vertex& sv1, const vertex& sv2) {
	if (verticies.capacity() - 3 <= verticies.size())
		verticies.reserve(verticies.capacity() * 2);
	verticies.push_back(sv0);
	verticies.push_back(sv1);
	verticies.push_back(sv2);
}

void SpriteBatch::DrawVerts(const vec2& offset, vertex sv0, vertex sv1, vertex sv2) { 
	sv0.position += offset;
	sv1.position += offset;
	sv2.position += offset;
	DrawVerts(sv0, sv1, sv2);
}

bool SpriteBatch::IsDrawing() {
	return isDrawing;
}
