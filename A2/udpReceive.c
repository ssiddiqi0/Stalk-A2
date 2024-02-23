
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

#define MSG_MAX_LEN 2000
static List* list_receive; 

static pthread_cond_t _syncOkToPrintCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t _syncOkToListMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t _syncOkToAddCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t _syncOkToListAddMutex = PTHREAD_MUTEX_INITIALIZER;
extern volatile int shutDown;
pthread_t receiver_thread_id;
int receiverPort;
struct sockaddr_in sinRemote;

void *receiver_thread() {

   printf("------------------RECEIVE-------------\n");
   
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Receiver: socket");
    }
    memset(&sinRemote, 0, sizeof(sinRemote));
	sinRemote.sin_family = AF_INET;
	sinRemote.sin_addr.s_addr = htonl(INADDR_ANY);
	sinRemote.sin_port = htons(receiverPort);

	if (bind(sockfd, (struct sockaddr*) &sinRemote, sizeof(sinRemote)) < 0) {
		//printf("ERROR: Socket could not be bound\nExiting program\n");
        perror("server: bind");
		close(sockfd);
	}
   
	while (1) {
        struct sockaddr_in sinN; // Address of sender
		unsigned int sin_len = sizeof(sinN);
        char messageRx[MSG_MAX_LEN];
      
        while (1){
            int bytesRx = recvfrom(sockfd, messageRx, MSG_MAX_LEN, 0, (struct sockaddr*) &sinN, &sin_len);
            if (bytesRx >= 0) {
                messageRx[bytesRx] = '\0'; // Null terminate the received message
                //printf("Message received (%d bytes): '%s'\n", bytesRx, messageRx);  

                if (strcmp(messageRx, "!\n") == 0) {
                printf("Shutdown signal received. Exiting...\n");
                 close(sockfd); // Close the socket to clean up resources
				// free(messageRx); 
			    //messageRx = NULL;
                udpReceive_waitForShutdown();
			
			
                exit(0);
               
            }      
            }
            char* message = (char*)malloc(sizeof(messageRx)); // or len(messageRx)
			strncpy(message, messageRx, MSG_MAX_LEN);  // we could also add extra parameter for no of byte
            int listAppendStatus = List_append(list_receive, message); // or may be list_remove method
            if (listAppendStatus == -1){
                pthread_mutex_lock(&_syncOkToListAddMutex);
                {				
                    pthread_cond_wait(&_syncOkToAddCondVar, &_syncOkToListAddMutex);
                    List_append(list_receive, message);
                }
                pthread_mutex_unlock(&_syncOkToListAddMutex);
            }
            else{
                screen_Signaller(); // sigal screen there is new msg to print on screen
            }
            // free(message);
            // message = NULL;
        } 
        
        //close(sockfd);   // not sure where this will go
    }
    return NULL;
}

// wait for signal from sender, signal to open socket and receieve msg
void receive_Signaller() {
    pthread_mutex_lock(&_syncOkToListMutex);
	{
		pthread_cond_signal(&_syncOkToPrintCondVar);
	}
	pthread_mutex_unlock(&_syncOkToListMutex);
}

void receive_ScreenSignaller() {
    pthread_mutex_lock(&_syncOkToListAddMutex);
	{
		pthread_cond_signal(&_syncOkToAddCondVar);
	}
	pthread_mutex_unlock(&_syncOkToListAddMutex);
}
 
 
void udpReceive_Init(List* list_r, int Rport) {
    receiverPort = Rport;
    list_receive = list_r;
    pthread_create(&receiver_thread_id, NULL, receiver_thread, NULL);
}

void udpReceive_waitForShutdown() {
    pthread_join(receiver_thread_id, NULL);
}
