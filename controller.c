#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>
#include <semaphore.h>
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

#define BUFF_SIZE 4096

FILE *out_file;
int throttle = 0;

//const size_t buffsize = 4096;
char condition[BUFF_SIZE];
char terrain[BUFF_SIZE];
char state[BUFF_SIZE];
struct addrinfo *address;
struct addrinfo *address2;
int fd;



pthread_t thread1, thread2, thread3;
int rc1, rc2, rc3;

sem_t logSemaphore;

void sendCommand(char msg[BUFF_SIZE]);
void logCommand(char msg[BUFF_SIZE]);
void getState();
void getTerrain();

void *logData(void *arg){
  char *msg = (char*)arg;

  out_file = fopen("DataLog","a+");
    
  fprintf(out_file, "%s\n", msg);
  getState();
  fprintf(out_file, "%s\n", state);
  fprintf(out_file, "%s\n", condition);
  getTerrain();
  fprintf(out_file, "%s\n", terrain);


  fclose(out_file);
  return 0;
}

void *getInput(void *arg){
    char input;
    while (1){
        char outgoing[BUFF_SIZE];
        scanf("%s", &input);
	
	switch (input){
	   case 'a':
		if(throttle <= 90)
		   	throttle += 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sendCommand(outgoing);
		logCommand("throttle up");
		printf("throttle: %d \n", throttle);
		break;
	   case 's':
		if(throttle >= 10)
			throttle -= 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sendCommand(outgoing);
		logCommand("throttle down");
		printf("throttle: %d \n", throttle);
		break;
	   case 'q':
		printf("spin left \n");
		strcpy(outgoing, "command:!\nrcs-roll:-0.5");
                sendCommand(outgoing);
		logCommand("spin left"); 
		break;
	   case 'w':
		printf("stop turning \n");
		strcpy(outgoing, "command:!\nrcs-roll:0");
                sendCommand(outgoing);
		logCommand("stop spinning");
		break;
	   case 'e':
		printf("turn right \n");
		strcpy(outgoing, "command:!\nrcs-roll:0.5");
                sendCommand(outgoing);
		logCommand("spin right");
		break;
	   case 'r':
		strcpy(outgoing, "state:?");
		sendCommand(outgoing);
		break;
	   case 'd':
		strcpy(outgoing, "terrain:?");
		sendCommand(outgoing);
		break;
	   case 'h':
		printf("a: throttle up\n");
		printf("s: throttle down\n");
		printf("q: spin left\n");
		printf("w: stop spinning\n");
		printf("e: spin right\n");
	   default:
		printf("invalid input, enter h for controls. \n");
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
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	
	strcpy(msg, "condition:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
	strcpy(condition, incoming);
        sendto(fd, incoming, strlen(incoming), 0, address2->ai_addr, address2->ai_addrlen);
       	usleep (100000);
  }
}

/*void *logData(void *arg, char message[buffersize]){ 
  FILE *out_file = fopen("DataLog", "w");
  if (out_file == NULL) 
  {   
	printf("Error! Could not open file\n");  
  }

  while(1){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	
	strcpy(msg, "state:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
        
	fprintf(out_file, "%s\n", incoming);

       	usleep (100000);
  }
  fclose(out_file);
}*/


int main ( int argc, char *argv[] )
{
    char *host = "127.0.1.1";
    char *port = "65200";
    char *port2 = "65250";

    out_file = fopen("DataLog", "w");

    if (out_file == NULL) 
    {   
	printf("Error! Could not open file\n");  
    }
    //empties file on start
    fclose(out_file);

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
    
    //struct sockaddr clientaddr;
    //socklen_t addrlen = sizeof(clientaddr);

    
    rc1 = pthread_create( &thread1, NULL, getInput, NULL);
    rc2 = pthread_create( &thread2, NULL, updateDash, NULL);
       

    assert(rc1 == 0);
    pthread_join(thread1, NULL);
    assert(rc2 == 0);
    pthread_join(thread2, NULL);
}

void getState(){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	
	strcpy(msg, "state:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
	strcpy(state, incoming);
}
void getTerrain(){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	
	strcpy(msg, "terrain:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
	strcpy(terrain, incoming);
	printf("%s\n",terrain);
}
void logCommand(char msg[BUFF_SIZE])
{
  	rc3 = pthread_create( &thread3, NULL, logData, msg);
        assert(rc3 == 0);
    	pthread_join(thread3, NULL); 
}

void sendCommand(char msg[BUFF_SIZE]){
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
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
