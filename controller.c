#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>
#include <assert.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

int throttle = 0;

const size_t buffsize = 4096;
struct addrinfo *address;
struct addrinfo *address2;
int fd;

void sendCommand(char msg[buffsize]);
void getCondition();

void *getInput(void *arg){
    char input;
    while (1){
        char outgoing[buffsize];
        scanf("%s", &input);
	switch (input){
	   case 'a':
		if(throttle <= 90)
		   	throttle += 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sendCommand(outgoing);
		printf("throttle: %d \n", throttle);
		break;
	   case 's':
		if(throttle >= 10)
			throttle -= 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sendCommand(outgoing);
		printf("throttle: %d \n", throttle);
		break;
	   case 'q':
		printf("turn left \n");
		strcpy(outgoing, "command:!\nrcs-roll:-0.5");
                sendCommand(outgoing);
		break;
	   case 'w':
		printf("stop turning \n");
		strcpy(outgoing, "command:!\nrcs-roll:0");
                sendCommand(outgoing);
		break;
	   case 'e':
		printf("turn right \n");
		strcpy(outgoing, "command:!\nrcs-roll:0.5");
                sendCommand(outgoing);
		break;
	   case 'r':
		strcpy(outgoing, "state:=");
		sendCommand(outgoing);
		break;
	   case 'd':
		strcpy(outgoing, "terrain:?");
		sendCommand(outgoing);
		break;
	   default:
		printf("invalid input \n");
		strcpy(outgoing, "");
		sendCommand(outgoing);
		break;
  	}
	//msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    	//incoming[msgsize] = '\0';
	//if(!strcmp(incoming, "command:="))
		//printf("reply is: %s \n", incoming);
    }
    pthread_exit(NULL);
}
void *updateDash(void *arg){ 
  while(1){
	char msg[buffsize];
    	char incoming[buffsize];
	size_t msgsize;
	
	strcpy(msg, "condition:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, buffsize, 0, NULL, 0); /* Don't need the senders address */
    	incoming[msgsize] = '\0';
        sendto(fd, incoming, strlen(incoming), 0, address2->ai_addr, address2->ai_addrlen);
       	usleep (100000);
  }
}
int main ( int argc, char *argv[] )
{
    pthread_t thread1, thread2;
    int rc1, rc2;

    char *host = "127.0.1.1";
    char *port = "65200";
    char *port2 = "65250";

    
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    const struct addrinfo hints2 = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    int err, err2;
    err = getaddrinfo( host, port, &hints, &address);
    if (err) {
        fprintf(stderr, "Error getting address: %s\n", gai_strerror(err));
           exit(1);
    }
    err2 = getaddrinfo( host, port2, &hints2, &address2);
    if (err) {
        fprintf(stderr, "Error getting address: %s\n", gai_strerror(err2));
           exit(1);
    }
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr, "error making socket: %s\n", strerror(errno));
        exit(1);
    }
    
    struct sockaddr clientaddr;
    socklen_t addrlen = sizeof(clientaddr);

    
    rc1 = pthread_create( &thread1, NULL, getInput, NULL);
    rc2 = pthread_create( &thread2, NULL, updateDash, NULL);
    
    assert(rc1 == 0);
    pthread_join(thread1, NULL);
    assert(rc2 == 0);
    pthread_join(thread2, NULL);
}



void sendCommand(char msg[buffsize]){
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
}
void getCondition(){
	
}
void finished(int sig)
{
    exit(0);
}

//static int fd;
void cleanup(void)
{
    close(fd);
}
