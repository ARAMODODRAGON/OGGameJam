#include "Shader.hpp"
#include <unordered_map>
#include <glew.h>

static uint32 GetShaderTypeFromString(const string& shadertype) {
	if (shadertype == "vertex") return GL_VERTEX_SHADER;
	if (shadertype == "fragment") return GL_FRAGMENT_SHADER;
	return -1;
}

uint32 LoadShaderSource(const string& source) { 
	// via "the cherno" https://youtu.be/8wFEzIYRZXg?t=1221

	std::unordered_map<uint32, string> sources;

	// read the file for each shader
	static const string typeToken = "#type";
	static const size_t typeTokenLen = typeToken.size();
	size_t pos = source.find(typeToken, 0);
	while (pos != string::npos) {
		size_t eol = source.find_first_of("\r\n", pos);
		if (eol == string::npos) {
			OGJ_DEBUG_ERROR("Syntax error");
			return -1;
		}
		size_t begin = pos + typeTokenLen + 1;
		uint32 shaderType = GetShaderTypeFromString(source.substr(begin, eol - begin));
		if (shaderType == -1) {
			pos = source.find(typeToken, eol);
			continue;
		}
		size_t nextLinePos = source.find_first_not_of("\r\n", eol);
		pos = source.find(typeToken, nextLinePos + 2);
		size_t endPos = pos - (nextLinePos == string::npos ? source.size() - 1 : nextLinePos);
		sources.emplace(shaderType, source.substr(nextLinePos, endPos));
	}

	// end 'the cherno'

	if (sources.size() == 0) {
		OGJ_DEBUG_ERROR("Invalid shader source");
		return -1;
	}

	vector<uint32> shaders;
	uint32 shaderProgram = glCreateProgram();

	for (auto& [type_, source_] : sources) {
		// create and load the shader
		uint32 shaderID = glCreateShader(type_);
		const char* csource = source_.c_str();
		glShaderSource(shaderID, 1, &csource, 0);
		glCompileShader(shaderID);

		// check for errors
		int32 success;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			// get error and print
			char infoLog[512];
			glGetShaderInfoLog(shaderID, 512, 0, infoLog);
			OGJ_DEBUG_ERROR("Failed to compile shader: " + string(infoLog));
			OGJ_DEBUG_LOG(source_);

			// delete the shader and return
			glDeleteShader(shaderID);
			continue;
		}

		glAttachShader(shaderProgram, shaderID);
		shaders.push_back(shaderID);
	}

	// no shaders were attached
	if (shaders.size() == 0) {
		OGJ_DEBUG_ERROR("Could not load any shaders, deleted shader program");
		glDeleteProgram(shaderProgram);
		return -1;
	}

	// link
	glLinkProgram(shaderProgram);

	// check linking errors
	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		// get error message and print
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, 0, infoLog);
		OGJ_DEBUG_ERROR("Failed to link program: \n" + string(infoLog));
		// delete shaders/program
		for (GLuint id : shaders) glDeleteShader(id);
		glDeleteProgram(shaderProgram);
		return -1;
	}

	// delete shaders
	for (GLuint id : shaders) glDeleteShader(id);
	shaders.clear();

	// return shader ID
	return shaderProgram;
}
