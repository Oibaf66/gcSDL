/***********************************************************************
 * SDL_cubevideo.c
 *
 * cubeSDL - SDL library 1.2 for GameCube
 * by infact <infact [at] quantentunnel [dot] de>
 * Source is based on code by Tantric, Softdev and others
 **********************************************************************/

#include "SDL_config.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_cubevideo.h"
#include "SDL_cubeevents_c.h"
#include "SDL_cubemouse_c.h"

#include "SDL_thread.h"

#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <ogc/texconv.h>

#define CUBEVID_DRIVER_NAME "cube"

/* supported SDL VideoModes */
static SDL_Rect mode_320;
static SDL_Rect mode_640;

static SDL_Rect* modes_descending[] =
{
	&mode_640,
	&mode_320,
	NULL
};

/* GC Video variables */

// Double buffered framebuffer
unsigned int *xfb[2] = { NULL, NULL };
int whichfb = 0; // Switch
GXRModeObj* vmode = 0;

/* Threaded Video */
static lwp_t videothread = LWP_THREAD_NULL;
static SDL_mutex * videomutex = 0;
u8 * screenTex = NULL; // screen capture
static int quit_flip_thread = 0;

/* GX/Scaler support variables */

#define HASPECT 320
#define VASPECT 240
#define TEXTUREMEM_SIZE (640*480*4)
#define DEFAULT_FIFO_SIZE 256 * 1024

static unsigned char texturemem[TEXTUREMEM_SIZE] ATTRIBUTE_ALIGN (32); // GX texture
static unsigned char textureconvert[TEXTUREMEM_SIZE] ATTRIBUTE_ALIGN (32); // 565 mem
static unsigned char gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static GXTexObj texobj;
static Mtx view;

typedef struct tagcamera {
	guVector pos;
	guVector up;
	guVector view;
} camera;


/* Square Matrix
 * This structure controls the size of the image on the screen.
 * Think of the output as a -80 x 80 by -60 x 60 graph.
 */
static s16 square[] ATTRIBUTE_ALIGN (32) = {
	/*
	 *     X,        Y, Z
	 * Values set are for roughly 4:3 aspect
	 */
	-HASPECT,  VASPECT, 0,	// 0
	 HASPECT,  VASPECT, 0,	// 1
	 HASPECT, -VASPECT, 0,	// 2
	-HASPECT, -VASPECT, 0	// 3
};

static camera cam = {
	{0.0F, 0.0F, 0.0F},
	{0.0F, 0.5F, 0.0F},
	{0.0F, 0.0F, -0.5F}
};

static int currentwidth, currentheight, currentbpp;

/* Initialization/Query functions */
static int CUBE_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **CUBE_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *CUBE_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int CUBE_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void CUBE_VideoQuit(_THIS);

/* Hardware surface functions */
static int CUBE_AllocHWSurface(_THIS, SDL_Surface *surface);
static int CUBE_LockHWSurface(_THIS, SDL_Surface *surface);
static void CUBE_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void CUBE_FreeHWSurface(_THIS, SDL_Surface *surface);
static int CUBE_FlipHWSurface(_THIS, SDL_Surface *surface);

/* etc. */
static void CUBE_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* GX/Scaler support functions */
static void SetupGX();
static void draw_init();

/* Video threading support functions */
void CUBE_VideoStart();
void CUBE_VideoStop();
static void StartVideoThread();
static void * flip_thread (void *arg);

/* Rect update support functions (for different bit depths) */
static void CUBE_UpdateRect(_THIS, SDL_Rect *rect);
static void UpdateRect_8(_THIS, SDL_Rect *rect);
static void UpdateRect_16(_THIS, SDL_Rect *rect);
static void UpdateRect_24(_THIS, SDL_Rect *rect);
static void UpdateRect_32(_THIS, SDL_Rect *rect);
static inline void Set_RGBAPixel(_THIS, int x, int y, u32 color);
static inline void Set_RGB565Pixel(_THIS, int x, int y, u16 color);

/* Surface flipping support functions */
static void flipHWSurface_8_16(_THIS, SDL_Surface *surface);
static void flipHWSurface_16_16(_THIS, SDL_Surface *surface);
static void flipHWSurface_24_16(_THIS, SDL_Surface *surface);
static void flipHWSurface_32_16(_THIS, SDL_Surface *surface);

/* CUBE driver bootstrap functions */

static int CUBE_Available(void)
{
	// always available
	return(1);
}

static void CUBE_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *CUBE_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
		}
		return(0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	// for threaded video
	videomutex = SDL_CreateMutex();

	/* Set the function pointers */
	device->VideoInit = CUBE_VideoInit;
	device->ListModes = CUBE_ListModes;
	device->SetVideoMode = CUBE_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = CUBE_SetColors;
	device->UpdateRects = CUBE_UpdateRects;
	device->VideoQuit = CUBE_VideoQuit;
	device->AllocHWSurface = CUBE_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = CUBE_LockHWSurface;
	device->UnlockHWSurface = CUBE_UnlockHWSurface;
	device->FlipHWSurface = CUBE_FlipHWSurface;
	device->FreeHWSurface = CUBE_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = CUBE_InitOSKeymap;
	device->PumpEvents = CUBE_PumpEvents;

	device->free = CUBE_DeleteDevice;

	return device;
}

VideoBootStrap CUBE_bootstrap = {
	CUBEVID_DRIVER_NAME, "cube video driver",
	CUBE_Available, CUBE_CreateDevice
};

void CUBE_InitVideoSystem()
{
	/* Initialise libogc's video system */
	VIDEO_Init();

	// Get current mode and use it
	vmode = VIDEO_GetPreferredMode(NULL);
	//FABIO
	VIDEO_Configure(vmode);

	/* Configure the framebuffers (double-buffering) */
	xfb[0] = (u32 *) SYS_AllocateFramebuffer (vmode);
	xfb[1] = (u32 *) SYS_AllocateFramebuffer (vmode);
	DCInvalidateRange(xfb[0], VIDEO_GetFrameBufferSize(vmode));
	DCInvalidateRange(xfb[1], VIDEO_GetFrameBufferSize(vmode));
	xfb[0] = (u32 *) MEM_K0_TO_K1 (xfb[0]);
	xfb[1] = (u32 *) MEM_K0_TO_K1 (xfb[1]);
	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);
	VIDEO_SetNextFramebuffer (xfb[0]);

	// Clear and show the screen.
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();

	console_init(xfb[0],20,64,vmode->fbWidth,vmode->xfbHeight,vmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Clear out FIFO area
	memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

	// Initialise GX
	GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
	GXColor background = { 0, 0, 0, 0xff };
	GX_SetCopyClear (background, 0x00ffffff);
	SetupGX();
}

int CUBE_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	// Set up the modes.
	mode_640.w = vmode->fbWidth;
	mode_640.h = vmode->efbHeight;  //FABIO
	mode_320.w = mode_640.w / 2;
	mode_320.h = mode_640.h / 2;

	// Set the current format (16 bit). 
	vformat->BitsPerPixel = 16;
	vformat->BytesPerPixel = 2;

	this->hidden->buffer = NULL;
	this->hidden->width = 0;
	this->hidden->height = 0;
	this->hidden->pitch = 0;

	// VideoInit done!
	return(0);
}

SDL_Rect **CUBE_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	return &modes_descending[0];
}

SDL_Surface *CUBE_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	SDL_Rect* mode;

	// Find a mode big enough to store the requested resolution
	mode = modes_descending[0];
	while (mode)
	{
		if (mode->w == width && mode->h == height)
			break;
		else
			++mode;
	}

	// Didn't find a mode?
	if (!mode)
	{
		SDL_SetError("Display mode (%dx%d) is unsupported.",
			width, height);
		return NULL;
	}

	// Not supported bit depth
	if(bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
	{
		SDL_SetError("Resolution (%d bpp) is unsupported (8/16/24/32 bpp only).",
			bpp);
		return NULL;
	}

	// Free existing buffer.
	if ( this->hidden->buffer ) {
		SDL_free( this->hidden->buffer );
	}

	// Allocate the new buffer.
	//this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	this->hidden->buffer = memalign(32, width * height * (bpp / 8));
	if ( ! this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}

	// Clear the buffer
	SDL_memset(this->hidden->buffer, 0, width * height * (bpp / 8));

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	/* Set up the new mode framebuffer */
	// we have a double buffered fullscreen framebuffer with a hardware palette
	current->flags = (flags & SDL_DOUBLEBUF) | (flags & SDL_FULLSCREEN)
						| (flags & SDL_HWPALETTE);

	this->hidden->width = current->w = width;
	this->hidden->height = current->h = height;
	this->hidden->pitch = current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;

	///FIXME: maybe get rid of this variables
	currentwidth = current->w;
	currentheight = current->h;
	currentbpp = bpp;

	draw_init();
	StartVideoThread();

	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int CUBE_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}

static void CUBE_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int CUBE_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void CUBE_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static int CUBE_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	switch(surface->format->BytesPerPixel)
	{
		case 1:
			flipHWSurface_8_16(this, surface);
			break;
		case 2:
			flipHWSurface_16_16(this, surface);
			break;
		case 3:
			flipHWSurface_24_16(this, surface);
			break;
		case 4:
			flipHWSurface_32_16(this, surface);
			break;
		default:
			return -1;
	}
	return 1;
}

static void CUBE_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	// update all given rects
	int i;
	for (i = 0; i < numrects; i++)
	{
		CUBE_UpdateRect(this, &rects[i]);
	}
}

int CUBE_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	const int last_color = firstcolor + ncolors;
	Uint16* const palette = this->hidden->palette;
	int component;

	/* Build the RGB565 palette. */
	for (component = firstcolor; component != last_color; ++component)
	{
		const SDL_Color* const in = &colors[component - firstcolor];
		const unsigned int r = (in->r >> 3) & 0x1f;
		const unsigned int g = (in->g >> 2) & 0x3f;
		const unsigned int b = (in->b >> 3) & 0x1f;

		palette[component] = (r << 11) | (g << 5) | b;
	}

	return(1);
}

void CUBE_VideoQuit(_THIS)
{
	// Shutdown GX hardware and clear framebuffer
	CUBE_VideoStop();
	GX_AbortFrame();
	GX_Flush();

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
}

/* GX/Scaler support functions */

static void SetupGX()
{
	Mtx44 p;
	int df = 1; // deflicker on/off

	GX_SetViewport (0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
	GX_SetDispCopyYScale ((f32) vmode->xfbHeight / (f32) vmode->efbHeight);
	GX_SetScissor (0, 0, vmode->fbWidth, vmode->efbHeight);

	GX_SetDispCopySrc (0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopyDst (vmode->fbWidth, vmode->xfbHeight);
	GX_SetCopyFilter (vmode->aa, vmode->sample_pattern, (df == 1) ? GX_TRUE : GX_FALSE, vmode->vfilter);

	GX_SetFieldMode (vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	GX_SetPixelFmt (GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	GX_SetDispCopyGamma (GX_GM_1_0);
	GX_SetCullMode (GX_CULL_NONE);
	GX_SetBlendMode(GX_BM_BLEND,GX_BL_DSTALPHA,GX_BL_INVSRCALPHA,GX_LO_CLEAR);

	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate (GX_TRUE);
	GX_SetNumChans(1);

	// matrix,     t,        b,        l,     r,   n,    f
	guOrtho(p, 480/2, -(480/2), -(640/2), 640/2, 100, 1000);
	GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);
}

static void draw_init()
{
	GX_ClearVtxDesc ();
	GX_SetVtxDesc (GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc (GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_SetArray (GX_VA_POS, square, 3 * sizeof (s16));

	GX_SetNumTexGens (1);
	GX_SetNumChans (0);

	GX_SetTexCoordGen (GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);

	memset (&view, 0, sizeof (Mtx));
	guLookAt(view, &cam.pos, &cam.up, &cam.view);
	GX_LoadPosMtxImm (view, GX_PNMTX0);

	GX_InvVtxCache (); // update vertex cache

	// initialize the texture obj depending on bit depth
	if (currentbpp == 8 || currentbpp == 16)
		GX_InitTexObj (&texobj, texturemem, currentwidth, currentheight, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
	else
		GX_InitTexObj (&texobj, texturemem, currentwidth, currentheight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);

	// load texture object so it's ready to use
	GX_LoadTexObj (&texobj, GX_TEXMAP0);
}

static inline void draw_vert (u8 pos, u8 c, f32 s, f32 t)
{
	GX_Position1x8 (pos);
	GX_Color1x8 (c);
	GX_TexCoord2f32 (s, t);
}

static inline void draw_square (Mtx v)
{
	Mtx m; // model matrix.
	Mtx mv; // modelview matrix.

	guMtxIdentity (m);
	guMtxTransApply (m, m, 0, 0, -100);
	guMtxConcat (v, m, mv);

	GX_LoadPosMtxImm (mv, GX_PNMTX0);

	GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
	draw_vert (0, 0, 0.0, 0.0);
	draw_vert (1, 0, 1.0, 0.0);
	draw_vert (2, 0, 1.0, 1.0);
	draw_vert (3, 0, 0.0, 1.0);
	GX_End ();
}

// Copies the current screen into a GX texture
static void TakeScreenshot()
{
	int texSize = vmode->fbWidth * vmode->efbHeight * 4;

	if(screenTex)
		free(screenTex);

	screenTex = (u8 *)memalign(32, texSize);

	if(screenTex == NULL)
		return;

	GX_SetTexCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetTexCopyDst(vmode->fbWidth, vmode->efbHeight, GX_TF_RGBA8, GX_FALSE);
	GX_CopyTex(screenTex, GX_FALSE);
	GX_PixModeSync();

	DCFlushRange(screenTex, texSize);
}

/* Video threading support functions */

void CUBE_VideoStart()
{
	SetupGX();
	draw_init();
	StartVideoThread();
}

void CUBE_VideoStop()
{
	quit_flip_thread = 1;
	if(videothread == LWP_THREAD_NULL) return;
	LWP_JoinThread(videothread, NULL);
	videothread = LWP_THREAD_NULL;
}

static void
StartVideoThread()
{
	if(videothread == LWP_THREAD_NULL)
	{
		quit_flip_thread = 0;
		LWP_CreateThread (&videothread, flip_thread, NULL, NULL, 0, 68);
	}
}

static void * flip_thread (void *arg)
{
	while(1)
	{
		if(quit_flip_thread == 2)
			break;

		// clear texture objects
		GX_InvVtxCache();
		GX_InvalidateTexAll();

		SDL_mutexP(videomutex);

		// load texture into GX
		DCFlushRange(texturemem, TEXTUREMEM_SIZE);

		GX_LoadTexObj(&texobj, GX_TEXMAP0);

		// render textured quad
		draw_square(view);
		GX_SetColorUpdate(GX_TRUE);

		if (quit_flip_thread == 1)
		{
			quit_flip_thread = 2;
			TakeScreenshot();
		}

		whichfb ^= 1;

		GX_CopyDisp(xfb[whichfb], GX_TRUE);
		GX_DrawDone();

		SDL_mutexV(videomutex);

		VIDEO_SetNextFramebuffer(xfb[whichfb]);
		VIDEO_Flush();
		VIDEO_WaitVSync();
	}

	return NULL;
}

/* Rect update support functions (for different bit depths) */

static void CUBE_UpdateRect(_THIS, SDL_Rect *rect)
{
	// update the given rect with correct bit depth
	const SDL_Surface* const screen = this->screen;
	SDL_mutexP(videomutex);

	switch(screen->format->BytesPerPixel)
	{
		case 1:
			UpdateRect_8(this, rect);
			break;
		case 2:
			UpdateRect_16(this, rect);
			break;
		case 3:
			UpdateRect_24(this, rect);
			break;
		case 4:
			UpdateRect_32(this, rect);
			break;
		default:
			fprintf(stderr, "Invalid BPP %d\n", screen->format->BytesPerPixel);
		break;
	}

	SDL_mutexV(videomutex);
}

static void UpdateRect_8(_THIS, SDL_Rect *rect)
{
	u8 *src;
	u8 *ptr;
	u16 color;
	int i, j;
	Uint16 *palette = this->hidden->palette;

	for (i = 0; i < rect->h; i++)
	{
		src = (this->hidden->buffer + (this->hidden->width * (i + rect->y)) + (rect->x));
		for (j = 0; j < rect->w; j++)
		{
			ptr = src + j;
			color = palette[*ptr];
			Set_RGB565Pixel(this, rect->x + j, rect->y + i, color);
		}
	}
}

static void UpdateRect_16(_THIS, SDL_Rect *rect)
{
	u8 *src;
	u8 *ptr;
	u16 color;
	int i, j;
	for (i = 0; i < rect->h; i++)
	{
		src = (this->hidden->buffer + (this->hidden->width * 2 * (i + rect->y)) + (rect->x * 2));
		for (j = 0; j < rect->w; j++)
		{
			ptr = src + (j * 2);
			color = (ptr[0] << 8) | ptr[1];
			Set_RGB565Pixel(this, rect->x + j, rect->y + i, color);
		}
	}
}

static void UpdateRect_24(_THIS, SDL_Rect *rect)
{
	u8 *src;
	u8 *ptr;
	u32 color;
	int i, j;
	for (i = 0; i < rect->h; i++)
	{
		src = (this->hidden->buffer + (this->hidden->width * 3 * (i + rect->y)) + (rect->x * 3));
		for (j = 0; j < rect->w; j++)
		{
			ptr = src + (j * 3);
			color = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | 0xff;
			Set_RGBAPixel(this, rect->x + j, rect->y + i, color);
		}
	}
}

static void UpdateRect_32(_THIS, SDL_Rect *rect)
{
	u8 *src;
	u8 *ptr;
	u32 color;
	int i, j;
	for (i = 0; i < rect->h; i++)
	{
		src = (this->hidden->buffer + (this->hidden->width * 4 * (i + rect->y)) + (rect->x * 4));
		for (j = 0; j < rect->w; j++)
		{
			ptr = src + (j * 4);
			color = (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | ptr[0];
			Set_RGBAPixel(this, rect->x + j, rect->y + i, color);
		}
	}
}

static inline void Set_RGBAPixel(_THIS, int x, int y, u32 color)
{
	u8 *truc = (u8*) texturemem;
	int width = this->hidden->width;
	u32 offset;

	offset = (((y >> 2) << 4) * width) + ((x >> 2) << 6) + (((y % 4 << 2) + x % 4) << 1);

	*(truc + offset) = color & 0xFF;
	*(truc + offset + 1) = (color >> 24) & 0xFF;
	*(truc + offset + 32) = (color >> 16) & 0xFF;
	*(truc + offset + 33) = (color >> 8) & 0xFF;
}

static inline void Set_RGB565Pixel(_THIS, int x, int y, u16 color)
{
	u8 *truc = (u8*) texturemem;
	int width = this->hidden->width;
	u32 offset;

	offset = (((y >> 2) << 3) * width) + ((x >> 2) << 5) + (((y % 4 << 2) + x % 4) << 1);

	*(truc + offset) = (color >> 8) & 0xFF;
	*(truc + offset + 1) = color & 0xFF;
}

/* Surface flipping support functions */

static void flipHWSurface_8_16(_THIS, SDL_Surface *surface)
{
	int new_pitch = this->hidden->width * 2;
	long long int *dst = (long long int *) texturemem;
	long long int *src1 = (long long int *) textureconvert;
	long long int *src2 = (long long int *) (textureconvert + new_pitch);
	long long int *src3 = (long long int *) (textureconvert + (new_pitch * 2));
	long long int *src4 = (long long int *) (textureconvert + (new_pitch * 3));
	int rowpitch = (new_pitch >> 3) * 3;
	int rowadjust = (new_pitch % 8) * 4;
	Uint16 *palette = this->hidden->palette;
	char *ra = NULL;
	int h, w;

	// Crude convert
	Uint16 * ptr_cv = (Uint16 *) textureconvert;
	Uint8 *ptr = (Uint8 *)this->hidden->buffer;

	for (h = 0; h < this->hidden->height; h++)
	{
		for (w = 0; w < this->hidden->width; w++)
		{
			Uint16 v = palette[*ptr];

			*ptr_cv++ = v;
			ptr++;
		}
	}

	// Same as 16bit
	for (h = 0; h < this->hidden->height; h += 4)
	{
		for (w = 0; w < (this->hidden->width >> 2); w++)
		{
			*dst++ = *src1++;
			*dst++ = *src2++;
			*dst++ = *src3++;
			*dst++ = *src4++;
		}

		src1 += rowpitch;
		src2 += rowpitch;
		src3 += rowpitch;
		src4 += rowpitch;

		if ( rowadjust )
		{
			ra = (char *)src1;
			src1 = (long long int *)(ra + rowadjust);
			ra = (char *)src2;
			src2 = (long long int *)(ra + rowadjust);
			ra = (char *)src3;
			src3 = (long long int *)(ra + rowadjust);
			ra = (char *)src4;
			src4 = (long long int *)(ra + rowadjust);
		}
	}
}

static void flipHWSurface_16_16(_THIS, SDL_Surface *surface)
{
	int h, w;
	long long int *dst = (long long int *) texturemem;
	long long int *src1 = (long long int *) this->hidden->buffer;
	long long int *src2 = (long long int *) (this->hidden->buffer + this->hidden->pitch);
	long long int *src3 = (long long int *) (this->hidden->buffer + (this->hidden->pitch * 2));
	long long int *src4 = (long long int *) (this->hidden->buffer + (this->hidden->pitch * 3));
	int rowpitch = (this->hidden->pitch >> 3) * 3;
	int rowadjust = (this->hidden->pitch % 8) * 4;
	char *ra = NULL;

	for (h = 0; h < this->hidden->height; h += 4)
	{
		for (w = 0; w < this->hidden->width; w += 4)
		{
			*dst++ = *src1++;
			*dst++ = *src2++;
			*dst++ = *src3++;
			*dst++ = *src4++;
		}

		src1 += rowpitch;
		src2 += rowpitch;
		src3 += rowpitch;
		src4 += rowpitch;

		if ( rowadjust )
		{
			ra = (char *)src1;
			src1 = (long long int *)(ra + rowadjust);
			ra = (char *)src2;
			src2 = (long long int *)(ra + rowadjust);
			ra = (char *)src3;
			src3 = (long long int *)(ra + rowadjust);
			ra = (char *)src4;
			src4 = (long long int *)(ra + rowadjust);
		}
	}
}

static void flipHWSurface_24_16(_THIS, SDL_Surface *surface)
{
	SDL_Rect screen_rect = {0, 0, this->hidden->width, this->hidden->height};
	CUBE_UpdateRect(this, &screen_rect);
}

static void flipHWSurface_32_16(_THIS, SDL_Surface *surface)
{
	SDL_Rect screen_rect = {0, 0, this->hidden->width, this->hidden->height};
	CUBE_UpdateRect(this, &screen_rect);
}

/* Other support functions */

// Reset the scaling for widescreen output
void CUBE_SetWidescreen(int wide)
{
	if(wide)
	{
		vmode->viWidth = 678;
		vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 678) / 2;
	}
	else
	{
		vmode->viWidth = 640;
		vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 640) / 2;
	}
	VIDEO_Configure (vmode);
	VIDEO_Flush();
	
	VIDEO_WaitVSync ();

	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while (VIDEO_GetNextField())
			VIDEO_WaitVSync();
}

// Change the image stretching
void CUBE_ChangeSquare(int xscale, int yscale, int xshift, int yshift)
{
	square[6] = square[3]  =  xscale + xshift;
	square[0] = square[9]  = -xscale + xshift;
	square[4] = square[1]  =  yscale - yshift;
	square[7] = square[10] = -yscale - yshift;

	// update memory BEFORE the GPU accesses it!
	DCFlushRange (square, 32);
}
