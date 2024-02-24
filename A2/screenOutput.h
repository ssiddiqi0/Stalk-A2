#ifndef _SCREENOUTPUT_H
#define _SCREENOUTPUT_H

void screen_Init();
void screen_Signaller();
void cancelScreen_thread();
void screen_waitForShutdown();
void freemsg();

#endif
