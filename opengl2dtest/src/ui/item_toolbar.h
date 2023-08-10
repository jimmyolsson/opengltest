#pragma once
#include "../graphics/renderer.h"

struct UIToolbar
{
	short item_selected;
	short scale = 1;

	glm::vec2 toolbar_size_initial = glm::vec2(182, 22);
	glm::vec2 highlight_size_initial = glm::vec2(24, 23);

	Quad toolbar;
	Quad highlight;
};

void menu_scale(UIToolbar* self, int width, int height, int scale)
{
	// Change scale
	self->toolbar.scale = self->toolbar_size_initial * (float)scale;
	self->highlight.scale = self->highlight_size_initial * (float)scale;

	// Reposition
	glm::vec2 toolbar_pos = glm::vec2(width / 2 - self->toolbar.scale.x / 2, 0);
	self->toolbar.position = toolbar_pos;

	glm::vec2 highlight_pos = glm::vec2((toolbar_pos.x - scale) + self->item_selected * (20*scale), 0);
	self->highlight.position = highlight_pos;
	self->scale = scale;
}

void menu_select_item(UIToolbar* self, int item)
{
	self->item_selected = item;
	self->highlight.position.x = ((self->toolbar.position.x - self->scale) + (20 * self->scale) * item);
}

void menu_select_next(UIToolbar* self)
{
	self->item_selected--;
	menu_select_item(self, self->item_selected);
}

void menu_select_prev(UIToolbar* self)
{
	self->item_selected++;
	menu_select_item(self, self->item_selected);
}

void menu_render(UIToolbar* self, Renderer* renderer, glm::mat4 view, float width, float height)
{
	renderer_render_quad(renderer, view, &self->toolbar);
	renderer_render_quad(renderer, view, &self->highlight);
}

UIToolbar menu_create(float screen_w, float screen_h)
{
	UIToolbar menu;
	menu.item_selected = 0;

	glm::vec2 toolbar_pos = glm::vec2(screen_w / 2 - menu.toolbar_size_initial.x / 2, 0);
	menu.toolbar = quad_create(TEXTURE_UI_TOOLBAR,
		toolbar_pos,
		menu.toolbar_size_initial);

	menu.highlight = quad_create(TEXTURE_UI_TOOLBAR_HIGHLIGHT,
		glm::vec2(toolbar_pos.x - menu.scale, toolbar_pos.y + menu.scale), // pos
		menu.highlight_size_initial); // scale

	return menu;
}

