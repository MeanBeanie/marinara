#ifndef MARINARA_H
#define MARINARA_H
#include <GLFW/glfw3.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MARINARA_EMPTY_WINDOW (MarinaraWindow){0}

typedef struct {
	GLFWwindow* glfw;
	size_t width, height;

	GLuint texture;
	GLuint shader;
	GLuint VBO, EBO, VAO;
} MarinaraWindow;

/*
	 Creates a MarinaraWindow struct with the given parameters, along with initializing GLFW and GLAD
*/
MarinaraWindow marinara_createWindow(size_t width, size_t height, const char* title);
void marinara_destroyWindow(MarinaraWindow* window);

void marinara_createTexture(MarinaraWindow* window, uint32_t* pixels);
void marinara_updateTexture(MarinaraWindow* window, uint32_t* pixels);

bool marinara_windowIsOpen(MarinaraWindow window);
void marinara_presentWindow(MarinaraWindow window);

static inline void marinara_framebufferSizeCallback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

#ifdef __cplusplus
}
#endif

#endif // MARINARA_H
