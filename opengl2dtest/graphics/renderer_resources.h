#pragma once

#define TEXTURES_LAST TEXTURE_UI_TOOLBAR_HIGHLIGHT
enum TextureType
{
	TEXTURE_NONE = -1,
	TEXTURE_ATLAS_CHUNK = 0,
	TEXTURE_UI_CROSSHAIR,
	TEXTURE_UI_TOOLBAR,
	TEXTURE_UI_TOOLBAR_HIGHLIGHT
};

#define SHADERS_LAST SHADER_BASIC_COLOR
enum ShaderType {
	SHADER_CHUNK = 0,
	SHADER_OUTLINE,
	SHADER_BASIC_TEXTURE,
	SHADER_BASIC_COLOR
};