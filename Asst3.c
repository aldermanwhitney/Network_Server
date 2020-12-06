#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define BACKLOG 5

int argument = 0;


// the argument we will pass to the connection-handler threads
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};



int readAndErrorCheck2(struct connection *c){
return 0;
}



void readAndErrorCheck(struct connection *c){

char msgType[4];
//char msgReg[] = "REG";
int bytesRead;

    bytesRead = read(c->fd, msgType, 3);
    msgType[bytesRead] = '\0';
    printf("read %d bytes |%s|\n", bytesRead, msgType);

if(strncmp(msgType, "REG", 3)==0){
printf("Message is regular type. \n");

//read first vertical bar in 
//char vertBar[2];
//bytesRead = read(c->fd, msgType, 1);
//vertBar[bytesRead] = '\0';

//error checking logic

char buffer[300];

//while((bytesRead = read(c->fd, buffer, 100))>0){
//}

bytesRead = read(c->fd, buffer, 11);
buffer[bytesRead] = '\0';
printf("Server Recieved: %s", buffer);




}
else if(strncmp(msgType, "ERR", 3)==0){

}
else{

}


return;
}


void *echo(void *arg)
{
    char host[100]; 
    char port[10]; 
    char buf[101];
    struct connection *c = (struct connection *) arg;
    int error;
    //int nread;

	// find out the name and port of the remote host
    error = getnameinfo((struct sockaddr *) &c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);
    	// we provide:
    	// the address and its length
    	// a buffer to write the host name, and its length
    	// a buffer to write the port (as a string), and its length
    	// flags, in this case saying that we want the port as a number, not a service name
    if (error != 0) {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        close(c->fd);
        return NULL;
    }

    //connected to client
    printf("[%s:%s] Connection\n", host, port);

    char resp[] = "REG|13|Knock, Knock.|";
    char resp2[] = "REG|4|Boo.|";
    char resp3[] = "REG|23|Awe, don't cry.|";

   char recArg[] = "REG|12|Who's There?|";
   char recArg2[] = "REG|17|Boo, who?|";
 //  char recArg3[] = "REG|12|Yuck.|";


    write(c->fd, resp, sizeof(resp));
	
    int bytesRead = 0;    
    while ((bytesRead = read(c->fd, buf, 100)) > 0) {
        buf[bytesRead] = '\0';
        printf("read %d bytes |%s|\n", bytesRead, buf);
    	
	if(strcmp(buf, recArg)==0){
	 write(c->fd, resp2, sizeof(resp2));
	}	
	else if(strcmp(buf, recArg2)==0){
	 write(c->fd, resp3, sizeof(resp3));
	}	
	else{//(strcmp(buf, recArg)==0){
	 write(c->fd, resp, sizeof(resp));
	}	
	
    	////memset(resp2, '\0', sizeof(resp2));
	////memset(resp2, 0, 11);
    	//memcpy(resp, resp2, strlen(resp2)+1);
    }


   printf("[%s:%s] got EOF\n", host, port);

    close(c->fd);
    free(c);
    return NULL;
}

int server(char *port)
{
    struct addrinfo hint, *address_list, *addr;
    struct connection *con;
    int error, sfd;
    pthread_t tid;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    	// setting AI_PASSIVE means that we want to create a listening socket

    // get socket and address info for listening port
    // - for a listening socket, give NULL as the host name (because the socket is on
    //   the local host)
    error = getaddrinfo(NULL, port, &hint, &address_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (addr = address_list; addr != NULL; addr = addr->ai_next) {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        
        // if we couldn't create the socket, try the next method
        if (sfd == -1) {
            continue;
        }

        // if we were able to create the socket, try to set it up for
        // incoming connections;
        // 
        // note that this requires two steps:
        // - bind associates the socket with the specified port on the local host
        // - listen sets up a queue for incoming connections and allows us to use accept
        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0)) {
            break;
        }

        // unable to set it up, so try the next method
        close(sfd);
    }

    if (addr == NULL) {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(address_list);

    // at this point sfd is bound and listening
    printf("Waiting for connection\n");
    for (;;) {
    	// create argument struct for child thread
		con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);
        	// addr_len is a read/write parameter to accept
        	// we set the initial value, saying how much space is available
        	// after the call to accept, this field will contain the actual address length
        
        // wait for an incoming connection
        con->fd = accept(sfd, (struct sockaddr *) &con->addr, &con->addr_len);
        	// we provide
        	// sfd - the listening socket
        	// &con->addr - a location to write the address of the remote host
        	// &con->addr_len - a location to write the length of the address
        	//
        	// accept will block until a remote host tries to connect
        	// it returns a new socket that can be used to communicate with the remote
        	// host, and writes the address of the remote hist into the provided location
        
        // if we got back -1, it means something went wrong
        if (con->fd == -1) {
            perror("accept");
            continue;
        }

		// spin off a worker thread to handle the remote connection
        error = pthread_create(&tid, NULL, echo, con);

		// if we couldn't spin off the thread, clean up and wait for another connection
        if (error != 0) {
            fprintf(stderr, "Unable to create thread: %d\n", error);
            close(con->fd);
            free(con);
            continue;
        }

		// otherwise, detach the thread and wait for the next connection request
        pthread_detach(tid);
    }

    // never reach here
    return 0;
}



int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    (void) server(argv[1]);
    return EXIT_SUCCESS;
}



