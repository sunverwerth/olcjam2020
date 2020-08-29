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

#include "sys.h"
#include <cstdlib>
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <fstream>

static std::ofstream logfile;

int sys_init() {
	logfile.open("codejam.log");
	if (!logfile.good()) {
		sys_crash("Could not open log file.");
		return 1;
	}

	log("Hello, world!");
	log("sys_init");

	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		sys_crash("Could not initialize SDL2.");
		return 1;
	}
	return 0;
}

void sys_shutdown() {
	log("Goodbye, world!");
	SDL_Quit();
}

void sys_crash(const char* reason) {
#ifdef _WIN32
	MessageBoxA(0, reason, "Fatal error", MB_ICONERROR | MB_OK);
#endif
	log_error(reason);
	exit(1);
}

void log(const char* fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	static char buf[512];
	vsprintf_s(buf, fmt, vl);
	logfile << buf << std::endl;
	va_end(vl);
	logfile.flush();
}

void log_error(const char* fmt, ...) {
	va_list vl;
	va_start(vl, fmt);
	static char buf[512];
	vsprintf_s(buf, fmt, vl);
	logfile << "ERROR: " << buf << std::endl;
	va_end(vl);
	logfile.flush();
}

std::string sys_read_file(const char* filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.good()) {
		log_error("Could not open file %s.", filename);
		return "";
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0);
	std::string content(size, ' ');
	file.read(&content[0], size);
	return content;
}