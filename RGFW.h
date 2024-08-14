/*
* Copyright (C) 2023-24 ColleagueRiley
*
* libpng license
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/

/*
	(MAKE SURE RGFW_IMPLEMENTATION is in exactly one header or you use -D RGFW_IMPLEMENTATION)
	#define RGFW_IMPLEMENTATION - makes it so source code is included with header
*/

/*
	#define RGFW_IMPLEMENTATION - (required) makes it so the source code is included
	#define RGFW_PRINT_ERRORS - (optional) makes it so RGFW prints errors when they're found
	#define RGFW_OSMESA - (optional) use OSmesa as backend (instead of system's opengl api + regular opengl)
	#define RGFW_BUFFER - (optional) just draw directly to (RGFW) window pixel buffer that is drawn to screen (the buffer is in the RGBA format)
	#define RGFW_EGL - (optional) use EGL for loading an OpenGL context (instead of the system's opengl api)
	#define RGFW_OPENGL_ES1 - (optional) use EGL to load and use Opengl ES (version 1) for backend rendering (instead of the system's opengl api)
									This version doesn't work for desktops (I'm pretty sure)
	#define RGFW_OPENGL_ES2 - (optional) use OpenGL ES (version 2)
	#define RGFW_OPENGL_ES3 - (optional) use OpenGL ES (version 3)
	#define RGFW_DIRECTX - (optional) use directX for the rendering backend (rather than opengl) (windows only, defaults to opengl for unix)
	#define RGFW_NO_API - (optional) don't use any rendering API (no opengl, no vulkan, no directX)

	#define RGFW_LINK_EGL (optional) (windows only) if EGL is being used, if EGL functions should be defined dymanically (using GetProcAddress)
	#define RGFW_LINK_OSMESA (optional) (windows only) if EGL is being used, if OS Mesa functions should be defined dymanically  (using GetProcAddress)

	#define RGFW_X11 (optional) (unix only) if X11 should be used. This option is turned on by default by unix systems except for MacOS
	#define RGFW_WGL_LOAD (optional) (windows only) if WGL should be loaded dynamically during runtime
	#define RGFW_NO_X11_CURSOR (optional) (unix only) don't use XCursor
	#define RGFW_NO_X11_CURSOR_PRELOAD (optional) (unix only) Use XCursor, but don't link it in code, (you'll have to link it with -lXcursor)

	#define RGFW_NO_DPI - Do not include calculate DPI (no XRM nor libShcore included)

	#define RGFW_ALLOC_DROPFILES (optional) if room should be allocating for drop files (by default it's global data)
	#define RGFW_MALLOC x - choose what function to use to allocate, by default the standard malloc is used
	#define RGFW_CALLOC x - choose what function to use to allocate (calloc), by default the standard calloc is used
	#define RGFW_FREE x - choose what function to use to allocated memory, by default the standard free is used

 	#define RGFW_EXPORT - Use when building RGFW 
    #define RGFW_IMPORT - Use when linking with RGFW (not as a single-header)
	
	#define RGFW_STD_INT - force the use stdint.h (for systems that might not have stdint.h (msvc)) 
*/

/*
	RGFW embedded exclusive args:

	#define RGFW_DRM - use libdrm backend
*/

/*
	Credits :
		EimaMei/Sacode : Much of the code for creating windows using winapi, Wrote the Silicon library, helped with MacOS Support, siliapp.h -> referencing 

		stb - This project is heavily inspired by the stb single header files

		GLFW:
			certain parts of winapi and X11 are very poorly documented,
			GLFW's source code was referenced and used throughout the project (used code is marked in some way),
			this mainly includes, code for drag and drops, code for setting the icon to a bitmap and the code for managing the clipboard for X11 (as these parts are not documented very well)

			GLFW Copyright, https::/github.com/GLFW/GLFW

			Copyright (c) 2002-2006 Marcus Geelnard
			Copyright (c) 2006-2019 Camilla Löwy

		contributors : (feel free to put yourself here if you contribute)
		krisvers -> code review
		EimaMei (SaCode) -> code review
		Code-Nycticebus -> bug fixes
		Rob Rohan -> X11 bugs and missing features, MacOS/Cocoa fixing memory issues/bugs 
		AICDG (@THISISAGOODNAME) -> vulkan support (example)
		@Easymode -> support, testing/debugging, bug fixes and reviews
*/

/* if no platform is defined */
#if !defined(RGFW_DRM) 
#error no platform defined
#endif

#ifndef RGFW_MALLOC
	#include <stdlib.h>

	#ifndef __USE_POSIX199309
	#define __USE_POSIX199309
	#endif

	#include <time.h>
	#define RGFW_MALLOC malloc
	#define RGFW_CALLOC calloc
	#define RGFW_FREE free
#endif

#if !_MSC_VER
	#ifndef inline
		#ifndef __APPLE__
			#define inline __inline
		#endif
	#endif
#endif

#if defined(RGFW_EXPORT) ||  defined(RGFW_IMPORT)
	#if defined(_WIN32)
		#if defined(__TINYC__) && (defined(RGFW_EXPORT) ||  defined(RGFW_IMPORT))
			#define __declspec(x) __attribute__((x))
		#endif

		#if defined(RGFW_EXPORT)
			#define RGFWDEF __declspec(dllexport)
		#else 
			#define RGFWDEF __declspec(dllimport)
		#endif
	#else
		#if defined(RGFW_EXPORT)
			#define RGFWDEF __attribute__((visibility("default")))
		#endif
	#endif
#endif 

#ifndef RGFWDEF
	#ifdef __clang__
		#define RGFWDEF static inline
	#else
		#define RGFWDEF inline
	#endif
#endif

#ifndef RGFW_ENUM
	#define RGFW_ENUM(type, name) type name; enum
#endif

#ifndef RGFW_UNUSED
	#define RGFW_UNUSED(x) (void)(x);
#endif

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
	extern "C" {
#endif

	/* makes sure the header file part is only defined once by default */
#ifndef RGFW_HEADER

#define RGFW_HEADER

#if !defined(u8)
	#if ((defined(_MSC_VER) || defined(__SYMBIAN32__)) && !defined(RGFW_STD_INT)) /* MSVC might not have stdint.h */
		typedef unsigned char 	u8;
		typedef signed char		i8;
		typedef unsigned short  u16;
		typedef signed short 	i16;
		typedef unsigned int 	u32;
		typedef signed int		i32;
		typedef unsigned long	u64;
		typedef signed long		i64;
	#else /* use stdint standard types instead of c ""standard"" types */
		#include <stdint.h>

		typedef uint8_t     u8;
		typedef int8_t      i8;
		typedef uint16_t   u16;
		typedef int16_t    i16;
		typedef uint32_t   u32;
		typedef int32_t    i32;
		typedef uint64_t   u64;
		typedef int64_t    i64;
	#endif
#endif

#if !defined(b8) /* RGFW bool type */
	typedef u8 b8;
	typedef u32 b32;
#endif

#define RGFW_TRUE 1
#define RGFW_FALSE 0

#if (defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)) && !defined(RGFW_EGL)
	#define RGFW_EGL
#endif

#if !defined(RGFW_OSMESA) && !defined(RGFW_EGL) && !defined(RGFW_OPENGL) && !defined(RGFW_DIRECTX) && !defined(RGFW_BUFFER) && !defined(RGFW_NO_API)
	#define RGFW_OPENGL
#endif

#ifdef RGFW_EGL
	#if defined(__APPLE__)
		#warning  EGL is not supported for Cocoa, switching back to the native opengl api
		#undef RGFW_EGL
	#endif

	#include <EGL/egl.h>
#elif defined(RGFW_OSMESA)
	#ifndef __APPLE__
		#include <GL/osmesa.h>
	#else
		#include <OpenGL/osmesa.h>
	#endif
#endif

#ifndef RGFW_ALPHA
	#define RGFW_ALPHA 128 /* alpha value for RGFW_TRANSPARENT_WINDOW (WINAPI ONLY, macOS + linux don't need this) */
#endif

/*! Optional arguments for making a windows */
#define RGFW_TRANSPARENT_WINDOW		(1L<<9) /*!< the window is transparent (only properly works on X11 and MacOS, although it's although for windows) */
#define RGFW_NO_BORDER		(1L<<3) /*!< the window doesn't have border */
#define RGFW_NO_RESIZE		(1L<<4) /*!< the window cannot be resized  by the user */
#define RGFW_ALLOW_DND     (1L<<5) /*!< the window supports drag and drop*/
#define RGFW_HIDE_MOUSE (1L<<6) /*! the window should hide the mouse or not (can be toggled later on) using `RGFW_window_mouseShow*/
#define RGFW_FULLSCREEN (1L<<8) /* the window is fullscreen by default or not */
#define RGFW_CENTER (1L<<10) /*! center the window on the screen */
#define RGFW_OPENGL_SOFTWARE (1L<<11) /*! use OpenGL software rendering */
#define RGFW_COCOA_MOVE_TO_RESOURCE_DIR (1L << 12) /* (cocoa only), move to resource folder */
#define RGFW_SCALE_TO_MONITOR (1L << 13) /* scale the window to the screen */
#define RGFW_NO_INIT_API (1L << 2) /* DO not init an API (mostly for bindings, you should use `#define RGFW_NO_API` in C */

#define RGFW_NO_GPU_RENDER (1L<<14) /* don't render (using the GPU based API)*/
#define RGFW_NO_CPU_RENDER (1L<<15) /* don't render (using the CPU based buffer rendering)*/
#define RGFW_WINDOW_HIDE (1L <<  16)/* the window is hidden */

typedef RGFW_ENUM(u8, RGFW_event_types) {
	/*! event codes */
 	RGFW_keyPressed = 1, /* a key has been pressed */
	RGFW_keyReleased, /*!< a key has been released*/
	/*! key event note
		the code of the key pressed is stored in
		RGFW_Event.keyCode
		!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!!

		while a string version is stored in
		RGFW_Event.KeyString

		RGFW_Event.lockState holds the current lockState
		this means if CapsLock, NumLock are active or not
	*/
	RGFW_mouseButtonPressed, /*!< a mouse button has been pressed (left,middle,right)*/
	RGFW_mouseButtonReleased, /*!< a mouse button has been released (left,middle,right)*/
	RGFW_mousePosChanged, /*!< the position of the mouse has been changed*/
	/*! mouse event note
		the x and y of the mouse can be found in the vector, RGFW_Event.point

		RGFW_Event.button holds which mouse button was pressed
	*/
	RGFW_jsButtonPressed, /*!< a joystick button was pressed */
	RGFW_jsButtonReleased, /*!< a joystick button was released */
	RGFW_jsAxisMove, /*!< an axis of a joystick was moved*/
	/*! joystick event note
		RGFW_Event.joystick holds which joystick was altered, if any
		RGFW_Event.button holds which joystick button was pressed

		RGFW_Event.axis holds the data of all the axis
		RGFW_Event.axisCount says how many axis there are
	*/
	RGFW_windowMoved, /*!< the window was moved (by the user) */
	RGFW_windowResized, /*!< the window was resized (by the user), [on webASM this means the browser was resized] */
	RGFW_focusIn, /*!< window is in focus now */
	RGFW_focusOut, /*!< window is out of focus now */
	RGFW_mouseEnter, /* mouse entered the window */
	RGFW_mouseLeave, /* mouse left the window */
	RGFW_windowRefresh, /* The window content needs to be refreshed */

	/* attribs change event note
		The event data is sent straight to the window structure
		with win->r.x, win->r.y, win->r.w and win->r.h
	*/
	RGFW_quit, /*!< the user clicked the quit button*/ 
	RGFW_dnd, /*!< a file has been dropped into the window*/
	RGFW_dnd_init /*!< the start of a dnd event, when the place where the file drop is known */
	/* dnd data note
		The x and y coords of the drop are stored in the vector RGFW_Event.point

		RGFW_Event.droppedFilesCount holds how many files were dropped

		This is also the size of the array which stores all the dropped file string,
		RGFW_Event.droppedFiles
	*/
};

/*! mouse button codes (RGFW_Event.button) */
#define RGFW_mouseLeft  1 /*!< left mouse button is pressed*/
#define RGFW_mouseMiddle  2 /*!< mouse-wheel-button is pressed*/
#define RGFW_mouseRight  3 /*!< right mouse button is pressed*/
#define RGFW_mouseScrollUp  4 /*!< mouse wheel is scrolling up*/
#define RGFW_mouseScrollDown  5 /*!< mouse wheel is scrolling down*/

#ifndef RGFW_MAX_PATH
#define RGFW_MAX_PATH 260 /* max length of a path (for dnd) */
#endif
#ifndef RGFW_MAX_DROPS
#define RGFW_MAX_DROPS 260 /* max items you can drop at once */
#endif


/* for RGFW_Event.lockstate */
#define RGFW_CAPSLOCK (1L << 1)
#define RGFW_NUMLOCK (1L << 2)

/*! joystick button codes (based on xbox/playstation), you may need to change these values per controller */
#ifndef RGFW_joystick_codes
	typedef RGFW_ENUM(u8, RGFW_joystick_codes) {
		RGFW_JS_A = 0, /*!< or PS X button */
		RGFW_JS_B = 1, /*!< or PS circle button */
		RGFW_JS_Y = 2, /*!< or PS triangle button */
		RGFW_JS_X = 3, /*!< or PS square button */
		RGFW_JS_START = 9, /*!< start button */
		RGFW_JS_SELECT = 8, /*!< select button */
		RGFW_JS_HOME = 10, /*!< home button */
		RGFW_JS_UP = 13, /*!< dpad up */
		RGFW_JS_DOWN = 14, /*!< dpad down*/
		RGFW_JS_LEFT = 15, /*!< dpad left */
		RGFW_JS_RIGHT = 16, /*!< dpad right */
		RGFW_JS_L1 = 4, /*!< left bump */
		RGFW_JS_L2 = 5, /*!< left trigger*/
		RGFW_JS_R1 = 6, /*!< right bumper */
		RGFW_JS_R2 = 7, /*!< right trigger */
	};
#endif

/*! basic vector type, if there's not already a point/vector type of choice */
#ifndef RGFW_point
	typedef struct { i32 x, y; } RGFW_point;
#endif

/*! basic rect type, if there's not already a rect type of choice */
#ifndef RGFW_rect
	typedef struct { i32 x, y, w, h; } RGFW_rect;
#endif

/*! basic area type, if there's not already a area type of choice */
#ifndef RGFW_area
	typedef struct { u32 w, h; } RGFW_area;
#endif

#ifndef __cplusplus
#define RGFW_POINT(x, y) (RGFW_point){(i32)(x), (i32)(y)}
#define RGFW_RECT(x, y, w, h) (RGFW_rect){(i32)(x), (i32)(y), (i32)(w), (i32)(h)}
#define RGFW_AREA(w, h) (RGFW_area){(u32)(w), (u32)(h)}
#else
#define RGFW_POINT(x, y) {(i32)(x), (i32)(y)}
#define RGFW_RECT(x, y, w, h) {(i32)(x), (i32)(y), (i32)(w), (i32)(h)}
#define RGFW_AREA(w, h) {(u32)(w), (u32)(h)}
#endif

#ifndef RGFW_NO_MONITOR
	/*! structure for monitor data */
	typedef struct RGFW_monitor {
		char name[128]; /*!< monitor name */
		RGFW_rect rect; /*!< monitor Workarea */
		float scaleX, scaleY; /*!< monitor content scale*/
		float physW, physH; /*!< monitor physical size */
	} RGFW_monitor;

	/*
		NOTE : Monitor functions should be ran only as many times as needed (not in a loop)
	*/

	/*! get an array of all the monitors (max 6) */
	RGFWDEF RGFW_monitor* RGFW_getMonitors(void);
	/*! get the primary monitor */
	RGFWDEF RGFW_monitor RGFW_getPrimaryMonitor(void);
#endif

/* NOTE: some parts of the data can represent different things based on the event (read comments in RGFW_Event struct) */
/*! Event structure for checking/getting events */
typedef struct RGFW_Event {
	char keyName[16]; /*!< key name of event*/

	/*! drag and drop data */
	/* 260 max paths with a max length of 260 */
#ifdef RGFW_ALLOC_DROPFILES
	char** droppedFiles;
#else
	char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH]; /*!< dropped files*/
#endif
	u32 droppedFilesCount; /*!< house many files were dropped */

	u32 type; /*!< which event has been sent?*/
	RGFW_point point; /*!< mouse x, y of event (or drop point) */
	
	u8 keyCode; /*!< keycode of event 	!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!! */	
	
	b8 repeat; /*!< key press event repeated (the key is being held) */
	b8 inFocus;  /*!< if the window is in focus or not (this is always true for MacOS windows due to the api being weird) */

	u8 lockState;
	
	u8 button; /* !< which mouse button was pressed */
	double scroll; /*!< the raw mouse scroll value */

	u16 joystick; /*! which joystick this event applies to (if applicable to any) */
	u8 axisesCount; /*!< number of axises */
	RGFW_point axis[2]; /*!< x, y of axises (-100 to 100) */

	u64 frameTime, frameTime2; /*!< this is used for counting the fps */
} RGFW_Event;

/*! source data for the window (used by the APIs) */
typedef struct RGFW_window_src {
	#if defined(RGFW_EGL)
		EGLSurface EGL_surface;
		EGLDisplay EGL_display;
		EGLContext EGL_context;
	#endif
} RGFW_window_src;



typedef struct RGFW_window {
	RGFW_window_src src; /*!< src window data */

#if defined(RGFW_OSMESA) || defined(RGFW_BUFFER) 
	u8* buffer; /*!< buffer for non-GPU systems (OSMesa, basic software rendering) */
	/* when rendering using RGFW_BUFFER, the buffer is in the RGBA format */
#endif
	void* userPtr; /* ptr for usr data */
	
	RGFW_Event event; /*!< current event */

	RGFW_rect r; /*!< the x, y, w and h of the struct */
	
	RGFW_point _lastMousePoint; /*!< last cusor point (for raw mouse data) */
	
	u32 _winArgs; /*!< windows args (for RGFW to check) */
} RGFW_window; /*!< Window structure for managing the window */

#if defined(RGFW_DRM) 
	typedef u64 RGFW_thread; /*!< thread type unix */
#else
	typedef void* RGFW_thread; /*!< thread type for window */
#endif

/** * @defgroup Window_management
* @{ */ 

/*! this has to be set before createWindow is called, else the fulscreen size is used */
RGFWDEF void RGFW_setBufferSize(RGFW_area size); /*!< the buffer cannot be resized (by RGFW) */

RGFW_window* RGFW_createWindow(
	const char* name, /* name of the window */
	RGFW_rect rect, /* rect of window */
	u16 args /* extra arguments (NULL / (u16)0 means no args used)*/
); /*!< function to create a window struct */

/*! get the size of the screen to an area struct */
RGFWDEF RGFW_area RGFW_getScreenSize(void);

/*!
	this function checks an *individual* event (and updates window structure attributes)
	this means, using this function without a while loop may cause event lag

	ex.

	while (RGFW_window_checkEvent(win) != NULL) [this keeps checking events until it reaches the last one]

	this function is optional if you choose to use event callbacks, 
	although you still need some way to tell RGFW to process events eg. `RGFW_window_checkEvents`
*/

RGFW_Event* RGFW_window_checkEvent(RGFW_window* win); /*!< check current event (returns a pointer to win->event or NULL if there is no event)*/

/*!
	for RGFW_window_eventWait and RGFW_window_checkEvents
	waitMS -> Allows th	e function to keep checking for events even after `RGFW_window_checkEvent == NULL`
			  if waitMS == 0, the loop will not wait for events
			  if waitMS == a positive integer, the loop will wait that many miliseconds after there are no more events until it returns
			  if waitMS == a negative integer, the loop will not return until it gets another event
*/
typedef RGFW_ENUM(i32, RGFW_eventWait) {
	RGFW_NEXT = -1,
	RGFW_NO_WAIT = 0
};

/*! sleep until RGFW gets an event or the timer ends (defined by OS) */
RGFWDEF void RGFW_window_eventWait(RGFW_window* win, i32 waitMS);

/*!
	check all the events until there are none left, 
	this should only be used if you're using callbacks only
*/
RGFWDEF void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS);

/*! 
	Tell RGFW_window_eventWait to stop waiting, to be ran from another thread
*/
RGFWDEF void RGFW_stopCheckEvents(void);

/*! window managment functions*/
RGFWDEF void RGFW_window_close(RGFW_window* win); /*!< close the window and free leftover data */

/*! moves window to a given point */
RGFWDEF void RGFW_window_move(RGFW_window* win,
	RGFW_point v/*!< new pos*/
);

#ifndef RGFW_NO_MONITOR
	/*! move to a specific monitor */
	RGFWDEF void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m /* monitor */);
#endif

/*! resize window to a current size/area */
RGFWDEF void RGFW_window_resize(RGFW_window* win, /*!< source window */
	RGFW_area a/*!< new size*/
);

/*! set the minimum size a user can shrink a window to a given size/area */
RGFWDEF void RGFW_window_setMinSize(RGFW_window* win, RGFW_area a);
/*! set the minimum size a user can extend a window to a given size/area */
RGFWDEF void RGFW_window_setMaxSize(RGFW_window* win, RGFW_area a);

RGFWDEF void RGFW_window_maximize(RGFW_window* win); /*!< maximize the window size */
RGFWDEF void RGFW_window_minimize(RGFW_window* win); /*!< minimize the window (in taskbar (per OS))*/
RGFWDEF void RGFW_window_restore(RGFW_window* win); /*!< restore the window from minimized (per OS)*/

/*! if the window should have a border or not (borderless) based on bool value of `border` */
RGFWDEF void RGFW_window_setBorder(RGFW_window* win, b8 border);

/*! turn on / off dnd (RGFW_ALLOW_DND stil must be passed to the window)*/
RGFWDEF void RGFW_window_setDND(RGFW_window* win, b8 allow);

#ifndef RGFW_NO_PASSTHROUGH
	/*!! turn on / off mouse passthrough */
	RGFWDEF void RGFW_window_setMousePassthrough(RGFW_window* win, b8 passthrough);
#endif 

/*! rename window to a given string */
RGFWDEF void RGFW_window_setName(RGFW_window* win,
	char* name
);

void RGFW_window_setIcon(RGFW_window* win, /*!< source window */
	u8* icon /*!< icon bitmap */,
	RGFW_area a /*!< width and height of the bitmap*/,
	i32 channels /*!< how many channels the bitmap has (rgb : 3, rgba : 4) */
); /*!< image resized by default */

/*!< sets mouse to bitmap (very simular to RGFW_window_setIcon), image NOT resized by default*/
RGFWDEF void RGFW_window_setMouse(RGFW_window* win, u8* image, RGFW_area a, i32 channels);

/*!< sets the mouse to a standard API cursor (based on RGFW_MOUSE, as seen at the end of the RGFW_HEADER part of this file) */
RGFWDEF	void RGFW_window_setMouseStandard(RGFW_window* win, u8 mouse);

RGFWDEF void RGFW_window_setMouseDefault(RGFW_window* win); /*!< sets the mouse to the default mouse icon */
/*
	Locks cursor at the center of the window
	win->event.point become raw mouse movement data 

	this is useful for a 3D camera
*/
RGFWDEF void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area);
/*! stop holding the mouse and let it move freely */
RGFWDEF void RGFW_window_mouseUnhold(RGFW_window* win);

/*! hide the window */
RGFWDEF void RGFW_window_hide(RGFW_window* win);
/*! show the window */
RGFWDEF void RGFW_window_show(RGFW_window* win);

/*
	makes it so `RGFW_window_shouldClose` returns true
	by setting the window event.type to RGFW_quit
*/
RGFWDEF void RGFW_window_setShouldClose(RGFW_window* win);

/*! where the mouse is on the screen */
RGFWDEF RGFW_point RGFW_getGlobalMousePoint(void);

/*! where the mouse is on the window */
RGFWDEF RGFW_point RGFW_window_getMousePoint(RGFW_window* win);

/*! show the mouse or hide the mouse*/
RGFWDEF void RGFW_window_showMouse(RGFW_window* win, i8 show);
/*! move the mouse to a set x, y pos*/
RGFWDEF void RGFW_window_moveMouse(RGFW_window* win, RGFW_point v);

/*! if the window should close (RGFW_close was sent or escape was pressed) */
RGFWDEF b8 RGFW_window_shouldClose(RGFW_window* win);
/*! if window is fullscreen'd */
RGFWDEF b8 RGFW_window_isFullscreen(RGFW_window* win);
/*! if window is hidden */
RGFWDEF b8 RGFW_window_isHidden(RGFW_window* win);
/*! if window is minimized */
RGFWDEF b8 RGFW_window_isMinimized(RGFW_window* win);
/*! if window is maximized */
RGFWDEF b8 RGFW_window_isMaximized(RGFW_window* win);

/** @} */ 

/** * @defgroup Monitor
* @{ */ 

#ifndef RGFW_NO_MONITOR
/*
scale the window to the monitor,
this is run by default if the user uses the arg `RGFW_SCALE_TO_MONITOR` during window creation
*/
RGFWDEF void RGFW_window_scaleToMonitor(RGFW_window* win);
/*! get the struct of the window's monitor  */
RGFWDEF RGFW_monitor RGFW_window_getMonitor(RGFW_window* win);
#endif

/** @} */ 

/** * @defgroup Input
* @{ */ 

/*error handling*/
RGFWDEF b8 RGFW_Error(void); /*!< returns true if an error has occurred (doesn't print errors itself) */

/*! returns true if the key should be shifted */
RGFWDEF b8 RGFW_shouldShift(u32 keycode, u8 lockState);

/*! get char from RGFW keycode (using a LUT), uses shift'd version if shift = true */
RGFWDEF char RGFW_keyCodeToChar(u32 keycode, b8 shift);
/*! get char from RGFW keycode (using a LUT), uses lockState for shouldShift) */
RGFWDEF char RGFW_keyCodeToCharAuto(u32 keycode, u8 lockState);

/*! if window == NULL, it checks if the key is pressed globally. Otherwise, it checks only if the key is pressed while the window in focus.*/
RGFWDEF b8 RGFW_isPressed(RGFW_window* win, u8 key); /*!< if key is pressed (key code)*/

RGFWDEF b8 RGFW_wasPressed(RGFW_window* win, u8 key); /*!< if key was pressed (checks previous state only) (key code)*/

RGFWDEF b8 RGFW_isHeld(RGFW_window* win, u8 key); /*!< if key is held (key code)*/
RGFWDEF b8 RGFW_isReleased(RGFW_window* win, u8 key); /*!< if key is released (key code)*/

/* if a key is pressed and then released, pretty much the same as RGFW_isReleased */
RGFWDEF b8 RGFW_isClicked(RGFW_window* win, u8 key /*!< key code*/);

/*! if a mouse button is pressed */
RGFWDEF b8 RGFW_isMousePressed(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button is held */
RGFWDEF b8 RGFW_isMouseHeld(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button was released */
RGFWDEF b8 RGFW_isMouseReleased(RGFW_window* win, u8 button /*!< mouse button code */ );
/*! if a mouse button was pressed (checks previous state only) */
RGFWDEF b8 RGFW_wasMousePressed(RGFW_window* win, u8 button /*!< mouse button code */ );
/** @} */ 

/** * @defgroup Clipboard
* @{ */ 
RGFWDEF char* RGFW_readClipboard(size_t* size); /*!< read clipboard data */
RGFWDEF void RGFW_clipboardFree(char* str); /*!< the string returned from RGFW_readClipboard must be freed */

RGFWDEF void RGFW_writeClipboard(const char* text, u32 textLen); /*!< write text to the clipboard */
/** @} */ 

/**
	
	
	Event callbacks, 
	these are completely optional, you can use the normal 
	RGFW_checkEvent() method if you prefer that

* @defgroup Callbacks
* @{ 
*/

/*! RGFW_windowMoved, the window and its new rect value  */
typedef void (* RGFW_windowmovefunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_windowResized, the window and its new rect value  */
typedef void (* RGFW_windowresizefunc)(RGFW_window* win, RGFW_rect r);
/*! RGFW_quit, the window that was closed */
typedef void (* RGFW_windowquitfunc)(RGFW_window* win);
/*! RGFW_focusIn / RGFW_focusOut, the window who's focus has changed and if its inFocus */
typedef void (* RGFW_focusfunc)(RGFW_window* win, b8 inFocus);
/*! RGFW_mouseEnter / RGFW_mouseLeave, the window that changed, the point of the mouse (enter only) and if the mouse has entered */
typedef void (* RGFW_mouseNotifyfunc)(RGFW_window* win, RGFW_point point, b8 status);
/*! RGFW_mousePosChanged, the window that the move happened on and the new point of the mouse  */
typedef void (* RGFW_mouseposfunc)(RGFW_window* win, RGFW_point point);
/*! RGFW_dnd_init, the window, the point of the drop on the windows */
typedef void (* RGFW_dndInitfunc)(RGFW_window* win, RGFW_point point);
/*! RGFW_windowRefresh, the window that needs to be refreshed */
typedef void (* RGFW_windowrefreshfunc)(RGFW_window* win);
/*! RGFW_keyPressed / RGFW_keyReleased, the window that got the event, the keycode, the string version, the state of mod keys, if it was a press (else it's a release) */
typedef void (* RGFW_keyfunc)(RGFW_window* win, u32 keycode, char keyName[16], u8 lockState, b8 pressed);
/*! RGFW_mouseButtonPressed / RGFW_mouseButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release)  */
typedef void (* RGFW_mousebuttonfunc)(RGFW_window* win, u8 button, double scroll, b8 pressed);
/*! RGFW_jsButtonPressed / RGFW_jsButtonReleased, the window that got the event, the button that was pressed, the scroll value, if it was a press (else it's a release) */
typedef void (* RGFW_jsButtonfunc)(RGFW_window* win, u16 joystick, u8 button, b8 pressed);
/*! RGFW_jsAxisMove, the window that got the event, the joystick in question, the axis values and the amount of axises */
typedef void (* RGFW_jsAxisfunc)(RGFW_window* win, u16 joystick, RGFW_point axis[2], u8 axisesCount);


/*!  RGFW_dnd, the window that had the drop, the drop data and the amount files dropped returns previous callback function (if it was set) */
#ifdef RGFW_ALLOC_DROPFILES
	typedef void (* RGFW_dndfunc)(RGFW_window* win, char** droppedFiles, u32 droppedFilesCount);
#else
	typedef void (* RGFW_dndfunc)(RGFW_window* win, char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH], u32 droppedFilesCount);
#endif
/*! set callback for a window move event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowmovefunc RGFW_setWindowMoveCallback(RGFW_windowmovefunc func);
/*! set callback for a window resize event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowresizefunc RGFW_setWindowResizeCallback(RGFW_windowresizefunc func);
/*! set callback for a window quit event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowquitfunc RGFW_setWindowQuitCallback(RGFW_windowquitfunc func);
/*! set callback for a mouse move event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseposfunc RGFW_setMousePosCallback(RGFW_mouseposfunc func);
/*! set callback for a window refresh event returns previous callback function (if it was set)  */
RGFWDEF RGFW_windowrefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowrefreshfunc func);
/*! set callback for a window focus change event returns previous callback function (if it was set)  */
RGFWDEF RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func);
/*! set callback for a mouse notify event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallBack(RGFW_mouseNotifyfunc func);
/*! set callback for a drop event event returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndfunc RGFW_setDndCallback(RGFW_dndfunc func);
/*! set callback for a start of a drop event returns previous callback function (if it was set)  */
RGFWDEF RGFW_dndInitfunc RGFW_setDndInitCallback(RGFW_dndInitfunc func);
/*! set callback for a key (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func);
/*! set callback for a mouse button (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_mousebuttonfunc RGFW_setMouseButtonCallback(RGFW_mousebuttonfunc func);
/*! set callback for a controller button (press / release ) event returns previous callback function (if it was set)  */
RGFWDEF RGFW_jsButtonfunc RGFW_setjsButtonCallback(RGFW_jsButtonfunc func);
/*! set callback for a joystick axis mov event returns previous callback function (if it was set)  */
RGFWDEF RGFW_jsAxisfunc RGFW_setjsAxisCallback(RGFW_jsAxisfunc func);

/** @} */ 

/** * @defgroup Threads
* @{ */ 

#ifndef RGFW_NO_THREADS
	/*! threading functions*/

	/*! NOTE! (for X11/linux) : if you define a window in a thread, it must be run after the original thread's window is created or else there will be a memory error */
	/*
		I'd suggest you use sili's threading functions instead
		if you're going to use sili
		which is a good idea generally
	*/

	#if defined(__unix__) || defined(__APPLE__) || defined(RGFW_WEBASM) 
		typedef void* (* RGFW_threadFunc_ptr)(void*);
	#else
		typedef DWORD (__stdcall *RGFW_threadFunc_ptr) (LPVOID lpThreadParameter);  
	#endif

	RGFWDEF RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args); /*!< create a thread*/
	RGFWDEF void RGFW_cancelThread(RGFW_thread thread); /*!< cancels a thread*/
	RGFWDEF void RGFW_joinThread(RGFW_thread thread); /*!< join thread to current thread */
	RGFWDEF void RGFW_setThreadPriority(RGFW_thread thread, u8 priority); /*!< sets the priority priority  */
#endif

/** @} */ 

/** * @defgroup joystick
* @{ */ 

/*! joystick count starts at 0*/
/*!< register joystick to window based on a number (the number is based on when it was connected eg. /dev/js0)*/
RGFWDEF u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber);
RGFWDEF u16 RGFW_registerJoystickF(RGFW_window* win, char* file);

RGFWDEF u32 RGFW_isPressedJS(RGFW_window* win, u16 controller, u8 button);

/** @} */ 

/** * @defgroup graphics_API
* @{ */ 

/*!< make the window the current opengl drawing context

	NOTE:
 	if you want to switch the graphics context's thread, 
	you have to run RGFW_window_makeCurrent(NULL); on the old thread
	then RGFW_window_makeCurrent(valid_window) on the new thread
*/
RGFWDEF void RGFW_window_makeCurrent(RGFW_window* win);

/*< updates fps / sets fps to cap (must by ran manually by the user at the end of a frame), returns current fps */
RGFWDEF u32 RGFW_window_checkFPS(RGFW_window* win, u32 fpsCap);

/* supports openGL, directX, OSMesa, EGL and software rendering */
RGFWDEF void RGFW_window_swapBuffers(RGFW_window* win); /*!< swap the rendering buffer */
RGFWDEF void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval);

RGFWDEF void RGFW_window_setGPURender(RGFW_window* win, i8 set);
RGFWDEF void RGFW_window_setCPURender(RGFW_window* win, i8 set);

/*! native API functions */
#if defined(RGFW_OPENGL) || defined(RGFW_EGL)
	/*! OpenGL init hints */
	RGFWDEF void RGFW_setGLStencil(i32 stencil); /*!< set stencil buffer bit size (8 by default) */
	RGFWDEF void RGFW_setGLSamples(i32 samples); /*!< set number of sampiling buffers (4 by default) */
	RGFWDEF void RGFW_setGLStereo(i32 stereo); /*!< use GL_STEREO (GL_FALSE by default) */
	RGFWDEF void RGFW_setGLAuxBuffers(i32 auxBuffers); /*!< number of aux buffers (0 by default) */

	/*! which profile to use for the opengl verion */
	typedef RGFW_ENUM(u8, RGFW_GL_profile)  { RGFW_GL_CORE = 0,  RGFW_GL_COMPATIBILITY  };
	/*! Set OpenGL version hint (core or compatibility profile)*/
	RGFWDEF void RGFW_setGLVersion(RGFW_GL_profile profile, i32 major, i32 minor);
	RGFWDEF void RGFW_setDoubleBuffer(b8 useDoubleBuffer); 
    RGFWDEF void* RGFW_getProcAddress(const char* procname); /*!< get native opengl proc address */
    RGFWDEF void RGFW_window_makeCurrent_OpenGL(RGFW_window* win); /*!< to be called by RGFW_window_makeCurrent */

/** @} */ 

/** * @defgroup Supporting
* @{ */ 
RGFWDEF u64 RGFW_getTime(void); /*!< get time in seconds */
RGFWDEF u64 RGFW_getTimeNS(void); /*!< get time in nanoseconds */
RGFWDEF void RGFW_sleep(u64 milisecond); /*!< sleep for a set time */

/*!
	key codes and mouse icon enums
*/

typedef RGFW_ENUM(u8, RGFW_Key) {
	RGFW_KEY_NULL = 0,
	RGFW_Escape,
	RGFW_F1,
	RGFW_F2,
	RGFW_F3,
	RGFW_F4,
	RGFW_F5,
	RGFW_F6,
	RGFW_F7,
	RGFW_F8,
	RGFW_F9,
	RGFW_F10,
	RGFW_F11,
	RGFW_F12,

	RGFW_Backtick,

	RGFW_0,
	RGFW_1,
	RGFW_2,
	RGFW_3,
	RGFW_4,
	RGFW_5,
	RGFW_6,
	RGFW_7,
	RGFW_8,
	RGFW_9,

	RGFW_Minus,
	RGFW_Equals,
	RGFW_BackSpace,
	RGFW_Tab,
	RGFW_CapsLock,
	RGFW_ShiftL,
	RGFW_ControlL,
	RGFW_AltL,
	RGFW_SuperL,
	RGFW_ShiftR,
	RGFW_ControlR,
	RGFW_AltR,
	RGFW_SuperR,
	RGFW_Space,

	RGFW_a,
	RGFW_b,
	RGFW_c,
	RGFW_d,
	RGFW_e,
	RGFW_f,
	RGFW_g,
	RGFW_h,
	RGFW_i,
	RGFW_j,
	RGFW_k,
	RGFW_l,
	RGFW_m,
	RGFW_n,
	RGFW_o,
	RGFW_p,
	RGFW_q,
	RGFW_r,
	RGFW_s,
	RGFW_t,
	RGFW_u,
	RGFW_v,
	RGFW_w,
	RGFW_x,
	RGFW_y,
	RGFW_z,

	RGFW_Period,
	RGFW_Comma,
	RGFW_Slash,
	RGFW_Bracket,
	RGFW_CloseBracket,
	RGFW_Semicolon,
	RGFW_Return,
	RGFW_Quote,
	RGFW_BackSlash,

	RGFW_Up,
	RGFW_Down,
	RGFW_Left,
	RGFW_Right,

	RGFW_Delete,
	RGFW_Insert,
	RGFW_End,
	RGFW_Home,
	RGFW_PageUp,
	RGFW_PageDown,

	RGFW_Numlock,
	RGFW_KP_Slash,
	RGFW_Multiply,
	RGFW_KP_Minus,
	RGFW_KP_1,
	RGFW_KP_2,
	RGFW_KP_3,
	RGFW_KP_4,
	RGFW_KP_5,
	RGFW_KP_6,
	RGFW_KP_7,
	RGFW_KP_8,
	RGFW_KP_9,
	RGFW_KP_0,
	RGFW_KP_Period,
	RGFW_KP_Return,

	final_key,
};


typedef RGFW_ENUM(u8, RGFW_mouseIcons) {
	RGFW_MOUSE_NORMAL = 0,
	RGFW_MOUSE_ARROW,
	RGFW_MOUSE_IBEAM,
	RGFW_MOUSE_CROSSHAIR,
	RGFW_MOUSE_POINTING_HAND,
	RGFW_MOUSE_RESIZE_EW,
	RGFW_MOUSE_RESIZE_NS,
	RGFW_MOUSE_RESIZE_NWSE,
	RGFW_MOUSE_RESIZE_NESW,
	RGFW_MOUSE_RESIZE_ALL,
	RGFW_MOUSE_NOT_ALLOWED,
};

/** @} */ 

#endif /* RGFW_HEADER */

/*
Example to get you started :

linux : gcc main.c -lX11 -lXcursor -lGL
windows : gcc main.c -lopengl32 -lshell32 -lgdi32
macos : gcc main.c -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo

#define RGFW_IMPLEMENTATION
#include "RGFW.h"

u8 icon[4 * 3 * 3] = {0xFF, 0x00, 0x00, 0xFF,    0xFF, 0x00, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

int main() {
	RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(500, 500, 500, 500), (u64)0);

	RGFW_window_setIcon(win, icon, RGFW_AREA(3, 3), 4);

	for (;;) {
		RGFW_window_checkEvent(win); // NOTE: checking events outside of a while loop may cause input lag
		if (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_Escape))
			break;

		RGFW_window_swapBuffers(win);

		glClearColor(0xFF, 0XFF, 0xFF, 0xFF);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	RGFW_window_close(win);
}

	compiling :

	if you wish to compile the library all you have to do is create a new file with this in it

	rgfw.c
	#define RGFW_IMPLEMENTATION
	#include "RGFW.h"

	then you can use gcc (or whatever compile you wish to use) to compile the library into object file

	ex. gcc -c RGFW.c -fPIC

	after you compile the library into an object file, you can also turn the object file into an static or shared library

	(commands ar and gcc can be replaced with whatever equivalent your system uses)
	static : ar rcs RGFW.a RGFW.o
	shared :
		windows:
			gcc -shared RGFW.o -lopengl32 -lshell32 -lgdi32 -o RGFW.dll
		linux:
			gcc -shared RGFW.o -lX11 -lXcursor -lGL -o RGFW.so
		macos:
			gcc -shared RGFW.o -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo
*/

#ifdef RGFW_DRM
	#define RGFW_OS_BASED_VALUE(d) d
#endif


#ifdef RGFW_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/*
RGFW_IMPLEMENTATION starts with generic RGFW defines

This is the start of keycode data
*/



/* 
	the c++ compiler doesn't support setting up an array like, 
	we'll have to do it during runtime using a function & this messy setup
*/
#ifndef __cplusplus
#define RGFW_NEXT ,
#define RGFW_MAP
#else 
#define RGFW_NEXT ;
#define RGFW_MAP RGFW_keycodes
#endif

#ifdef RGFW_DRM
#include <linux/input-event-codes.h>
#endif

u8 RGFW_keycodes [RGFW_OS_BASED_VALUE(130)] = {
#ifdef __cplusplus
	0
};
void RGFW_init_keys(void) {
#endif
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_GRAVE)] = RGFW_Backtick 		RGFW_NEXT

	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_0)] = RGFW_0 					RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_1)] = RGFW_1						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_2)] = RGFW_2						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_3)] = RGFW_3						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_4)] = RGFW_4						RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_5)] = RGFW_5                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_6)] = RGFW_6                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_7)] = RGFW_7                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_8)] = RGFW_8                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_9)] = RGFW_9,

	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_SPACE)] = RGFW_Space,

	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_A)] = RGFW_a                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_B)] = RGFW_b                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_C)] = RGFW_c                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_D)] = RGFW_d                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_E)] = RGFW_e                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F)] = RGFW_f                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_G)] = RGFW_g                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_H)] = RGFW_h                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_I)] = RGFW_i                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_J)] = RGFW_j                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_K)] = RGFW_k                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_L)] = RGFW_l                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_M)] = RGFW_m                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_N)] = RGFW_n                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_O)] = RGFW_o                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_P)] = RGFW_p                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_Q)] = RGFW_q                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_R)] = RGFW_r                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_S)] = RGFW_s                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_T)] = RGFW_t                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_U)] = RGFW_u                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_V)] = RGFW_v                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_W)] = RGFW_w                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_X)] = RGFW_x                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_Y)] = RGFW_y                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_Z)] = RGFW_z,

	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_DOT)] = RGFW_Period             			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_COMMA)] = RGFW_Comma               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_SLASH)] = RGFW_Slash               			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFTBRACE)] = RGFW_Bracket      			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHTBRACE)] = RGFW_CloseBracket             RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_SEMICOLON)] = RGFW_Semicolon                 RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_APOSTROPHE)] = RGFW_Quote                 			RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_BACKSLASH)] = RGFW_BackSlash,
	
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_ENTER)] = RGFW_Return              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_DELETE)] = RGFW_Delete                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_NUMLOCK)] = RGFW_Numlock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KPSLASH)] = RGFW_KP_Slash               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KPASTERISK)] = RGFW_Multiply              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KPMINUS)] = RGFW_KP_Minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP1)] = RGFW_KP_1               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP2)] = RGFW_KP_2               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP3)] = RGFW_KP_3               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP4)] = RGFW_KP_4               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP5)] = RGFW_KP_5               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP6)] = RGFW_KP_6               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP7)] = RGFW_KP_7               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP8)] = RGFW_KP_8               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP9)] = RGFW_KP_9               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KP0)] = RGFW_KP_0               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KPDOT)] = RGFW_KP_Period              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_KPENTER)] = RGFW_KP_Return,
	
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_MINUS)] = RGFW_Minus              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_EQUAL)] = RGFW_Equals               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_BACKSPACE)] = RGFW_BackSpace              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_TAB)] = RGFW_Tab                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_CAPSLOCK)] = RGFW_CapsLock               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFTSHIFT)] = RGFW_ShiftL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFTCTRL)] = RGFW_ControlL               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFTALT)] = RGFW_AltL                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFTMETA)] = RGFW_SuperL,
	
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHTCTRL)] = RGFW_ControlR               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHTMETA)] = RGFW_SuperR,
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHTSHIFT)] = RGFW_ShiftR              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHTALT)] = RGFW_AltR,

	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F1)] = RGFW_F1                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F2)] = RGFW_F2                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F3)] = RGFW_F3                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F4)] = RGFW_F4                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F5)] = RGFW_F5              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F6)] = RGFW_F6              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F7)] = RGFW_F7              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F8)] = RGFW_F8                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F9)] = RGFW_F9                 		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F10)] = RGFW_F10               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F11)] = RGFW_F11               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_F12)] = RGFW_F12               RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_UP)] = RGFW_Up                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_DOWN)] = RGFW_Down                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_LEFT)] = RGFW_Left                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_RIGHT)] = RGFW_Right              RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_INSERT)] = RGFW_Insert                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_END)] = RGFW_End                  		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_PAGEUP)] = RGFW_PageUp                		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_PAGEDOWN)] = RGFW_PageDown            RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_ESC)] = RGFW_Escape                   		RGFW_NEXT
	RGFW_MAP [RGFW_OS_BASED_VALUE(KEY_HOME)] = RGFW_Home                    		RGFW_NEXT
#ifndef __cplusplus
};
#else 
}
#endif

#undef RGFW_NEXT
#undef RGFW_MAP

typedef struct {
	b8 current  : 1;
	b8 prev  : 1;
} RGFW_keyState;

RGFW_keyState RGFW_keyboard[final_key] = { {0, 0} };

RGFWDEF u32 RGFW_apiKeyCodeToRGFW(u32 keycode);

u32 RGFW_apiKeyCodeToRGFW(u32 keycode) {
	#ifdef __cplusplus
	if (RGFW_OS_BASED_VALUE(49, 192, 50, DOM_VK_BACK_QUOTE, KEY_GRAVE) != RGFW_Backtick) {
		RGFW_init_keys();
	}
	#endif

	/* make sure the key isn't out of bounds */
	if (keycode > sizeof(RGFW_keycodes) / sizeof(u8))
		return 0;
	
	return RGFW_keycodes[keycode];
}

RGFWDEF void RGFW_resetKey(void);
void RGFW_resetKey(void) {
	size_t len = final_key; /*!< last_key == length */
	
	size_t i; /*!< reset each previous state  */
	for (i = 0; i < len; i++)
		RGFW_keyboard[i].prev = 0;
}

b8 RGFW_shouldShift(u32 keycode, u8 lockState) {
    #define RGFW_xor(x, y) (( (x) && (!(y)) ) ||  ((y) && (!(x)) ))
    b8 caps4caps = (lockState & RGFW_CAPSLOCK) && ((keycode >= RGFW_a) && (keycode <= RGFW_z));
    b8 shouldShift = RGFW_xor((RGFW_isPressed(NULL, RGFW_ShiftL) || RGFW_isPressed(NULL, RGFW_ShiftR)), caps4caps);
    #undef RGFW_xor

	return shouldShift;
}	

char RGFW_keyCodeToChar(u32 keycode, b8 shift) {
    static const char map[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', '0', '1', '2', '3', '4', '5', '6', '7', '8', 
        '9', '-', '=', 0, '\t',  0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '.', ',', '/', '[', ']',  ';', '\n', '\'', '\\', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  '/', '*', '-', '1', '2', '3',  '3', '5', '6', '7', '8',  '9', '0', '\n'
    };

    static const char mapCaps[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', ')', '!', '@', '#', '$', '%', '^', '&', '*', 
        '(', '_', '+', 0, '0',  0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '>', '<', '?', '{', '}',  ':', '\n', '"', '|', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '?', '*', '-', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if (shift == RGFW_FALSE)
        return map[keycode]; 
    return mapCaps[keycode];
}

char RGFW_keyCodeToCharAuto(u32 keycode, u8 lockState) { return RGFW_keyCodeToChar(keycode, RGFW_shouldShift(keycode, lockState)); }

/*
	this is the end of keycode data
*/

/* joystick data */
u8 RGFW_jsPressed[4][16]; /*!< if a key is currently pressed or not (per joystick) */

i32 RGFW_joysticks[4]; /*!< limit of 4 joysticks at a time */
u16 RGFW_joystickCount; /*!< the actual amount of joysticks */

/* 
	event callback defines start here
*/


/*
	These exist to avoid the 
	if (func == NULL) check 
	for (allegedly) better performance
*/
void RGFW_windowmovefuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
void RGFW_windowresizefuncEMPTY(RGFW_window* win, RGFW_rect r) { RGFW_UNUSED(win); RGFW_UNUSED(r); }
void RGFW_windowquitfuncEMPTY(RGFW_window* win) { RGFW_UNUSED(win); }
void RGFW_focusfuncEMPTY(RGFW_window* win, b8 inFocus) {RGFW_UNUSED(win); RGFW_UNUSED(inFocus);}
void RGFW_mouseNotifyfuncEMPTY(RGFW_window* win, RGFW_point point, b8 status) {RGFW_UNUSED(win); RGFW_UNUSED(point); RGFW_UNUSED(status);}
void RGFW_mouseposfuncEMPTY(RGFW_window* win, RGFW_point point) {RGFW_UNUSED(win); RGFW_UNUSED(point);}
void RGFW_dndInitfuncEMPTY(RGFW_window* win, RGFW_point point) {RGFW_UNUSED(win); RGFW_UNUSED(point);}
void RGFW_windowrefreshfuncEMPTY(RGFW_window* win) {RGFW_UNUSED(win); }
void RGFW_keyfuncEMPTY(RGFW_window* win, u32 keycode, char keyName[16], u8 lockState, b8 pressed) {RGFW_UNUSED(win); RGFW_UNUSED(keycode); RGFW_UNUSED(keyName); RGFW_UNUSED(lockState); RGFW_UNUSED(pressed);}
void RGFW_mousebuttonfuncEMPTY(RGFW_window* win, u8 button, double scroll, b8 pressed) {RGFW_UNUSED(win); RGFW_UNUSED(button); RGFW_UNUSED(scroll); RGFW_UNUSED(pressed);}
void RGFW_jsButtonfuncEMPTY(RGFW_window* win, u16 joystick, u8 button, b8 pressed){RGFW_UNUSED(win); RGFW_UNUSED(joystick); RGFW_UNUSED(button); RGFW_UNUSED(pressed); }
void RGFW_jsAxisfuncEMPTY(RGFW_window* win, u16 joystick, RGFW_point axis[2], u8 axisesCount){RGFW_UNUSED(win); RGFW_UNUSED(joystick); RGFW_UNUSED(axis); RGFW_UNUSED(axisesCount); }

#ifdef RGFW_ALLOC_DROPFILES
void RGFW_dndfuncEMPTY(RGFW_window* win, char** droppedFiles, u32 droppedFilesCount) {RGFW_UNUSED(win); RGFW_UNUSED(droppedFiles); RGFW_UNUSED(droppedFilesCount);}
#else
void RGFW_dndfuncEMPTY(RGFW_window* win, char droppedFiles[RGFW_MAX_DROPS][RGFW_MAX_PATH], u32 droppedFilesCount) {RGFW_UNUSED(win); RGFW_UNUSED(droppedFiles); RGFW_UNUSED(droppedFilesCount);}
#endif

RGFW_windowmovefunc RGFW_windowMoveCallback = RGFW_windowmovefuncEMPTY;
RGFW_windowresizefunc RGFW_windowResizeCallback = RGFW_windowresizefuncEMPTY;
RGFW_windowquitfunc RGFW_windowQuitCallback = RGFW_windowquitfuncEMPTY;
RGFW_mouseposfunc RGFW_mousePosCallback = RGFW_mouseposfuncEMPTY;
RGFW_windowrefreshfunc RGFW_windowRefreshCallback = RGFW_windowrefreshfuncEMPTY;
RGFW_focusfunc RGFW_focusCallback = RGFW_focusfuncEMPTY;
RGFW_mouseNotifyfunc RGFW_mouseNotifyCallBack = RGFW_mouseNotifyfuncEMPTY;
RGFW_dndfunc RGFW_dndCallback = RGFW_dndfuncEMPTY;
RGFW_dndInitfunc RGFW_dndInitCallback = RGFW_dndInitfuncEMPTY;
RGFW_keyfunc RGFW_keyCallback = RGFW_keyfuncEMPTY;
RGFW_mousebuttonfunc RGFW_mouseButtonCallback = RGFW_mousebuttonfuncEMPTY;
RGFW_jsButtonfunc RGFW_jsButtonCallback = RGFW_jsButtonfuncEMPTY;
RGFW_jsAxisfunc RGFW_jsAxisCallback = RGFW_jsAxisfuncEMPTY;

void RGFW_window_checkEvents(RGFW_window* win, i32 waitMS) { 
	RGFW_window_eventWait(win, waitMS);

	while (RGFW_window_checkEvent(win) != NULL && RGFW_window_shouldClose(win) == 0) { 
		if (win->event.type == RGFW_quit) return; 
	}
	
	#ifdef RGFW_WEBASM /* webasm needs to run the sleep function for asyncify */
		RGFW_sleep(0);
	#endif
}

RGFW_windowmovefunc RGFW_setWindowMoveCallback(RGFW_windowmovefunc func) { 
	RGFW_windowmovefunc	prev =  (RGFW_windowMoveCallback == RGFW_windowmovefuncEMPTY) ? NULL : RGFW_windowMoveCallback;
	RGFW_windowMoveCallback = func;
	return prev;
}
RGFW_windowresizefunc RGFW_setWindowResizeCallback(RGFW_windowresizefunc func) {
    RGFW_windowresizefunc prev = (RGFW_windowResizeCallback == RGFW_windowresizefuncEMPTY) ? NULL : RGFW_windowResizeCallback;
    RGFW_windowResizeCallback = func;
    return prev;
}
RGFW_windowquitfunc RGFW_setWindowQuitCallback(RGFW_windowquitfunc func) {
    RGFW_windowquitfunc prev = (RGFW_windowQuitCallback == RGFW_windowquitfuncEMPTY) ? NULL : RGFW_windowQuitCallback;
    RGFW_windowQuitCallback = func;
    return prev;
}

RGFW_mouseposfunc RGFW_setMousePosCallback(RGFW_mouseposfunc func) {
    RGFW_mouseposfunc prev = (RGFW_mousePosCallback == RGFW_mouseposfuncEMPTY) ? NULL : RGFW_mousePosCallback;
    RGFW_mousePosCallback = func;
    return prev;
}
RGFW_windowrefreshfunc RGFW_setWindowRefreshCallback(RGFW_windowrefreshfunc func) {
    RGFW_windowrefreshfunc prev = (RGFW_windowRefreshCallback == RGFW_windowrefreshfuncEMPTY) ? NULL : RGFW_windowRefreshCallback;
    RGFW_windowRefreshCallback = func;
    return prev;
}
RGFW_focusfunc RGFW_setFocusCallback(RGFW_focusfunc func) {
    RGFW_focusfunc prev = (RGFW_focusCallback == RGFW_focusfuncEMPTY) ? NULL : RGFW_focusCallback;
    RGFW_focusCallback = func;
    return prev;
}

RGFW_mouseNotifyfunc RGFW_setMouseNotifyCallBack(RGFW_mouseNotifyfunc func) {
    RGFW_mouseNotifyfunc prev = (RGFW_mouseNotifyCallBack == RGFW_mouseNotifyfuncEMPTY) ? NULL : RGFW_mouseNotifyCallBack;
    RGFW_mouseNotifyCallBack = func;
    return prev;
}
RGFW_dndfunc RGFW_setDndCallback(RGFW_dndfunc func) {
    RGFW_dndfunc prev = (RGFW_dndCallback == RGFW_dndfuncEMPTY) ? NULL : RGFW_dndCallback;
    RGFW_dndCallback = func;
    return prev;
}
RGFW_dndInitfunc RGFW_setDndInitCallback(RGFW_dndInitfunc func) {
    RGFW_dndInitfunc prev = (RGFW_dndInitCallback == RGFW_dndInitfuncEMPTY) ? NULL : RGFW_dndInitCallback;
    RGFW_dndInitCallback = func;
    return prev;
}
RGFW_keyfunc RGFW_setKeyCallback(RGFW_keyfunc func) {
    RGFW_keyfunc prev = (RGFW_keyCallback == RGFW_keyfuncEMPTY) ? NULL : RGFW_keyCallback;
    RGFW_keyCallback = func;
    return prev;
}
RGFW_mousebuttonfunc RGFW_setMouseButtonCallback(RGFW_mousebuttonfunc func) {
    RGFW_mousebuttonfunc prev = (RGFW_mouseButtonCallback == RGFW_mousebuttonfuncEMPTY) ? NULL : RGFW_mouseButtonCallback;
    RGFW_mouseButtonCallback = func;
    return prev;
}
RGFW_jsButtonfunc RGFW_setjsButtonCallback(RGFW_jsButtonfunc func) {
    RGFW_jsButtonfunc prev = (RGFW_jsButtonCallback == RGFW_jsButtonfuncEMPTY) ? NULL : RGFW_jsButtonCallback;
    RGFW_jsButtonCallback = func;
    return prev;
}
RGFW_jsAxisfunc RGFW_setjsAxisCallback(RGFW_jsAxisfunc func) {
    RGFW_jsAxisfunc prev = (RGFW_jsAxisCallback == RGFW_jsAxisfuncEMPTY) ? NULL : RGFW_jsAxisCallback;
    RGFW_jsAxisCallback = func;
    return prev;
}
/* 
no more event call back defines
*/

#define RGFW_ASSERT(check, str) {\
	if (!(check)) { \
		printf(str); \
		assert(check); \
	} \
}

b8 RGFW_error = 0;
b8 RGFW_Error(void) { return RGFW_error; }

#define SET_ATTRIB(a, v) { \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}
	
RGFW_area RGFW_bufferSize = {0, 0};
void RGFW_setBufferSize(RGFW_area size) {
	RGFW_bufferSize = size;
}


RGFWDEF RGFW_window* RGFW_window_basic_init(RGFW_rect rect, u16 args);

/* do a basic initialization for RGFW_window, this is to standard it for each OS */
RGFW_window* RGFW_window_basic_init(RGFW_rect rect, u16 args) {
	RGFW_window* win = (RGFW_window*) RGFW_MALLOC(sizeof(RGFW_window)); /*!< make a new RGFW struct */

	/* clear out dnd info */
#ifdef RGFW_ALLOC_DROPFILES
	win->event.droppedFiles = (char**) RGFW_MALLOC(sizeof(char*) * RGFW_MAX_DROPS);
	u32 i;
	for (i = 0; i < RGFW_MAX_DROPS; i++)
		win->event.droppedFiles[i] = (char*) RGFW_CALLOC(RGFW_MAX_PATH, sizeof(char));
#endif

	RGFW_area screenR = RGFW_getScreenSize();
	
	/* rect based the requested args */
	if (args & RGFW_FULLSCREEN)
		rect = RGFW_RECT(0, 0, screenR.w, screenR.h);

	if (args & RGFW_CENTER)
		rect = RGFW_RECT((screenR.w - rect.w) / 2, (screenR.h - rect.h) / 2, rect.w, rect.h);

	/* set and init the new window's data */
	win->r = rect;
	win->event.inFocus = 1;
	win->event.droppedFilesCount = 0;
	RGFW_joystickCount = 0;
	win->_winArgs = 0;
	win->event.lockState = 0;

	return win;
}

#ifndef RGFW_NO_MONITOR
void RGFW_window_scaleToMonitor(RGFW_window* win) {
	RGFW_monitor monitor = RGFW_window_getMonitor(win);
	
	RGFW_window_resize(win, RGFW_AREA(((u32) monitor.scaleX) * win->r.w, ((u32) monitor.scaleX) * win->r.h));
}
#endif

RGFW_window* RGFW_root = NULL;


#define RGFW_HOLD_MOUSE			(1L<<2) /*!< hold the moues still */
#define RGFW_MOUSE_LEFT 		(1L<<3) /* if mouse left the window */

void RGFW_clipboardFree(char* str) { RGFW_FREE(str); }

RGFW_keyState RGFW_mouseButtons[5] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };

b8 RGFW_isMousePressed(RGFW_window* win, u8 button) {
	assert(win != NULL);
	return RGFW_mouseButtons[button].current && (win != NULL) && win->event.inFocus; 
}
b8 RGFW_wasMousePressed(RGFW_window* win, u8 button) {
	assert(win != NULL); 
	return RGFW_mouseButtons[button].prev && (win != NULL) && win->event.inFocus; 
}
b8 RGFW_isMouseHeld(RGFW_window* win, u8 button) {
	return (RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));
}
b8 RGFW_isMouseReleased(RGFW_window* win, u8 button) {
	return (!RGFW_isMousePressed(win, button) && RGFW_wasMousePressed(win, button));	
}

b8 RGFW_isPressed(RGFW_window* win, u8 key) {
	return RGFW_keyboard[key].current && (win == NULL || win->event.inFocus);
}

b8 RGFW_wasPressed(RGFW_window* win, u8 key) {
	return RGFW_keyboard[key].prev && (win == NULL || win->event.inFocus);
}

b8 RGFW_isHeld(RGFW_window* win, u8 key) {
	return (RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));
}

b8 RGFW_isClicked(RGFW_window* win, u8 key) {
	return (RGFW_wasPressed(win, key) && !RGFW_isPressed(win, key));
}

b8 RGFW_isReleased(RGFW_window* win, u8 key) {
	return (!RGFW_isPressed(win, key) && RGFW_wasPressed(win, key));	
}

void RGFW_window_makeCurrent(RGFW_window* win) {
#if defined(RGFW_OPENGL)
	RGFW_window_makeCurrent_OpenGL(win);
#else
	RGFW_UNUSED(win)
#endif
}

void RGFW_window_setGPURender(RGFW_window* win, i8 set) {
	if (!set && !(win->_winArgs & RGFW_NO_GPU_RENDER))
		win->_winArgs |= RGFW_NO_GPU_RENDER;
		
	else if (set && win->_winArgs & RGFW_NO_GPU_RENDER)
		win->_winArgs ^= RGFW_NO_GPU_RENDER;
}

void RGFW_window_setCPURender(RGFW_window* win, i8 set) {
	if (!set && !(win->_winArgs & RGFW_NO_CPU_RENDER))
		win->_winArgs |= RGFW_NO_CPU_RENDER;

	else if (set && win->_winArgs & RGFW_NO_CPU_RENDER)
		win->_winArgs ^= RGFW_NO_CPU_RENDER;
}

void RGFW_window_maximize(RGFW_window* win) {
	assert(win != NULL);

	RGFW_area screen = RGFW_getScreenSize();

	RGFW_window_move(win, RGFW_POINT(0, 0));
	RGFW_window_resize(win, screen);
}

b8 RGFW_window_shouldClose(RGFW_window* win) {
	assert(win != NULL);
	return (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_Escape));
}

void RGFW_window_setShouldClose(RGFW_window* win) { win->event.type = RGFW_quit; RGFW_windowQuitCallback(win); }

#ifndef RGFW_NO_MONITOR
	void RGFW_window_moveToMonitor(RGFW_window* win, RGFW_monitor m) {
		RGFW_window_move(win, RGFW_POINT(m.rect.x + win->r.x, m.rect.y + win->r.y));
	}
#endif

RGFWDEF void RGFW_captureCursor(RGFW_window* win, RGFW_rect);
RGFWDEF void RGFW_releaseCursor(RGFW_window* win);

void RGFW_window_mouseHold(RGFW_window* win, RGFW_area area) {
	if ((win->_winArgs & RGFW_HOLD_MOUSE))
		return;
	

	if (!area.w && !area.h)
		area = RGFW_AREA(win->r.w / 2, win->r.h / 2);
		
	win->_winArgs |= RGFW_HOLD_MOUSE;
	RGFW_captureCursor(win, win->r);
	RGFW_window_moveMouse(win, RGFW_POINT(win->r.x + (win->r.w / 2), win->r.y + (win->r.h / 2)));
}

void RGFW_window_mouseUnhold(RGFW_window* win) {
	if ((win->_winArgs & RGFW_HOLD_MOUSE)) {
		win->_winArgs ^= RGFW_HOLD_MOUSE;

		RGFW_releaseCursor(win);
	}
}

u32 RGFW_window_checkFPS(RGFW_window* win, u32 fpsCap) {
	u64 deltaTime = RGFW_getTimeNS() - win->event.frameTime;

	u32 output_fps = 0;
	u64 fps = round(1e+9 / deltaTime);
	output_fps= fps;

	if (fpsCap && fps > fpsCap) {
		u64 frameTimeNS = 1e+9 / fpsCap;
		u64 sleepTimeMS = (frameTimeNS - deltaTime) / 1e6;

		if (sleepTimeMS > 0) {
			RGFW_sleep(sleepTimeMS);
			win->event.frameTime = 0;
		}
	}

	win->event.frameTime = RGFW_getTimeNS();
	
	if (fpsCap == 0) 
		return (u32) output_fps;
	
	deltaTime = RGFW_getTimeNS() - win->event.frameTime2;
	output_fps = round(1e+9 / deltaTime);
	win->event.frameTime2 = RGFW_getTimeNS();

	return output_fps;
}

u32 RGFW_isPressedJS(RGFW_window* win, u16 c, u8 button) { 
	RGFW_UNUSED(win);
	return RGFW_jsPressed[c][button]; 
}

RGFWDEF void RGFW_updateLockState(RGFW_window* win, b8 capital, b8 numlock);	
void RGFW_updateLockState(RGFW_window* win, b8 capital, b8 numlock) {
	if (capital && !(win->event.lockState & RGFW_CAPSLOCK))
		win->event.lockState |= RGFW_CAPSLOCK;
	else if (!capital && (win->event.lockState & RGFW_CAPSLOCK))			
		win->event.lockState ^= RGFW_CAPSLOCK;
	
	if (numlock && !(win->event.lockState & RGFW_NUMLOCK))
		win->event.lockState |= RGFW_NUMLOCK;
	else if (!numlock && (win->event.lockState & RGFW_NUMLOCK))
		win->event.lockState ^= RGFW_NUMLOCK;
}

#if defined(RGFW_DRM)
	struct timespec;

	int nanosleep(const struct timespec* duration, struct timespec* rem);
	int clock_gettime(clockid_t clk_id, struct timespec* tp);
	int setenv(const char *name, const char *value, int overwrite);

	void RGFW_window_setDND(RGFW_window* win, b8 allow) {
		if (allow && !(win->_winArgs & RGFW_ALLOW_DND))
			win->_winArgs |= RGFW_ALLOW_DND;

		else if (!allow && (win->_winArgs & RGFW_ALLOW_DND))
			win->_winArgs ^= RGFW_ALLOW_DND;
	}
#endif

/*
	graphics API specific code (end of generic code)
	starts here 
*/


/* 
	OpenGL defines start here   (Normal, EGL, OSMesa)
*/

#if defined(RGFW_OPENGL) || defined(RGFW_EGL) || defined(RGFW_OSMESA)
	#if !defined(__APPLE__) && !defined(RGFW_NO_GL_HEADER)
		#include <GL/gl.h>
	#endif

/* EGL, normal OpenGL only */
#if !defined(RGFW_OSMESA) 
	i32 RGFW_majorVersion = 0, RGFW_minorVersion = 0;
	b8 RGFW_profile = RGFW_GL_CORE;
	
	#ifndef RGFW_EGL
	i32 RGFW_STENCIL = 8, RGFW_SAMPLES = 4, RGFW_STEREO = 0, RGFW_AUX_BUFFERS = 0, RGFW_DOUBLE_BUFFER = 1;
	#else
	i32 RGFW_STENCIL = 0, RGFW_SAMPLES = 0, RGFW_STEREO = 0, RGFW_AUX_BUFFERS = 0, RGFW_DOUBLE_BUFFER = 1;
	#endif


	void RGFW_setGLStencil(i32 stencil) { RGFW_STENCIL = stencil; }
	void RGFW_setGLSamples(i32 samples) { RGFW_SAMPLES = samples; }
	void RGFW_setGLStereo(i32 stereo) { RGFW_STEREO = stereo; }
	void RGFW_setGLAuxBuffers(i32 auxBuffers) { RGFW_AUX_BUFFERS = auxBuffers; }
	void RGFW_setDoubleBuffer(b8 useDoubleBuffer) { RGFW_DOUBLE_BUFFER = useDoubleBuffer; }

	void RGFW_setGLVersion(b8 profile, i32 major, i32 minor) {
        RGFW_profile = profile;
		RGFW_majorVersion = major;
		RGFW_minorVersion = minor;
	}

/* EGL only (no OSMesa nor normal OPENGL) */
#elif defined(RGFW_EGL)

#include <EGL/egl.h>

#if defined(RGFW_LINK_EGL)
	typedef EGLBoolean(EGLAPIENTRY* PFN_eglInitialize)(EGLDisplay, EGLint*, EGLint*);

	PFNEGLINITIALIZEPROC eglInitializeSource;
	PFNEGLGETCONFIGSPROC eglGetConfigsSource;
	PFNEGLCHOOSECONFIGPROC eglChooseConfigSource;
	PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurfaceSource;
	PFNEGLCREATECONTEXTPROC eglCreateContextSource;
	PFNEGLMAKECURRENTPROC eglMakeCurrentSource;
	PFNEGLGETDISPLAYPROC eglGetDisplaySource;
	PFNEGLSWAPBUFFERSPROC eglSwapBuffersSource;
	PFNEGLSWAPINTERVALPROC eglSwapIntervalSource;
	PFNEGLBINDAPIPROC eglBindAPISource;
	PFNEGLDESTROYCONTEXTPROC eglDestroyContextSource;
	PFNEGLTERMINATEPROC eglTerminateSource;
	PFNEGLDESTROYSURFACEPROC eglDestroySurfaceSource;

#define eglInitialize eglInitializeSource
#define eglGetConfigs eglGetConfigsSource
#define eglChooseConfig eglChooseConfigSource
#define eglCreateWindowSurface eglCreateWindowSurfaceSource
#define eglCreateContext eglCreateContextSource
#define eglMakeCurrent eglMakeCurrentSource
#define eglGetDisplay eglGetDisplaySource
#define eglSwapBuffers eglSwapBuffersSource
#define eglSwapInterval eglSwapIntervalSource
#define eglBindAPI eglBindAPISource
#define eglDestroyContext eglDestroyContextSource
#define eglTerminate eglTerminateSource
#define eglDestroySurface eglDestroySurfaceSource;
#endif


#define EGL_SURFACE_MAJOR_VERSION_KHR 0x3098
#define EGL_SURFACE_MINOR_VERSION_KHR 0x30fb

#ifndef RGFW_GL_ADD_ATTRIB
#define RGFW_GL_ADD_ATTRIB(attrib, attVal) \
	if (attVal) { \
		attribs[index] = attrib;\
		attribs[index + 1] = attVal;\
		index += 2;\
	}
#endif


	void RGFW_createOpenGLContext(RGFW_window* win) {
#if defined(RGFW_LINK_EGL)
		eglInitializeSource = (PFNEGLINITIALIZEPROC) eglGetProcAddress("eglInitialize");
		eglGetConfigsSource = (PFNEGLGETCONFIGSPROC) eglGetProcAddress("eglGetConfigs");
		eglChooseConfigSource = (PFNEGLCHOOSECONFIGPROC) eglGetProcAddress("eglChooseConfig");
		eglCreateWindowSurfaceSource = (PFNEGLCREATEWINDOWSURFACEPROC) eglGetProcAddress("eglCreateWindowSurface");
		eglCreateContextSource = (PFNEGLCREATECONTEXTPROC) eglGetProcAddress("eglCreateContext");
		eglMakeCurrentSource = (PFNEGLMAKECURRENTPROC) eglGetProcAddress("eglMakeCurrent");
		eglGetDisplaySource = (PFNEGLGETDISPLAYPROC) eglGetProcAddress("eglGetDisplay");
		eglSwapBuffersSource = (PFNEGLSWAPBUFFERSPROC) eglGetProcAddress("eglSwapBuffers");
		eglSwapIntervalSource = (PFNEGLSWAPINTERVALPROC) eglGetProcAddress("eglSwapInterval");
		eglBindAPISource = (PFNEGLBINDAPIPROC) eglGetProcAddress("eglBindAPI");
		eglDestroyContextSource = (PFNEGLDESTROYCONTEXTPROC) eglGetProcAddress("eglDestroyContext");
		eglTerminateSource = (PFNEGLTERMINATEPROC) eglGetProcAddress("eglTerminate");
		eglDestroySurfaceSource = (PFNEGLDESTROYSURFACEPROC) eglGetProcAddress("eglDestroySurface");
#endif /* RGFW_LINK_EGL */

		win->src.EGL_display = eglGetDisplay((EGLNativeDisplayType) win->src.display);

		EGLint major, minor;

		eglInitialize(win->src.EGL_display, &major, &minor);

		#ifndef EGL_OPENGL_ES1_BIT
		#define EGL_OPENGL_ES1_BIT 0x1
		#endif

		EGLint egl_config[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE,
			#ifdef RGFW_OPENGL_ES1
			EGL_OPENGL_ES1_BIT,
			#elif defined(RGFW_OPENGL_ES3)
			EGL_OPENGL_ES3_BIT,
			#elif defined(RGFW_OPENGL_ES2)
			EGL_OPENGL_ES2_BIT,
			#else
			EGL_OPENGL_BIT,
			#endif
			EGL_NONE, EGL_NONE
		};

		EGLConfig config;
		EGLint numConfigs;
		eglChooseConfig(win->src.EGL_display, egl_config, &config, 1, &numConfigs);


		win->src.EGL_surface = eglCreateWindowSurface(win->src.EGL_display, config, (EGLNativeWindowType) win->src.window, NULL);

		EGLint attribs[] = {
			EGL_CONTEXT_CLIENT_VERSION,
			#ifdef RGFW_OPENGL_ES1
			1,
			#else
			2,
			#endif
			EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE
		};

		size_t index = 4;
		RGFW_GL_ADD_ATTRIB(EGL_STENCIL_SIZE, RGFW_STENCIL);
		RGFW_GL_ADD_ATTRIB(EGL_SAMPLES, RGFW_SAMPLES);

        if (RGFW_DOUBLE_BUFFER)
            RGFW_GL_ADD_ATTRIB(EGL_RENDER_BUFFER, EGL_BACK_BUFFER);

		if (RGFW_majorVersion) {
			attribs[1] = RGFW_majorVersion;
	
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MAJOR_VERSION, RGFW_majorVersion);
			RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_MINOR_VERSION, RGFW_minorVersion);

			if (RGFW_profile == RGFW_GL_CORE) {
				RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
			}
			else {
				RGFW_GL_ADD_ATTRIB(EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);
			}

		}

		#if defined(RGFW_OPENGL_ES1) || defined(RGFW_OPENGL_ES2) || defined(RGFW_OPENGL_ES3)
		eglBindAPI(EGL_OPENGL_ES_API);
		#else
		eglBindAPI(EGL_OPENGL_API);		
		#endif
      		
		win->src.EGL_context = eglCreateContext(win->src.EGL_display, config, EGL_NO_CONTEXT, attribs);
		
		if (win->src.EGL_context == NULL)
			fprintf(stderr, "failed to create an EGL opengl context\n");

		eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
		eglSwapBuffers(win->src.EGL_display, win->src.EGL_surface);
	}

	void RGFW_window_makeCurrent_OpenGL(RGFW_window* win) {
		eglMakeCurrent(win->src.EGL_display, win->src.EGL_surface, win->src.EGL_surface, win->src.EGL_context);
	}

	void* RGFW_getProcAddress(const char* procname) { 
		return (void*) eglGetProcAddress(procname); 
	}

	void RGFW_closeEGL(RGFW_window* win) {
		eglDestroySurface(win->src.EGL_display, win->src.EGL_surface);
		eglDestroyContext(win->src.EGL_display, win->src.EGL_context);

		eglTerminate(win->src.EGL_display);
	}
	
	void RGFW_window_swapInterval(RGFW_window* win, i32 swapInterval) {
		assert(win != NULL);
		
		eglSwapInterval(win->src.EGL_display, swapInterval);

	}
#endif /* RGFW_EGL */

/* 
	end of RGFW_EGL defines
*/

/* OPENGL Normal / EGL defines only (no OS MESA)  Ends here */

#elif defined(RGFW_OSMESA) /* OSmesa only */
RGFWDEF void RGFW_OSMesa_reorganize(void);

/* reorganize buffer for osmesa */
void RGFW_OSMesa_reorganize(void) {
	u8* row = (u8*) RGFW_MALLOC(win->r.w * 3);

	i32 half_height = win->r.h / 2;
	i32 stride = win->r.w * 3;

	i32 y;
	for (y = 0; y < half_height; ++y) {
		i32 top_offset = y * stride;
		i32 bottom_offset = (win->r.h - y - 1) * stride;
		memcpy(row, win->buffer + top_offset, stride);
		memcpy(win->buffer + top_offset, win->buffer + bottom_offset, stride);
		memcpy(win->buffer + bottom_offset, row, stride);
	}

	RGFW_FREE(row);
}
#endif /* RGFW_OSMesa */

#endif /* RGFW_GL (OpenGL, EGL, OSMesa )*/

/*
This is where OS specific stuff starts
*/


#if defined(RGFW_DRM)
	int RGFW_eventWait_forceStop[] = {0, 0, 0}; /* for wait events */

	#ifdef __linux__
		#include <linux/joystick.h>
		#include <fcntl.h>
		#include <unistd.h>
		
		RGFW_Event* RGFW_linux_updateJoystick(RGFW_window* win) {
			static int xAxis = 0, yAxis = 0;
			u8 i;
			for (i = 0; i < RGFW_joystickCount; i++) {
				struct js_event e;


				if (RGFW_joysticks[i] == 0)
					continue;

				i32 flags = fcntl(RGFW_joysticks[i], F_GETFL, 0);
				fcntl(RGFW_joysticks[i], F_SETFL, flags | O_NONBLOCK);

				ssize_t bytes;
				while ((bytes = read(RGFW_joysticks[i], &e, sizeof(e))) > 0) {
					switch (e.type) {
					case JS_EVENT_BUTTON:
						win->event.type = e.value ? RGFW_jsButtonPressed : RGFW_jsButtonReleased;
						win->event.button = e.number;
						RGFW_jsPressed[i][e.number] = e.value;
						RGFW_jsButtonCallback(win, i, e.number, e.value);
						return &win->event;
					case JS_EVENT_AXIS:
						ioctl(RGFW_joysticks[i], JSIOCGAXES, &win->event.axisesCount);

						if ((e.number == 0 || e.number % 2) && e.number != 1)
							xAxis = e.value;
						else
							yAxis = e.value;

						win->event.axis[e.number / 2].x = xAxis;
						win->event.axis[e.number / 2].y = yAxis;
						win->event.type = RGFW_jsAxisMove;
						win->event.joystick = i;
						RGFW_jsAxisCallback(win, i, win->event.axis, win->event.axisesCount);
						return &win->event;

						default: break;
					}
				}
			}

			return NULL;
		}

	#endif
#endif

/* linux libdrm defines*/
#if defined(RGFW_DRM)
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
	u16 RGFW_registerJoystickF(RGFW_window* win, char* file) {
		assert(win != NULL);

#ifdef __linux__

		i32 js = open(file, O_RDONLY);

		if (js && RGFW_joystickCount < 4) {
			RGFW_joystickCount++;

			RGFW_joysticks[RGFW_joystickCount - 1] = open(file, O_RDONLY);

			u8 i;
			for (i = 0; i < 16; i++)
				RGFW_jsPressed[RGFW_joystickCount - 1][i] = 0;

		}

		else {
#ifdef RGFW_PRINT_ERRORS
			RGFW_error = 1;
			fprintf(stderr, "Error RGFW_registerJoystickF : Cannot open file %s\n", file);
#endif
		}

		return RGFW_joystickCount - 1;
#endif
	}
	
	u16 RGFW_registerJoystick(RGFW_window* win, i32 jsNumber) {
		assert(win != NULL);

#ifdef __linux__
		char file[15];
		sprintf(file, "/dev/input/js%i", jsNumber);

		return RGFW_registerJoystickF(win, file);
#endif
	}
	
	void RGFW_stopCheckEvents(void) { 
		RGFW_eventWait_forceStop[2] = 1;
		while (1) {
			const char byte = 0;
			const ssize_t result = write(RGFW_eventWait_forceStop[1], &byte, 1);
			if (result == 1 || result == -1)
				break;
		}
	}

	void RGFW_window_eventWait(RGFW_window* win, i32 waitMS) {
		if (waitMS == 0)
			return;
		
		u8 i;

		if (RGFW_eventWait_forceStop[0] == 0 || RGFW_eventWait_forceStop[1] == 0) {
			if (pipe(RGFW_eventWait_forceStop) != -1) {
				fcntl(RGFW_eventWait_forceStop[0], F_GETFL, 0);
				fcntl(RGFW_eventWait_forceStop[0], F_GETFD, 0);
				fcntl(RGFW_eventWait_forceStop[1], F_GETFL, 0);
				fcntl(RGFW_eventWait_forceStop[1], F_GETFD, 0);
			}
		}

		struct pollfd fds[] = {
			{ RGFW_eventWait_forceStop[0], POLLIN, 0 },
			#ifdef __linux__ /* blank space for 4 joystick files*/
			{ -1, POLLIN, 0 }, {-1, POLLIN, 0 }, {-1, POLLIN, 0 },  {-1, POLLIN, 0} 
			#endif
		};

		u8 index = 2;
		
		#if defined(__linux__)
			for (i = 0; i < RGFW_joystickCount; i++) {
				if (RGFW_joysticks[i] == 0)
					continue;

				fds[index].fd = RGFW_joysticks[i];
				index++;
			}
		#endif


		u64 start = RGFW_getTimeNS();
		
		while (waitMS >= -1) {
			if (poll(fds, index, waitMS) <= 0)
				break;

			if (waitMS > 0) {
				waitMS -= (RGFW_getTimeNS() - start) / 1e+6;
			}
		}

		/* drain any data in the stop request */
		if (RGFW_eventWait_forceStop[2]) {	
			char data[64];
			(void)!read(RGFW_eventWait_forceStop[0], data, sizeof(data));
			
			RGFW_eventWait_forceStop[2] = 0;
		}
	}

	u64 RGFW_getTimeNS(void) { 
		struct timespec ts = { 0 };
		clock_gettime(1, &ts);
		unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

		return nanoSeconds;
	}

	u64 RGFW_getTime(void) {
		struct timespec ts = { 0 };
		clock_gettime(1, &ts);
		unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

		return (double)(nanoSeconds) * 1e-9;
	}
#endif /* end of libdrm time defines*/

/* unix (macOS, linux, web asm) only stuff */
#if defined(RGFW_DRM)
/* unix threading */
#ifndef RGFW_NO_THREADS
#include <pthread.h>

	RGFW_thread RGFW_createThread(RGFW_threadFunc_ptr ptr, void* args) {
		RGFW_UNUSED(args);
		
		RGFW_thread t;
		pthread_create((pthread_t*) &t, NULL, *ptr, NULL);
		return t;
	}
	void RGFW_cancelThread(RGFW_thread thread) { pthread_cancel((pthread_t) thread); }
	void RGFW_joinThread(RGFW_thread thread) { pthread_join((pthread_t) thread, NULL); }
#ifdef __linux__
	void RGFW_setThreadPriority(RGFW_thread thread, u8 priority) { pthread_setschedprio((pthread_t)thread, priority); }
#endif
#endif

#ifndef RGFW_WEBASM
/* unix sleep */
	void RGFW_sleep(u64 ms) {
		struct timespec time;
		time.tv_sec = 0;
		time.tv_nsec = ms * 1e+6;

		nanosleep(&time, NULL);
	}
#endif

#endif /* end of unix / mac stuff*/
#endif /*RGFW_IMPLEMENTATION*/

#if defined(__cplusplus) && !defined(__EMSCRIPTEN__)
}
#endif
