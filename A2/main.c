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
	
	puts("Welcome");
	
	// refrence for: https://beej.us/guide/bgnet/html/split/client-server-background.html#datagram
	struct addrinfo hints, *servinfo;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int rv;
	if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo");
		return 1;
	}

	List* list1 = List_create(); 
	List* list2 = List_create();
	
	keyboard_Init(list2);

	// servinfo store the info of remote address and port
	send_Init(list2, &servinfo, atoi(argv[1]));
	udpReceive_Init(list1, atoi(argv[1]));
	screen_Init(list1);
	
	keyboard_waitForShutdown();
	send_waitForShutdown();
	udpReceive_waitForShutdown();
	screen_waitForShutdown();

	// Free memory alloacted by getaddrinfo to servinfo
	freeaddrinfo(servinfo);

	// Free both list
	List_free(list1, free);
	List_free(list2, free);

	puts("Connection closed");

	return 0;
}


 




