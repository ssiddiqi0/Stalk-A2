#ifndef _UDPSENDER_H
#define _UDPSENDER_H

void send_Init();
void send_waitForShutdown();
void send_Singaller();
void cancelSender_thread();

#endif
