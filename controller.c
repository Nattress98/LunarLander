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

char condition[BUFF_SIZE];
char terrain[BUFF_SIZE];
char state[BUFF_SIZE];
struct addrinfo *address;
struct addrinfo *address2;
int fd;



pthread_t thread1, thread2, thread3, thread4;
int rc1, rc2, rc3, rc4;

sem_t logSemaphore;
sem_t serverSemaphore;

void *getState(void *arg);
void *getTerrain(void *arg);
void *getCondition(void *arg);

void sendCommand(char msg[BUFF_SIZE]);
void logCommand(char msg[BUFF_SIZE]);
//void getState();
//void getTerrain();
//void getCondition();

void *logData(void *arg){
  sem_wait(&logSemaphore);
  char *msg = (char*)arg;
  out_file = fopen("DataLog","a+");
    
  fprintf(out_file, "%s\n", msg);

  rc4 = pthread_create( &thread4, NULL, getState, NULL);
  //getState();
  assert(rc4 == 0);
  pthread_join(thread4, NULL);
 
  fprintf(out_file, "%s\n", state);

  fprintf(out_file, "%s\n", condition);

  rc4 = pthread_create( &thread4, NULL, getTerrain, NULL);
  //getTerrain();
  assert(rc4 == 0);
  pthread_join(thread4, NULL);
  fprintf(out_file, "%s\n", terrain);



  fclose(out_file);
  sem_post(&logSemaphore);
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
	
	rc4 = pthread_create( &thread4, NULL, getCondition, NULL);
  	//getCondition();
  	assert(rc4 == 0);
  	pthread_join(thread4, NULL);
        sendto(fd, condition, strlen(condition), 0, address2->ai_addr, address2->ai_addrlen);
       	usleep (100000);
  }
}


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

    sem_init(&serverSemaphore, 0, 1);
    sem_init(&logSemaphore, 0, 1);
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
    sem_destroy(&serverSemaphore);
    sem_destroy(&logSemaphore);
}

void *getState(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	
	sem_wait(&serverSemaphore);
	
	strcpy(msg, "state:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "state:="); 

	if(s != NULL){
		strcpy(state, incoming);
	}
	sem_post(&serverSemaphore);
	return 0;
}
void *getTerrain(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	sem_wait(&serverSemaphore);
	strcpy(msg, "terrain:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "terrain:="); 

	if(s != NULL){
		strcpy(terrain, incoming);
	}
	sem_post(&serverSemaphore);
}
void *getCondition(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	sem_wait(&serverSemaphore);
	strcpy(msg, "condition:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "condition:="); 

	if(s != NULL){
		strcpy(condition, incoming);
	}
	sem_post(&serverSemaphore);
}
void logCommand(char msg[BUFF_SIZE])
{
  	rc3 = pthread_create( &thread3, NULL, logData, msg);
        assert(rc3 == 0);
    	pthread_join(thread3, NULL); 
}

void sendCommand(char msg[BUFF_SIZE])
{
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	sem_wait(&serverSemaphore);
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
	sem_post(&serverSemaphore);
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
