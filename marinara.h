#ifndef MARINARA_H
#define MARINARA_H
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MARINARA_SUCCESS = 1,
	MARINARA_BAD_DIMS,
	MARINARA_FAILED_WINDOW_CREATE,
	MARINARA_FAILED_GLAD_INIT,
	MARINARA_FAILED_TEXTURE_CREATE,
} MarinaraError;

/*
	 Returns a basic explanation of the given error code
	 NOTE! The returned explanation is not in depth, do not rely on it for specifics 
*/
char* marinara_errorToString(MarinaraError error);

typedef struct {
	GLFWwindow* glfw;
	size_t width, height;

	GLuint texture;
	GLuint shader;
	GLuint VBO, EBO, VAO;
} MarinaraWindow;

/*
	 Fills a MarinaraWindow struct with the given parameters, along with initializing GLFW and GLAD
	 Returns a MarinaraError for the user to do error checking 
*/
MarinaraError marinara_createWindow(MarinaraWindow* window, size_t width, size_t height, const char* title);
void marinara_destroyWindow(MarinaraWindow* window);

int marinara_createTexture(MarinaraWindow* window, uint32_t* pixels);
void marinara_updateTexture(MarinaraWindow* window, uint32_t* pixels);

int marinara_windowIsOpen(MarinaraWindow window);
void marinara_presentWindow(MarinaraWindow window);

static inline void marinara_framebufferSizeCallback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

#ifdef __cplusplus
}
#endif

#endif // MARINARA_H
