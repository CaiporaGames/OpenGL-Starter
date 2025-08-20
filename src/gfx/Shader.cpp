#include "gfx/Shader.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdio>

ShaderProgram::~ShaderProgram() {
	if (program_) glDeleteProgram(program_);
}
bool ShaderProgram::ensureCreated() {
	if (program_ == 0) program_ = glCreateProgram();
	return program_ != 0;
}
bool ShaderProgram::readFile(const char* path, std::string& out) {
	std::ifstream f(path, std::ios::in | std::ios::binary);
	if (!f) { std::fprintf(stderr, "Could not open %s\n", path); return false; }
	std::ostringstream ss; ss << f.rdbuf(); out = ss.str(); return true;
}
bool ShaderProgram::checkShader(GLuint sh, const char* label) {
	GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (ok) return true;
	GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
	std::vector<char> buf(len ? len : 1); glGetShaderInfoLog(sh, len, nullptr, buf.data());
	std::fprintf(stderr, "[SH COMPILE %s]\n%s\n", label, buf.data());
	return false;
}
bool ShaderProgram::checkProgram(GLuint prg, const char* label) {
	GLint ok = 0; glGetProgramiv(prg, GL_LINK_STATUS, &ok);
	if (ok) return true;
	GLint len = 0; glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &len);
	std::vector<char> buf(len ? len : 1); glGetProgramInfoLog(prg, len, nullptr, buf.data());
	std::fprintf(stderr, "[PRG LINK %s]\n%s\n", label, buf.data());
	return false;
}
bool ShaderProgram::compile(GLenum type, const std::string& src, GLuint& out) {
	out = glCreateShader(type);
	const char* p = src.c_str();
	glShaderSource(out, 1, &p, nullptr);
	glCompileShader(out);
	return checkShader(out, type == GL_VERTEX_SHADER ? "VS" : "FS");
}
bool ShaderProgram::attachFromFile(GLenum type, const char* path) {
	if (!ensureCreated()) return false;
	std::string src;
	if (!readFile(path, src)) return false;
	GLuint sh = 0;
	if (!compile(type, src, sh)) { glDeleteShader(sh); return false; }
	glAttachShader(program_, sh);
	glDeleteShader(sh);
	return true;
}
bool ShaderProgram::link() {
	glLinkProgram(program_);
	return checkProgram(program_, "LINK");
}
GLint ShaderProgram::uniform(const char* name) const {
	return glGetUniformLocation(program_, name);
}
