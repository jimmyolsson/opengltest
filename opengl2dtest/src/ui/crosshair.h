#pragma once
#include "../util/common_graphics.h"
#include "../graphics/renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

struct UICrosshair
{
	Quad quad;
};

void crosshair_scale(UICrosshair* self, int scale)
{
	
}

void crosshair_render(UICrosshair* self, Renderer* renderer, glm::mat4 view, float screen_w, float screen_h)
{
	int size = 20;
	self->quad.position = glm::vec2(screen_w / 2 - size / 2, screen_h / 2 - size / 2);
	renderer_render_quad(renderer, view, &self->quad);
}

UICrosshair crosshair_create(float screen_w, float screen_h)
{
	UICrosshair crosshair{};

	int size = 20;
	crosshair.quad = quad_create(TEXTURE_UI_CROSSHAIR,
		glm::vec2(screen_w / 2 - size / 2, screen_h / 2 - size / 2),
		glm::vec2(size, size));

	return crosshair;
}

