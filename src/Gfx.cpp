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

#include "Gfx.h"
#include "sys.h"
#include "Log.h"
#include "glad.h"

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (type == GL_DEBUG_TYPE_ERROR) log_error(message);
	else log(message);
}

Gfx::Gfx(const char* title, int width, int height, bool fullscreen) : width(width), height(height) {
	log("Gfx::gfx()");
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	if (!window) sys_crash("Could not create SDL window.");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
	unsigned int flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifdef _DEBUG
	flags |= GL_CONTEXT_FLAG_DEBUG_BIT;
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GL_CONTEXT_CORE_PROFILE_BIT);

	context = SDL_GL_CreateContext(window);
	if (!context) sys_crash("Could not create GL context.");

	if (SDL_GL_MakeCurrent(window, context)) sys_crash("Could not make context current.");

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) sys_crash("Could not load GL functions.");

#ifdef _DEBUG
	if (glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debugCallback, nullptr);
	}
#endif
}

Gfx::~Gfx() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}

void Gfx::beginFrame() {
	glViewport(0, 0, width, height);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Gfx::renderFrame() {
	SDL_GL_SwapWindow(window);
}