// this code formating and concept is  taken form brain fraser's video of such as message use of messageRx, fgets,
// and the way declaring thread using mutex, init, signallers, waitforshutdown functions and encapsulating the code to keep main clean

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

#define MAX_LEN 256
static pthread_t keyboardInput_thread_id;

static pthread_mutex_t KeyboardMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t keyboardCondMutex = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t ListCriticalSectionMutex = PTHREAD_MUTEX_INITIALIZER;

List* list_send;
void* keyboardInput_thread(){    

    while (1){

        char* message;
        char messageRx[MAX_LEN];
        fgets(messageRx, MAX_LEN, stdin);

        while(1){
            char* message = (char*)malloc(strlen(messageRx)); 
			strncpy(message, messageRx, strlen(messageRx));
            
            int listAppendStatus; 
            pthread_mutex_lock(&ListCriticalSectionMutex);
            {				
                // Mutex lock when accessing critical section
                listAppendStatus = List_append(list_send, message); 
            }
            pthread_mutex_unlock(&ListCriticalSectionMutex);

            // If list append succes then signal the udpSender there is new msg on list
            // else if list append fail means no new space no list wait until recevies signal there is empty node to add
            if (listAppendStatus == -1){
                pthread_mutex_lock(&KeyboardMutex);
                {				
                    pthread_cond_wait(&keyboardCondMutex, &KeyboardMutex);
                }
                pthread_mutex_unlock(&KeyboardMutex);
                List_append(list_send, message);
            }
            else{
                // Signal if udp_rececive new node available if it is waiting to add new msg on list if full 
                send_Singaller();
                // User input message is "!" in single line then pthread cancel all the thread To interrupt a running thread
                if (*(message) == '!'){
                    cancelReceive_thread();
                    cancelScreen_thread();
                    cancelKeyboard_thread();
                    cancelSender_thread();
                    return NULL;
                }
                break;
            }
            free(message);
        }
    } 
    return NULL;
}

void keyboardInput_Signaller(){
    pthread_mutex_lock(&KeyboardMutex);
    {
        pthread_cond_signal(&keyboardCondMutex);
    }
    pthread_mutex_unlock(&KeyboardMutex);
}

void keyboard_Init(List* list_s){
    list_send = list_s;
    pthread_create(&keyboardInput_thread_id, NULL, keyboardInput_thread, NULL);
}

// Cancel thread for terminating condition of "!"
void cancelKeyboard_thread(){

    pthread_cancel(keyboardInput_thread_id);
}
void keyboard_waitForShutdown(){
     // Allow to wait for other threads to join 
    pthread_join(keyboardInput_thread_id, NULL);

     // Destroy mutex after joining to make sure no other thread is using mutex which can cause deadlock
    pthread_mutex_destroy(&KeyboardMutex);
    pthread_cond_destroy(&keyboardCondMutex);
    pthread_mutex_destroy(&ListCriticalSectionMutex);
}