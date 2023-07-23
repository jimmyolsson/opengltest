#include "shader.h"
#include "../util/common_graphics.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// This shouldnt output to console but w/e
void check_errors(unsigned int shader_handle, bool linking)
{
	GLint success;
	GLchar infoLog[1024];
	if (linking)
	{
		GL_CALL(glGetProgramiv(shader_handle, GL_LINK_STATUS, &success));
		if (!success)
		{
			GL_CALL(glGetProgramInfoLog(shader_handle, 1024, NULL, infoLog));
			
			g_logger_error("SHADER::Linking failed: %s", infoLog);
		}
	}
	else
	{
		GL_CALL(glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success));
		if (!success)
		{
			GL_CALL(glGetShaderInfoLog(shader_handle, 1024, NULL, infoLog));
			g_logger_error("SHADER::Compilation failed: %s", infoLog);
		}
	}
}

void _load_from_file_and_compile(ShaderProgram* sp, const char* path, unsigned int type)
{
	std::string fileContents;
	std::ifstream file;
	try
	{
		file.open(path);

		std::stringstream file_stream;

		file_stream << file.rdbuf();
		file.close();

		fileContents = file_stream.str();
	}
	catch (std::ifstream::failure& e)
	{
		g_logger_error("SHADER::Failed to read file: %s", path);
	}

	const char* code = fileContents.c_str();

	//Compile
	g_logger_debug("Compiling shader from: %s - type: %d", path, type);
	sp->size++;
	sp->shaders[sp->size] = glCreateShader(type);

	GL_CALL(glShaderSource(sp->shaders[sp->size], 1, &code, NULL));
	GL_CALL(glCompileShader(sp->shaders[sp->size]));
	check_errors(sp->shaders[sp->size], false);
}

void _link(ShaderProgram* sp)
{
	sp->handle = glCreateProgram();

	for (int i = 0; i <= sp->size; i++)
		GL_CALL(glAttachShader(sp->handle, sp->shaders[i]));

	GL_CALL(glLinkProgram(sp->handle));
	check_errors(sp->handle, true);

	for (int i = 0; i < sp->size; i++)
		GL_CALL(glDeleteShader(sp->shaders[i]));
}

ShaderProgram shader_create(const char* vs_path, const char* fs_path)
{
	ShaderProgram sp;
	_load_from_file_and_compile(&sp, vs_path, GL_VERTEX_SHADER);
	_load_from_file_and_compile(&sp, fs_path, GL_FRAGMENT_SHADER);
	_link(&sp);

	return sp;
}

void shader_use(ShaderProgram* sp)
{
	glUseProgram(sp->handle);
}

// TODO: log properly, uniform shit fails silently
void shader_set_mat4(ShaderProgram* sp, unsigned int location, glm::mat4 value)
{
	glUniformMatrix4fv(sp->uniform_locations[location], 1, GL_FALSE, &value[0][0]);
}

void shader_set_int(ShaderProgram* sp, unsigned int location, int value)
{
	glUniform1i(sp->uniform_locations[location], value);
}