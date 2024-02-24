#ifndef _KEYBOARDINPUT_H
#define _KEYBOARDINPUT_H

void keyboard_Init();
void cancelKeyboard_thread();
void keyboard_waitForShutdown();
void keyboardInput_Signaller();

#endif
