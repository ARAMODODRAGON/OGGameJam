#ifndef SPRITEBATCH_HPP
#define SPRITEBATCH_HPP
#include "General.hpp"
#include "Regions.hpp"

struct vertex {
	vec2 position;
	vec4 color;
};

struct SpriteBatch {

	static void Init();
	static void Exit();

	// begin drawing using an orthographic view based on *screensize*
	static void Begin(const vec2& screensize);

	// begin drawing using an orthographic view based on *region*
	static void Begin(const rect& region);

	// begin drawing using the given transform
	static void Begin(const mat4& transform);

	// finish drawing 
	static void End();

	// draws a quad using the given *position* and *color*
	static void DrawQuad(const rect& position, const vec4& color);

	// draws a quad using the bounds and color
	static void DrawQuad(const bounds& position, const vec4& color);
	
	// draws a quad using the bounds, color and the given rotation
	static void DrawQuad(const vec2& position, const bounds& box, const vec4& color, const float rotation);

	// draws a triangle to the screen using the three given verticies
	static void DrawVerts(const vertex& sv0, const vertex& sv1, const vertex& sv2);

	// draws a triangle to the screen using the three given verticies and the positional offset
	static void DrawVerts(const vec2& offset, vertex sv0, vertex sv1, vertex sv2);

	// checks if this is in a draw state
	static bool IsDrawing();

};

#endif // !SPRITEBATCH_HPP