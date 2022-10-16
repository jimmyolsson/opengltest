#pragma once

struct ItemMenu
{
	unsigned int vao;
	unsigned int vbo;
	glm::vec3 position;
	shader_program sp;

	unsigned int handle;
};

void menu_loadtexture(ItemMenu* self)
{
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("..\\resources\\textures\\gui\\item_menu.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	self->handle = texture1;
}

void menu_render(ItemMenu* menu, glm::mat4 p, glm::mat4 v)
{
	shader_use(&menu->sp);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, menu->handle);

	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::scale(transform, glm::vec3(0.8f, 0.15f, 1.0f));
	transform = glm::translate(transform, glm::vec3(-0.5f, -6.67f, 0.0f));

	shader_set_mat4(&menu->sp, "transform", transform);
	shader_set_int(&menu->sp, "texture1", menu->handle);

	glBindVertexArray(menu->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void static quad_create(unsigned int* vao, unsigned int* vbo)
{
	int index = 0;
	std::vector<int> gpu_data;
	for (int i = 0; i < 30 - 1; i++)
	{
		gpu_data.push_back(m_back_verticies[i]);
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

ItemMenu menu_create()
{
	ItemMenu menu;

	shader_load(&menu.sp, "..\\resources\\shaders\\basic_texture_vert.glsl", GL_VERTEX_SHADER);
	shader_load(&menu.sp, "..\\resources\\shaders\\basic_texture_frag.glsl", GL_FRAGMENT_SHADER);
	shader_link(&menu.sp);

	quad_create(&menu.vao, &menu.vbo);

	return menu;
}

