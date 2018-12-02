#ifndef _CONFIG_H_
#define _CONFIG_H_

//#define CLEANDESTROY          // destroy stuff (windows, glContext, ...)
#ifdef DEBUG
#define XRES 1900
#define YRES 1000
#else
#define XRES 1920
#define YRES 1080

#define FULLSCREEN

#endif


#define MOUSE_SENSE 0.1
//#define SOUND_DISABLED

/*
If USE_SOUND_THREAD is defined, 4klang plays music in realtime.
Otherwise music is prerendered and opening the intro takes longer.
*/
#define USE_SOUND_THREAD

#endif