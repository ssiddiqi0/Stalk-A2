// screen output
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "screenOutput.h"
#include "udpReceive.h"
#include "udpSender.h"
#include "keyboardInput.h"
#include "list.h"

static pthread_t screenWriter_thread_id;
static pthread_mutex_t syncOkToAScreenMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  screenCondMutex= PTHREAD_COND_INITIALIZER;
static List* list_receive;
char* message;

static pthread_mutex_t ListCriticalSectionMutex = PTHREAD_MUTEX_INITIALIZER;

void* screen_thread(){
    while(1)
    {
        // add condition to lock then wait for signal from udpReceiver if there is new msg on list
        pthread_mutex_lock(&syncOkToAScreenMutex);
        {
            pthread_cond_wait(&screenCondMutex, &syncOkToAScreenMutex);
        }
        pthread_mutex_unlock(&syncOkToAScreenMutex);
        
        // Critical section mutex
        pthread_mutex_lock(&ListCriticalSectionMutex);
        {
           message = List_trim(list_receive);
        }
        pthread_mutex_unlock(&ListCriticalSectionMutex);
        
        if (*(message) == '!'){
            //message == ! free the already alloacted memory before join
            freemsg();
            return NULL;
        }
        printf("Message received: ");
        puts(message);
        // free allocted memory of msg after use
        freemsg();
    }
    return NULL;
}
void freemsg(){
    free(message);
    message = NULL;
}
void screen_Signaller(){
    pthread_mutex_lock(&syncOkToAScreenMutex);
    {
        pthread_cond_signal(&screenCondMutex);
    }
    pthread_mutex_unlock(&syncOkToAScreenMutex);
}

void screen_Init(List* list_s){
    list_receive = list_s;
    pthread_create(&screenWriter_thread_id, NULL, screen_thread, NULL);
}

// cancel thread for terminating condition of "!"
void cancelScreen_thread(){
   // printf("ScreenOutput Thread cancelled\n");
    pthread_cancel(screenWriter_thread_id);
}

void screen_waitForShutdown(){
    if(message!=NULL){
        freemsg();
    }
    // allow to wait for other threads to join 
    pthread_join(screenWriter_thread_id, NULL);
    pthread_mutex_destroy(&syncOkToAScreenMutex);
    pthread_cond_destroy(&screenCondMutex);
    pthread_mutex_destroy(&ListCriticalSectionMutex);
}

