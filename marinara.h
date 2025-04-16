#ifndef MARINARA_H 
#define MARINARA_H 
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <local/logger.h>

#ifdef MARINARA_FILE
void marinara_toPPM(uint32_t* pixels, size_t width, size_t height, const char* filepath);
#endif // MARINARA FILE
#ifdef MARINARA_SDL
#include <SDL2/SDL.h>
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	int loaded;
} MarinaraSDL;

void marinara_loadSDL(MarinaraSDL* sdl, size_t width, size_t height, const char* title);
void marinara_pixelsToSDL(uint32_t* pixels, size_t width, size_t height, MarinaraSDL* sdl);
void marinara_closeSDL(MarinaraSDL* sdl);
int marinara_getSDLEvents(SDL_Event* e);
#endif // MARINARA_SDL
#ifdef MARINARA_X11
#include <X11/Xlib.h>
typedef struct {
	Display* display;
	Window window;
	GC gc;
	int loaded;
} MarinaraX11;

void marinara_loadX11(MarinaraX11* x11, size_t width, size_t height, const char* title);
void marinara_pixelsToX11(uint32_t* pixels, size_t width, size_t height, MarinaraX11* x11);
int marinara_nextX11Event(XEvent* event, MarinaraX11* x11);
void marinara_closeX11(MarinaraX11* x11);

#endif // MARINARA_X11
#ifdef MARINARA_WAYLAND
#include <wayland-client.h>
#include "libReqs/xdg-shell-client-protocol.h"
#include "libReqs/xdg-shell-protocol.c"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <syscall.h>

typedef struct {
	struct wl_compositor* compositor;
	struct wl_shm* shm;

	struct xdg_wm_base* wmBase;

	struct wl_display* display;
	struct wl_registry* registry;
	struct wl_registry_listener registryListener;

	struct wl_surface* surface;
	struct xdg_surface* xdgSurface;
	struct xdg_toplevel* toplevel;

	unsigned char* data;

	uint32_t* pixels;
	size_t width;
	size_t height;

	int isOpen;
} MarinaraWayland;

void marinara_loadWayland(MarinaraWayland* wayland, uint32_t* pixels, uint32_t width, uint32_t height, const char* title);
struct wl_buffer* marinara_displayWayland(MarinaraWayland* wayland);
int marinara_waylandDisplayDispatch(MarinaraWayland* wayland);
#endif // MARINARA_WAYLAND

#endif // MARINARA_H 
