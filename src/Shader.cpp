/*
MIT License

Copyright(c) 2020 Stephan Unverwerth

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Shader.h"
#include "sys.h"
#include "glad.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Texture.h"

Shader::Shader(const char* vsfile, const char* fsfile) {
	auto vscode = sys_read_file(vsfile);
	if (vscode.empty()) {
		log_error("No vertex shader code in %s", vsfile);
		return;
	}
	auto vs = glCreateShader(GL_VERTEX_SHADER);
	auto vscodeptr = vscode.c_str();
	int vscodeLength = vscode.size();
	glShaderSource(vs, 1, &vscodeptr, &vscodeLength);
	glCompileShader(vs);
	int vsstatus;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &vsstatus);
	if (vsstatus == GL_FALSE) {
		log_error("Could not compile vertex shader %s", vsfile);
		char infoLog[1024];
		int infoLength;
		glGetShaderInfoLog(vs, 1024, &infoLength, infoLog);
		log_error("%s", infoLog);
		glDeleteShader(vs);
		return;
	}

	auto fscode = sys_read_file(fsfile);
	if (fscode.empty()) {
		log_error("No fragment shader code in %s", fsfile);
		return;
	}
	auto fs = glCreateShader(GL_FRAGMENT_SHADER);
	auto fscodeptr = fscode.c_str();
	int fscodeLength = fscode.size();
	glShaderSource(fs, 1, &fscodeptr, &fscodeLength);
	glCompileShader(fs);
	int fsstatus;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &fsstatus);
	if (fsstatus == GL_FALSE) {
		log_error("Could not compile fragment shader %s", fsfile);
		char infoLog[1024];
		int infoLength;
		glGetShaderInfoLog(fs, 1024, &infoLength, infoLog);
		log_error("%s", infoLog);
		glDeleteShader(fs);
		return;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	int linkstatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkstatus);
	if (linkstatus == GL_FALSE) {
		log_error("Could not link shader program.");
		char infoLog[1024];
		int infoLength;
		glGetProgramInfoLog(vs, 1024, &infoLength, infoLog);
		log_error("%s", infoLog);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(program);
		return;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	int uniformCount;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
	char uniformName[256];
	for (int i = 0; i < uniformCount; i++){
		int length;
		int size;
		GLenum type;
		glGetActiveUniform(program, (GLuint)i, 256, &length, &size, &type, uniformName);
		uniformLocations[uniformName] = i;
	}
}

Shader::~Shader() {
	glDeleteProgram(program);
}

void Shader::use() const {
	glUseProgram(program);
}

int Shader::getUniformLocation(const char* name) const {
	auto it = uniformLocations.find(name);
	if (it == uniformLocations.end()) return -1;
	return it->second;
}

void Shader::uniform(const char* name, float v) {
	auto location = getUniformLocation(name);
	if (location < 0) return;
	glUniform1f(location, v);
}

void Shader::uniform(const char* name, const Vec2& v) {
	auto location = getUniformLocation(name);
	if (location < 0) return;
	glUniform2fv(location, 1, &v.x);
}

void Shader::uniform(const char* name, const Vec3& v) {
	auto location = getUniformLocation(name);
	if (location < 0) return;
	glUniform3fv(location, 1, &v.x);
}

void Shader::uniform(const char* name, const Vec4& v) {
	auto location = getUniformLocation(name);
	if (location < 0) return;
	glUniform4fv(location, 1, &v.x);
}

void Shader::texture(const char* name, const Texture& t) {
	auto location = getUniformLocation(name);
	if (location < 0) return;
	glUniform1i(location, t.unit());
}
