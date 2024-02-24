#ifndef _UDPRECEIVE_H
#define _UDPRECEIVE_H

void udpReceive_Init();
void cancelReceive_thread();
void receive_ScreenSignaller();
void udpReceive_waitForShutdown();


#endif
