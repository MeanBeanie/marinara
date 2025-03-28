#ifndef MARINARA_H 
#define MARINARA_H 
#include <stdlib.h>
#include <stdint.h>
#include <local/logger.h>

#define MARINARADEF static inline

#ifdef MARINARA_SDL
#include <SDL2/SDL.h>
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	int loaded;
} MarinaraSDL;

MARINARADEF void marinara_loadSDL(MarinaraSDL* sdl, size_t width, size_t height, const char* title);
MARINARADEF void marinara_pixelsToSDL(uint32_t* pixels, size_t width, size_t height, MarinaraSDL* sdl);
MARINARADEF int marinara_getSDLEvents(SDL_Event* e){ return SDL_PollEvent(e); };
MARINARADEF void marinara_closeSDL(MarinaraSDL* sdl);
#endif // MARINARA_SDL
#ifdef MARINARA_X11
#include <X11/Xlib.h>
typedef struct {
	Display* display;
	Window window;
	GC gc;
	int loaded;
} MarinaraX11;

MARINARADEF void marinara_loadX11(MarinaraX11* x11, size_t width, size_t height, const char* title);
MARINARADEF void marinara_pixelsToX11(uint32_t* pixels, size_t width, size_t height, MarinaraX11* x11);
MARINARADEF int marinara_nextX11Event(XEvent* event, MarinaraX11* x11);
MARINARADEF void marinara_closeX11(MarinaraX11* x11);

#endif // MARINARA_X11
#ifdef MARINARA_WAYLAND
#include <wayland-client.h>
#include "libReqs/xdg-shell-client-protocol.h"
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

MARINARADEF void marinara_loadWayland(MarinaraWayland* wayland, uint32_t* pixels, uint32_t width, uint32_t height, const char* title);
MARINARADEF struct wl_buffer* marinara_displayWayland(MarinaraWayland* wayland);
MARINARADEF int marinara_waylandDisplayDispatch(MarinaraWayland* wayland);
#endif // MARINARA_WAYLAND


#endif // MARINARA_H 

#define MARINARA_IMPLEMENTATION
#ifdef MARINARA_IMPLEMENTATION 

#ifdef MARINARA_SDL
MARINARADEF void marinara_loadSDL(MarinaraSDL *sdl, size_t width, size_t height, const char *title){
	sdl->window = NULL;
	sdl->renderer = NULL;
	sdl->texture = NULL;
	sdl->loaded = 0;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		ego_logger("Marinara SDL ERROR", "Failed to load SDL with SDL_Error: %s", SDL_GetError());
		exit(1);
	}

	sdl->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	if(sdl->window == NULL){
		ego_logger("Marinara SDL ERROR", "Failed to create SDL window with error: %s", SDL_GetError());
		exit(1);
	}

	sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
	if(sdl->renderer == NULL){
		ego_logger("Marinara SDL ERROR", "Failed to create renderer SDL with SDL_Error: %s", SDL_GetError());
		exit(1);
	}

	sdl->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	sdl->loaded = 1;
}

MARINARADEF void marinara_pixelsToSDL(uint32_t* pixels, size_t width, size_t height, MarinaraSDL* sdl){
	if(sdl->loaded == 0){ return; } 
	SDL_UpdateTexture(sdl->texture, NULL, pixels, width * sizeof(uint32_t));
	SDL_RenderClear(sdl->renderer);
	SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, NULL);
	SDL_RenderPresent(sdl->renderer);
}

MARINARADEF void marinara_closeSDL(MarinaraSDL* sdl){
	if(sdl->loaded == 0){ return; } 
	SDL_DestroyTexture(sdl->texture);
	SDL_DestroyRenderer(sdl->renderer);
	SDL_DestroyWindow(sdl->window);
	SDL_Quit();
}
#endif // MARINARA_SDL
#ifdef MARINARA_X11
MARINARADEF void marinara_loadX11(MarinaraX11* x11, size_t width, size_t height, const char* title){
	x11->loaded = 0;
	x11->display = XOpenDisplay(NULL);
	if(x11->display){
		ego_logger("Marinara X11 ERROR", "Failed to open display at $DISPLAY");
		exit(1);
	}
	int screen = DefaultScreen(x11->display);
	Window root = DefaultRootWindow(x11->display);

	x11->window = XCreateSimpleWindow(x11->display, root, 0, 0, width, height, 1, BlackPixel(x11->display, screen), WhitePixel(x11->display, screen));
	XMapWindow(x11->display, x11->window);
	XStoreName(x11->display, x11->window, title);
	x11->gc = XCreateGC(x11->display, root, 0, NULL);

	XSelectInput(x11->display, x11->window, ExposureMask);
	x11->loaded = 1;
}
MARINARADEF void marinara_pixelsToX11(uint32_t* pixels, size_t width, size_t height, MarinaraX11* x11){
	if(x11->loaded == 0){ return; }
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			uint32_t pixel = pixels[y*width + x];
			int color = (pixel>>24) || (pixel>>16) | (pixel>>8);
			XSetForeground(x11->display, x11->gc, color);
			XDrawPoint(x11->display, x11->window, x11->gc, x, y);
		}
	}
}
MARINARADEF int marinara_nextX11Event(XEvent* event, MarinaraX11* x11){
	return XNextEvent(x11->display, event);
}
MARINARADEF void marinara_closeX11(MarinaraX11* x11){
	if(x11->loaded == 0){ return; }
	XFreeGC(x11->display, x11->gc);
	XDestroyWindow(x11->display, x11->window);
	XCloseDisplay(x11->display);
}
#endif // MARINARA_X11
#ifdef MARINARA_WAYLAND
static void releaseBuffer(void* data, struct wl_buffer* wl_buffer){
	wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener bufferListener = {
	.release = releaseBuffer,
};

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial){
	MarinaraWayland* wayland = (MarinaraWayland*)data;
	xdg_surface_ack_configure(xdg_surface, serial);

	struct wl_buffer* buffer = marinara_displayWayland(wayland);
	wl_surface_attach(wayland->surface, buffer, 0, 0);
	wl_surface_commit(wayland->surface);
}

static const struct xdg_surface_listener xdgSurfaceListener = {
	.configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial){
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdgWmBaseListener = {
	.ping = xdg_wm_base_ping,
};

static void registryGlobalHandler(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version){
	MarinaraWayland* wayland = (MarinaraWayland*)data;

	if(strcmp(interface, "wl_compositor") == 0){
		wayland->compositor = (struct wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	}
	else if(strcmp(interface, "wl_shm") == 0){
		wayland->shm = (struct wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if(strcmp(interface, xdg_wm_base_interface.name) == 0){
		wayland->wmBase = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wayland->wmBase, &xdgWmBaseListener, wayland);
	}
}
static void registryGlobalRemoveHandler(void* data, struct wl_registry* registry, uint32_t name){}

MARINARADEF void marinara_loadWayland(MarinaraWayland* wayland, uint32_t* pixels, uint32_t width, uint32_t height, const char* title){
	wayland->isOpen = 1;
	wayland->pixels = pixels;
	wayland->width = width;
	wayland->height = height;
	wayland->display = wl_display_connect(NULL);
	if(!wayland->display){
		ego_logger("Marinara Wayland ERROR", "Failed to connect display");
		exit(1);
	}
	wayland->registry = wl_display_get_registry(wayland->display);
 
	wayland->registryListener.global = registryGlobalHandler;
	wayland->registryListener.global_remove = registryGlobalRemoveHandler;
	wl_registry_add_listener(wayland->registry, &wayland->registryListener, wayland);

	wl_display_roundtrip(wayland->display);

	wayland->surface = wl_compositor_create_surface(wayland->compositor);
	wayland->xdgSurface = xdg_wm_base_get_xdg_surface(wayland->wmBase, wayland->surface);
	xdg_surface_add_listener(wayland->xdgSurface, &xdgSurfaceListener, wayland);
	wayland->toplevel = xdg_surface_get_toplevel(wayland->xdgSurface);
	xdg_toplevel_set_title(wayland->toplevel, title);
	wl_surface_commit(wayland->surface);
}

MARINARADEF struct wl_buffer* marinara_displayWayland(MarinaraWayland* wayland){
	int stride = wayland->width * 4;
	int size = stride * wayland->height;

	int fd = syscall(SYS_memfd_create, "buffer", 0);
	ftruncate(fd, size);

	wayland->data = (unsigned char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(wayland->data == MAP_FAILED){
		close(fd);
		ego_logger("Marinara Wayland ERROR", "Failed to map data");
		exit(1);
	}
	struct wl_shm_pool* shmPool = wl_shm_create_pool(wayland->shm, fd, size);

	struct wl_buffer* buffer = wl_shm_pool_create_buffer(shmPool, 0, wayland->width, wayland->height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(shmPool);
	close(fd);

	for(int y = 0; y < wayland->height; y++){
		for(int x = 0; x < wayland->width; x++){
			struct pixel {
				uint8_t blue;
				uint8_t red;
				uint8_t green;
				uint8_t alpha;
			} *px = (struct pixel*)(wayland->data + y * stride + x * 4);
			uint32_t canvasPixel = wayland->pixels[y*wayland->width + x];
			px->red = (canvasPixel>>24)&0xFF;
			px->green = (canvasPixel>>16)&0xFF;
			px->blue = (canvasPixel>>8)&0xFF;
			px->alpha = canvasPixel&0xFF;
		}
	}

	munmap(wayland->data, size);
	wl_buffer_add_listener(buffer, &bufferListener, NULL);

	return buffer;
}

int marinara_waylandDisplayDispatch(MarinaraWayland* wayland){
	return wl_display_dispatch(wayland->display);
}
#endif // MARINARA_WAYLAND

#endif // FETTUCCINI_IMPLEMENTATION
