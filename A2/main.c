#include <pthread.h>
#include <stdlib.h>
#include <netdb.h> 
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


int main(int argc, char* argv[]) {
	
	printf("Arguments (%d) are:\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("Arg-%d: %s\n", i, argv[i]);
    }
	
	struct addrinfo hint, *remoteAddress;
	memset(&hint, 0, sizeof(struct addrinfo));

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;
	int rv;
	if ((rv = getaddrinfo(argv[2], argv[3], &hint, &remoteAddress)) != 0) {
		fprintf(stderr, "getaddrinfo");
		return 1;
	}else{
		printf("rv %d\n",rv);
	}

	List* list1 = List_create(); 
	List* list2 = List_create();
	
	udpReceive_Init(list1, atoi(argv[1]));
	screen_Init(list1);

	keyboard_Init(list2);
	send_Init(list2, &remoteAddress, atoi(argv[1]));	
	
	keyboard_waitForShutdown();
	send_waitForShutdown();
	udpReceive_waitForShutdown();
	screen_waitForShutdown();
	// freeaddrinfo(remoteAddress);
	return 0;
}


 




