
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "screenOutput.h"
#include "udpReceive.h"
#include "udpSender.h"
#include "keyboardInput.h"
#include "list.h"

#define MAX_LEN 256
static List* list_receive; 

static pthread_cond_t syncOkToAddCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t syncOkToListAddMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t ListCriticalSectionMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t receiver_thread_id;
int receiverPort;  // local port passed from main to bind and receive msg

void* receiver_thread() {

    // refernce: https://beej.us/guide/bgnet/html/split/client-server-background.html#datagram 
    // refernce: Brian Fraser Assignment 2 videos for sockets

    struct sockaddr_in sinOtherSide;
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Receiver: socket");
    }
    
    memset(&sinOtherSide, 0, sizeof(sinOtherSide));
	sinOtherSide.sin_family = AF_INET;
	sinOtherSide.sin_addr.s_addr = htonl(INADDR_ANY);
	sinOtherSide.sin_port = htons(receiverPort);

	if (bind(sockfd, (struct sockaddr*) &sinOtherSide, sizeof(sinOtherSide)) == -1) {
        close(sockfd);
        perror("Bind: Error");	
	}
   
	while (1) {
        struct sockaddr_in their_addr; 
		socklen_t addr_len = sizeof(their_addr);
        char messageRx[MAX_LEN];
      
        while (1){
            int bytesRx = recvfrom(sockfd, messageRx, MAX_LEN-1, 0, (struct sockaddr*) &their_addr, &addr_len);

            // Null terminate the received message
            if (bytesRx >= 0) {
                messageRx[bytesRx] = '\0';         
            }

            // Allocate memory for the msg
            char* message = (char*)malloc(sizeof(messageRx));
			strncpy(message, messageRx, MAX_LEN); 

            int listAppendStatus;
            pthread_mutex_lock(&ListCriticalSectionMutex);
            {				
                // Mutex lock when accessing critical section
                listAppendStatus = List_append(list_receive, message); 
            }
            pthread_mutex_unlock(&ListCriticalSectionMutex);
            
            // If list append is successful it will signal screen there is new msg on list, if not successful wait for signal
            if (listAppendStatus == -1){
                pthread_mutex_lock(&syncOkToListAddMutex);
                {				
                    pthread_cond_wait(&syncOkToAddCondVar, &syncOkToListAddMutex);
                    List_append(list_receive, message);
                }
                pthread_mutex_unlock(&syncOkToListAddMutex);
            }
            else{
                screen_Signaller(); // Signal screen there is new msg to print on screen

                // If message is "!" cancel all the threads
                if (*(message) == '!'){
                   
                    cancelReceive_thread();
                    cancelScreen_thread();
                    cancelKeyboard_thread();
                    cancelSender_thread();
                    return NULL;
                }   
            }
            break;
        } 
        
    }
    close(sockfd);  
    return NULL;
}

void receive_ScreenSignaller() {
    pthread_mutex_lock(&syncOkToListAddMutex);
	{
		pthread_cond_signal(&syncOkToAddCondVar);
	}
	pthread_mutex_unlock(&syncOkToListAddMutex);
}
 
 
void udpReceive_Init(List* list_r, int Rport) {
    receiverPort = Rport;
    list_receive = list_r;
    pthread_create(&receiver_thread_id, NULL, receiver_thread, NULL);
}

// Cancel thread for terminating condition of "!"
void cancelReceive_thread(){

    pthread_cancel(receiver_thread_id);
}

void udpReceive_waitForShutdown() {
    pthread_join(receiver_thread_id, NULL);
    pthread_cond_destroy(&syncOkToAddCondVar);
    pthread_mutex_destroy(&syncOkToListAddMutex);
    pthread_mutex_destroy(&ListCriticalSectionMutex);
}
