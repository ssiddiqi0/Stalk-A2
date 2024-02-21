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
static pthread_mutex_t _syncOkToAScreenMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  screenCondMutex= PTHREAD_COND_INITIALIZER;
static List* list_receive;

void *screen_thread(){
   printf("------------------SCREEN-------------\n");
    char* message;
    while(1)
    {
        pthread_mutex_lock(&_syncOkToAScreenMutex);
        {
            pthread_cond_wait(&screenCondMutex, &_syncOkToAScreenMutex);
        }
        pthread_mutex_unlock(&_syncOkToAScreenMutex);
        //printf("----- screenOutput:  realesed mutx -----\n");
        // add condition to lock, wait, signal and test when error in list_prepend
        message = List_trim(list_receive);
        printf("Message received: ");
        puts(message);
        free(message);
        message = NULL;
    }
    return NULL;
}

void screen_Signaller(){
    pthread_mutex_lock(&_syncOkToAScreenMutex);
    {
        pthread_cond_signal(&screenCondMutex);
    }
    pthread_mutex_unlock(&_syncOkToAScreenMutex);
}

void screen_Init(List* list_s){
    list_receive = list_s;
    pthread_create(&screenWriter_thread_id, NULL, screen_thread, NULL);
}


void screen_waitForShutdown(){
    pthread_join(screenWriter_thread_id, NULL);
}
