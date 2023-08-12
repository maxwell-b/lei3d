﻿#include "Shader.hpp"
#include "logging/GLDebug.hpp"

namespace lei3d 
{
	Shader::Shader()
	{
		// another clown emoji
	}

	Shader::Shader(const char* vertexShaderPath, const char* fragShaderPath)
	{
		// read from the files 
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			// open the files
			vShaderFile.open(vertexShaderPath);
			fShaderFile.open(fragShaderPath);
			std::stringstream vShaderStream;
			std::stringstream fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (const std::ifstream::failure& e)
		{
			if (vShaderFile.fail())
			{
				LEI_ERROR("ERROR - Failed to open vertex shader file: " + std::string(vertexShaderPath));
			}
			else if (fShaderFile.fail())
			{
				LEI_ERROR("ERROR - Failed to open fragment shader file: " + std::string(fragShaderPath));
			}
			else
			{
				// in my experience this outputs something like "basic_ios::clear: iostream error"
				// for something like a file not found. preferably the specific error would be output
				LEI_ERROR("ERROR - Shader File Not Successfully Read: " + std::string(e.what()));
			}
		}
		const char* vShaderCode = vertexCode.c_str(); // yeah I can work with c strings ( ͡° ͜ʖ ͡°)
		const char* fShaderCode = fragmentCode.c_str();

		// compile and configure shaders
		char infoLog[512];

		GLCall(unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER));
		GLCall(glShaderSource(vertexShaderID, 1, &vShaderCode, NULL));
		glCompileShader(vertexShaderID);

		int success;
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
			LEI_ERROR("VERTEX SHADER COMPILATION FAILED\n\n" + std::string(infoLog));
		}

		unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShaderID, 1, &fShaderCode, NULL);
		glCompileShader(fragmentShaderID);

		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
			LEI_ERROR("FRAGMENT SHADER COMPILATION FAILED\n\n" + std::string(infoLog));
		}

		m_ShaderID = glCreateProgram(); // member variable
		glAttachShader(m_ShaderID, vertexShaderID);
		glAttachShader(m_ShaderID, fragmentShaderID);
		glLinkProgram(m_ShaderID);

		glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
			LEI_ERROR("SHADER PROGRAM LINKING FAILED\n\n" + std::string(infoLog));
		}

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}

	/** 
	 * Set OpenGL to use this shader. 
	 */
	void Shader::bind() {
		GLCall(glUseProgram(m_ShaderID));
	}

	void Shader::unbind() {
		GLCall(glUseProgram(0));
	}

	void Shader::setUniformMat4(const std::string& name, glm::mat4& matrix)
	{
		bind();
		GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
		unbind();
	}

	void Shader::setInt(const std::string &name, int value)
    { 
		bind();
        glUniform1i(glGetUniformLocation(m_ShaderID, name.c_str()), value); 
		unbind();
    }

	int Shader::getUniformLocation(const std::string& name) {
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
			return m_UniformLocationCache[name];
		}

		GLCall(int location = glGetUniformLocation(m_ShaderID, name.c_str()));
		if (location == -1) {
			LEI_ERROR("Uniform does not exist: " + name);
		}
		return location;
	}
}