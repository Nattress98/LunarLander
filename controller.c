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

bool isActive = true;

pthread_t logThread, serverCommandThread;
int rc3, rc4;

sem_t logSemaphore;
sem_t serverSemaphore;

void *sendCommand(void *arg);
void *getState(void *arg);		
void *getTerrain(void *arg);
void *getCondition(void *arg);

void logCommand(char msg[BUFF_SIZE]);


//Logs data into a text file
void *logData(void *arg){
  char *msg = (char*)arg;
  out_file = fopen("DataLog.txt","a+");
    
  fprintf(out_file, "%s\n", msg);

  sem_wait(&serverSemaphore);
	  rc4 = pthread_create( &serverCommandThread, NULL, getState, NULL);
	  assert(rc4 == 0);
	  pthread_join(serverCommandThread, NULL);
  sem_post(&serverSemaphore);

  fprintf(out_file, "%s\n", state);

  fprintf(out_file, "%s\n", condition);

  sem_wait(&serverSemaphore);
	  rc4 = pthread_create( &serverCommandThread, NULL, getTerrain, NULL);
	  assert(rc4 == 0);
	  pthread_join(serverCommandThread, NULL);
  sem_post(&serverSemaphore);

  fprintf(out_file, "%s\n", terrain);

  fclose(out_file);

  return 0;
}

//gets input and sends prepares commands for the server
void *getInput(void *arg){
    char input;
    while (isActive){
        char outgoing[BUFF_SIZE];
        scanf("%s", &input);
	
	switch (input){
	   case 'a':
		if(throttle <= 90)
		   	throttle += 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sem_wait(&serverSemaphore);
			rc4 = pthread_create( &serverCommandThread, NULL, sendCommand, (void *) &outgoing);
	  		assert(rc4 == 0);
	  		pthread_join(serverCommandThread, NULL);
 		sem_post(&serverSemaphore);
		logCommand("throttle up \n");
		printf("throttle: %d \n", throttle);
		break;
	   case 's':
		if(throttle >= 10)
			throttle -= 10;
		sprintf(outgoing, "command:!\nmain-engine:%d", throttle);
		sem_wait(&serverSemaphore);
			rc4 = pthread_create( &serverCommandThread, NULL, sendCommand, (void *) &outgoing);
	  		assert(rc4 == 0);
	  		pthread_join(serverCommandThread, NULL);
 		sem_post(&serverSemaphore);
		logCommand("throttle down \n");
		printf("throttle: %d \n", throttle);
		break;
	   case 'q':
		printf("spin left \n");
		strcpy(outgoing, "command:!\nrcs-roll:-0.5");
		sem_wait(&serverSemaphore);
			rc4 = pthread_create( &serverCommandThread, NULL, sendCommand, (void *) &outgoing);
	  		assert(rc4 == 0);
	  		pthread_join(serverCommandThread, NULL);
 		sem_post(&serverSemaphore);
		logCommand("spin left \n"); 
		break;
	   case 'w':
		printf("stop turning \n");
		strcpy(outgoing, "command:!\nrcs-roll:0");
		sem_wait(&serverSemaphore);
			rc4 = pthread_create( &serverCommandThread, NULL, sendCommand, (void *) &outgoing);
	  		assert(rc4 == 0);
	  		pthread_join(serverCommandThread, NULL);
 		sem_post(&serverSemaphore);
		logCommand("stop spinning \n");
		break;
	   case 'e':
		printf("turn right \n");
		strcpy(outgoing, "command:!\nrcs-roll:0.5");
		sem_wait(&serverSemaphore);
			rc4 = pthread_create( &serverCommandThread, NULL, sendCommand, (void *) &outgoing);
	  		assert(rc4 == 0);
	  		pthread_join(serverCommandThread, NULL);
 		sem_post(&serverSemaphore);
		logCommand("spin right \n");
		break;
	   case 'r':
		sem_wait(&serverSemaphore);
		
		rc4 = pthread_create( &serverCommandThread, NULL, getState, NULL);
  		assert(rc4 == 0);
  		pthread_join(serverCommandThread, NULL);

 		sem_post(&serverSemaphore);
		printf("%s", state);
		break;
	   case 'd':
		sem_wait(&serverSemaphore);
		rc4 = pthread_create( &serverCommandThread, NULL, getTerrain, NULL);
  		assert(rc4 == 0);
  		pthread_join(serverCommandThread, NULL);

 		sem_post(&serverSemaphore);

		printf("%s", terrain);
		break;
	   case 'l':
		isActive = false;
		printf("exiting...\n");
		break;	
	   case 'h':
		printf("a: throttle up\n");
		printf("s: throttle down\n");
		printf("q: spin left\n");
		printf("w: stop spinning\n");
		printf("e: spin right\n");
		printf("r: get the current state of the lander \n");
		printf("d: get terrain details beneath the lander \n");
	   default:
		printf("invalid input, enter h for controls. \n");
		break;
  	}
    }
    pthread_exit(NULL);
}
//Updates dashboard every 0.1s
void *updateDash(void *arg){ 
  while(isActive){
	sem_wait(&serverSemaphore);
	rc4 = pthread_create( &serverCommandThread, NULL, getCondition, NULL);
  	assert(rc4 == 0);
  	pthread_join(serverCommandThread, NULL);
	sem_post(&serverSemaphore);
	char cond[BUFF_SIZE];
	strcpy(cond, condition);
	//removes % sign which is causing parsing error in Java file
	for (int i = 0; cond[i] != '\0'; i++)
	{
		if (cond[i] == '%')
			cond[i] = ' ';
	}
        sendto(fd, cond, strlen(cond), 0, address2->ai_addr, address2->ai_addrlen);
       	usleep (100000);
  }
  return 0;
}

//initialises and closes the program
int main ( int argc, char *argv[] )
{
    char *host = "127.0.1.1";
    char *port = "65200";
    char *port2 = "65250";

    out_file = fopen("DataLog.txt", "w");

    if (out_file == NULL) 
    {   
	printf("Error! Could not open file\n");  
    }
    //empties file on start
    fclose(out_file);

    pthread_t inputThread, updateDashThread;

    int rc1, rc2;

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
    
    rc1 = pthread_create( &inputThread, NULL, getInput, NULL);
    rc2 = pthread_create( &updateDashThread, NULL, updateDash, NULL);
       

    assert(rc1 == 0);
    pthread_join(inputThread, NULL);
    assert(rc2 == 0);
    pthread_join(updateDashThread, NULL);

    sem_destroy(&serverSemaphore);
    sem_destroy(&logSemaphore);
}

//gets the state of the lander and stores it in a variable
void *getState(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;
	

	
	strcpy(msg, "state:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "state:="); 

	if(s != NULL){
		strcpy(state, incoming);
	}

	return 0;
}
//gets the terrain information beneath the lander and stores it in the variable
void *getTerrain(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;

	strcpy(msg, "terrain:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "terrain:="); 

	if(s != NULL){
		strcpy(terrain, incoming);
	}
	return 0;
}
//gets the condition and stores it in the variable
void *getCondition(void *arg){
	char msg[BUFF_SIZE];
    	char incoming[BUFF_SIZE];
	size_t msgsize;

	strcpy(msg, "condition:?");
	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';

	char *s;
 	s = strstr(incoming, "condition:="); 

	if(s != NULL){
		strcpy(condition, incoming);
	}
	return 0;

}
//initiallises the data log thread
void logCommand(char msg[BUFF_SIZE])
{
  sem_wait(&logSemaphore);
  	rc3 = pthread_create( &logThread, NULL, logData, msg);
        assert(rc3 == 0);
    	pthread_join(logThread, NULL); 
  sem_post(&logSemaphore);
}

// sends a control  command to the server
void *sendCommand(void *arg)
{
	char *msg;
	msg = (char*) arg;
    	char incoming[BUFF_SIZE];
	size_t msgsize;

	sendto(fd, msg, strlen(msg), 0, address->ai_addr, address->ai_addrlen);
	msgsize = recvfrom(fd, incoming, BUFF_SIZE, 0, NULL, 0);
    	incoming[msgsize] = '\0';
	return 0;
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
