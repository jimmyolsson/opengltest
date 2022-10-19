#include "shader.h"
#include "glad/glad.h"

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
		glGetProgramiv(shader_handle, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader_handle, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader_handle, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

void shader_load(shader_program* sp, const char* path, unsigned int type)
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
		std::cout << "SHADER::FILE_NOT_SUCCESSFULLY_READ, " << path << "\n";
	}

	const char* code = fileContents.c_str();

	//Compile
	sp->size++;
	sp->shaders[sp->size] = glCreateShader(type);

	glShaderSource(sp->shaders[sp->size], 1, &code, NULL);
	glCompileShader(sp->shaders[sp->size]);
	check_errors(sp->shaders[sp->size], false);
}

void shader_link(shader_program* sp)
{
	sp->handle = glCreateProgram();

	for (int i = 0; i <= sp->size; i++)
		glAttachShader(sp->handle, sp->shaders[i]);

	glLinkProgram(sp->handle);
	check_errors(sp->handle, true);

	for (int i = 0; i < sp->size; i++)
		glDeleteShader(sp->shaders[i]);
}

void shader_use(shader_program* sp)
{
	glUseProgram(sp->handle);
}

// TODO: log properly, uniform shit fails silently
void shader_set_mat4(shader_program* sp, const char* name, glm::mat4 value)
{
	GLint l = glGetUniformLocation(sp->handle, name);
	glUniformMatrix4fv(l, 1, GL_FALSE, &value[0][0]);
}

void shader_set_int(shader_program* sp, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(sp->handle, name), value);
}