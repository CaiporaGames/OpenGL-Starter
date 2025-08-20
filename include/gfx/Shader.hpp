#pragma once
#include <glad/glad.h>
#include <string>

class ShaderProgram {
public:
	ShaderProgram() = default;
	~ShaderProgram();

	bool attachFromFile(GLenum type, const char* path);
	bool link();
	void use() const { glUseProgram(program_); }
	GLuint id() const { return program_; }
	GLint uniform(const char* name) const;

private:
	GLuint program_ = 0;
	bool ensureCreated();
	static bool compile(GLenum type, const std::string& src, GLuint& out);
	static bool readFile(const char* path, std::string& out);
	static bool checkShader(GLuint sh, const char* label);
	static bool checkProgram(GLuint prg, const char* label);
};
