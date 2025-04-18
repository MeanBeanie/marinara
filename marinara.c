#include "include/glad/glad.h"
#include "marinara.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

char* marinara_errorToString(MarinaraError error){
	switch(error){
		case MARINARA_SUCCESS: return "No error :3";
		case MARINARA_BAD_DIMS: return "Window was given at least one dimension that were less than or equal to 0";
		case MARINARA_FAILED_WINDOW_CREATE: return "Failed to create the GLFW window";
		case MARINARA_FAILED_GLAD_INIT: return "Failed to initalize GLAD";
		default: return "Unknown error type";
	};
	return "Somehow skipped the switch statement lmao, this is a bug and is bad";
}

MarinaraError marinara_createWindow(MarinaraWindow* window, size_t width, size_t height, const char* title){
	if(width < 1 || height < 1){ return MARINARA_BAD_DIMS; }
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window->glfw = glfwCreateWindow(width, height, title, NULL, NULL);
	window->width = width;
	window->height = height;
	if(window->glfw == NULL){
		glfwTerminate();
		return MARINARA_FAILED_WINDOW_CREATE;
	}
	glfwMakeContextCurrent(window->glfw);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		return MARINARA_FAILED_GLAD_INIT;
	}
	glViewport(0, 0, 800, 450);
	
	glfwSetFramebufferSizeCallback(window->glfw, marinara_framebufferSizeCallback);

	const char* vertShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"void main()\n{\n"
		"\tgl_Position = vec4(aPos, 1.0);\n"
		"\tTexCoord = aTexCoord;\n"
		"}\0";
	const char* fragShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D aTexture;\n"
		"void main()\n{\n"
		"\tFragColor = texture(aTexture, TexCoord);\n"
		"}\0";

	GLuint vertexShader, fragShader;
	int success;
	char infoLog[512];
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertShaderSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Vert Shader Failure: %s\nSource:\n%s\n--\n", infoLog, vertShaderSource);
	}

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSource, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		printf("Frag Shader Failure: %s\nSource:\n%s\n--\n", infoLog, fragShaderSource);
	}

	window->shader = glCreateProgram();
	glAttachShader(window->shader, vertexShader);
	glAttachShader(window->shader, fragShader);
	glLinkProgram(window->shader);
	glGetProgramiv(window->shader, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(window->shader, 512, NULL, infoLog);
		printf("Shader Failure: %s\n", infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);

	const float verts[] = {
		// x     y    z    tx   ty
		 -1.f, -1.f, 0.f,  0.f, 0.f,
		 -1.f,  1.f, 0.f,  0.f, 1.f,
		  1.f,  1.f, 0.f,  1.f, 1.f,
		  1.f, -1.f, 0.f,  1.f, 0.f,
	};
	const unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &window->VAO);
	glGenBuffers(1, &window->VBO);
	glGenBuffers(1, &window->EBO);
	
	glBindVertexArray(window->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, window->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return MARINARA_SUCCESS;
}
void marinara_destroyWindow(MarinaraWindow* window){
	glDeleteTextures(1, &window->texture);
	glDeleteVertexArrays(1, &window->VAO);
	glDeleteBuffers(1, &window->VBO);
	glDeleteBuffers(1, &window->EBO);
	glfwDestroyWindow(window->glfw);
	window->glfw = NULL;
	glfwTerminate();
}

int marinara_createTexture(MarinaraWindow* window, uint32_t *pixels){
	GLenum error;
	glGenTextures(1, &window->texture);
	glBindTexture(GL_TEXTURE_2D, window->texture);
	error = glGetError();
	if(error == GL_INVALID_VALUE){
		return MARINARA_FAILED_TEXTURE_CREATE;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window->width, window->height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
	error = glGetError();
	if(error != GL_NO_ERROR){
		return MARINARA_BAD_DIMS;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {0.f, 0.f, 0.f, 1.f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);;

	glBindTexture(GL_TEXTURE_2D, 0);

	return MARINARA_SUCCESS;
}
void marinara_updateTexture(MarinaraWindow* window, uint32_t *pixels){
	glBindTexture(GL_TEXTURE_2D, window->texture);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, window->width, window->height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
	GLenum err;

	glBindTexture(GL_TEXTURE_2D, 0);
}

int marinara_windowIsOpen(MarinaraWindow window){
	return !glfwWindowShouldClose(window.glfw);
}
void marinara_presentWindow(MarinaraWindow window){
	glClearColor(0.8f, 0.8f, 0.8f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, window.texture);
	glUseProgram(window.shader);
	glBindVertexArray(window.VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window.EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(window.glfw);
	glfwPollEvents();
}
