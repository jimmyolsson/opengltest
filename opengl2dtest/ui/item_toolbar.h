#pragma once
#include "../graphics/renderer.h"

struct UIToolbar
{
	short item_selected;

	Quad toolbar;
	Quad highlight;
};

// TODO: Center
void menu_render(UIToolbar* self, Renderer* renderer, glm::mat4 view, float width, float height)
{
	renderer_render_quad(renderer, view, &self->toolbar);
	renderer_render_quad(renderer, view, &self->highlight);
}

UIToolbar menu_create(float screen_w, float screen_h)
{
	UIToolbar menu;
	menu.item_selected = 0;
	int menu_scale_x = 732;
	int menu_scale_y = 85;

	menu.toolbar = quad_create(TEXTURE_UI_TOOLBAR,
		glm::vec2(screen_w / 2 - menu_scale_x / 2, 0),
		glm::vec2(menu_scale_x, menu_scale_y));

	menu.highlight = quad_create(TEXTURE_UI_TOOLBAR_HIGHLIGHT,
		glm::vec2(309, 0),
		glm::vec2(98, 88));

	return menu;
}

