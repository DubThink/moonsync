#ifndef _INTRO_H_
#define _INTRO_H_

#ifdef DEBUG
int  intro_init( HWND h);
#else
int  intro_init( void );
#endif

void intro_do(long time);

#endif
