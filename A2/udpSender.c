
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
#define MAX_LEN 256

static pthread_cond_t syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t syncOkToSendMutex = PTHREAD_MUTEX_INITIALIZER;

// address and port of the sender will passed from main afte using getaddrinfo
struct addrinfo* sinR; 
int senderPort;

pthread_t send_thread_id;

static List* list_send; 
static char* message;

static pthread_mutex_t ListCriticalSectionMutex = PTHREAD_MUTEX_INITIALIZER;

void* sender_thread() {
	int sockfd;
    while(1){

		// wait for signal from keyboard input if there is new msg 
		pthread_mutex_lock(&syncOkToSendMutex);
		{				
			pthread_cond_wait(&syncOkToSendCondVar, &syncOkToSendMutex);
		}
		pthread_mutex_unlock(&syncOkToSendMutex);

		if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Sender: socket");
		}
		pthread_mutex_lock(&ListCriticalSectionMutex);
        {				
            // mutex lock when accessing critical section
            message = List_trim(list_send);  
        }
        pthread_mutex_unlock(&ListCriticalSectionMutex);
		 
		if (sendto(sockfd, message, strlen(message), 0, sinR->ai_addr, sinR->ai_addrlen) == -1) {
			perror("ERROR: Message not successfully sent\n");
		}
	
		if (*(message) == '!'){
			// if user types "!" we need to free message memory
	        free(message);
        	message = NULL; 
            return NULL;
        }
		// free alloacated memory of message after sending message 
        free(message); 
    }
	close(sockfd); 
    return NULL;
}

// signal the waiting mutex when recives signal from keyboard there is new msg on screen
void send_Singaller() {
    pthread_mutex_lock(&syncOkToSendMutex);
	{
		pthread_cond_signal(&syncOkToSendCondVar);
	}
	pthread_mutex_unlock(&syncOkToSendMutex);
}

void send_Init(List* list, struct addrinfo** remote, int Sport) {
	sinR = *remote;
	senderPort = Sport;
	list_send = list;
    pthread_create(&send_thread_id, NULL, sender_thread, NULL);
}

// cancel thread for terminating condition of "!"
void cancelSender_thread(){
	//printf("UdpSender Thread cancelled\n");
    pthread_cancel(send_thread_id);
}

void send_waitForShutdown() {
	// just to make sure free the memory before join
	if(message!= NULL){
		free(message);
		message = NULL;
	}
    pthread_join(send_thread_id, NULL);
	pthread_cond_destroy(&syncOkToSendCondVar);
    pthread_mutex_destroy(&syncOkToSendMutex);
    pthread_mutex_destroy(&ListCriticalSectionMutex);
}
