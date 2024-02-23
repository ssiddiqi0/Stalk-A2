
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "screenOutput.h"
#include "udpReceive.h"
#include "udpSender.h"
#include "keyboardInput.h"
#include "list.h"
#define MSG_MAX_LEN 2000

static pthread_cond_t _syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t _syncOkToSendMutex = PTHREAD_MUTEX_INITIALIZER;

struct addrinfo* sinR; 
pthread_t send_thread_id;
int senderPort;
static List* list_send; 
extern volatile int shutDown;
void *sender_thread() {
	printf("------------------SENDER-------------\n");
    while(1){
		pthread_mutex_lock(&_syncOkToSendMutex);
		{				
			pthread_cond_wait(&_syncOkToSendCondVar, &_syncOkToSendMutex);
		}
		pthread_mutex_unlock(&_syncOkToSendMutex);

		//printf("udpsender: Received singnal from keyboardList:  \n");

		int sockfd;
		if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Sender: socket");
		}
		
		// struct sockaddr_in sinNew;
		// memset(&sinNew, 0, sizeof(sinNew));
		// sinNew.sin_family = AF_INET;
		// sinNew.sin_addr.s_addr = htonl(INADDR_ANY);
		// sinNew.sin_port = htons(senderPort);

		char* message;
		message = List_trim(list_send);  
		
		if (sendto(sockfd, message, strlen(message)+1, 0, sinR->ai_addr, sinR->ai_addrlen) == -1) {
			printf("ERROR: Message not successfully sent - udpSender\n");
		}
		// else{
		// 	printf("SUCESS : Message sent successfully from udpSender\n");
		// }
		//receive_Signaller();
		if (strcmp(message, "!\n") == 0) {
                printf("Shutdown signal received. Exiting...\n");
                
                //shutDown = 1; /
    
				 close(sockfd); // Close the socket to clean up resources
				// free(message); 
				// message = NULL;
				 send_waitForShutdown();
				 
				exit(0);
            } 
        free(message);
        message = NULL; 
    }
    return NULL;
}

void send_Singaller() {
    pthread_mutex_lock(&_syncOkToSendMutex);
	{
		pthread_cond_signal(&_syncOkToSendCondVar);
	}
	pthread_mutex_unlock(&_syncOkToSendMutex);
}

void send_Init(List* list, struct addrinfo** remoteAddress, int Sport) {
	sinR = *remoteAddress;
	senderPort = Sport;
	list_send = list;
    pthread_create(&send_thread_id, NULL, sender_thread, NULL);
}

void send_waitForShutdown() {
    pthread_join(send_thread_id, NULL);
}
