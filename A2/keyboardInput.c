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

#define MSG_MAX_LEN 2000
static pthread_t keyboardInput_thread_id;

static pthread_mutex_t _syncOkToAKeyboardMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t keyboardCondMutex = PTHREAD_COND_INITIALIZER;

List* list_send;

void *keyboardInput_thread(){    
    printf("------------------KEYBOARD-------------\n");
    while (1){
        char* message;
        char messageRx[MSG_MAX_LEN];
        fgets(messageRx, MSG_MAX_LEN, stdin);
        //printf("string from user is : %s\n", messageRx);
        while(1){
            char* message = (char*)malloc(strlen(messageRx));  // or len(messageRx)
			strncpy(message, messageRx, strlen(messageRx));
            int listApmessagependStatus = List_append(list_send, message); 
            if (listApmessagependStatus == -1){
                pthread_mutex_lock(&_syncOkToAKeyboardMutex);
                {				
                    pthread_cond_wait(&keyboardCondMutex, &_syncOkToAKeyboardMutex);
                }
                pthread_mutex_unlock(&_syncOkToAKeyboardMutex);
                List_append(list_send, message);
            }else{
                //printf("KeyBoard: new msg called : send_Singaller\n");
                send_Singaller();
                break;
            }
            //receive_ScreenSignaller(); 
            // signal if udp_rececive new node available if it is waiting to add new msg on list if full 
            free(message);
            message = NULL;
        }
    } 
    return NULL;
}

void keyboardInput_Signaller(){
    pthread_mutex_lock(&_syncOkToAKeyboardMutex);
    {
        pthread_cond_signal(&keyboardCondMutex);
    }
    pthread_mutex_unlock(&_syncOkToAKeyboardMutex);
}

void keyboard_Init(List* list_s){
    list_send = list_s;
    pthread_create(&keyboardInput_thread_id, NULL, keyboardInput_thread, NULL);
}


void keyboard_waitForShutdown(){
    pthread_join(keyboardInput_thread_id, NULL);
}