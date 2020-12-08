#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#define BACKLOG 5

int argument = 0;
int msgNumber = 0;
int bufSize = 101;
int bufBytesUsed = 0;


// the argument we will pass to the connection-handler threads
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};


/**Function takes a pointer to a connection struct
 * and a char type: f, l or c indicating message type
 *And writes the appropriate error message to the socket
 *Will return without a write() if connection is null or type is not f,l or c
 */
void sendErrorMsg(struct connection *c, char type){

if((c==NULL) || (type!='f' && type!='l' && type!='c')){
printf("Connection is null or wrong type.\n");
return;
}

if((msgNumber==1) && (type=='f')){
char errM1FT[] = "ERR|M1FT|";
printf("%s\n", errM1FT);
write(c->fd, errM1FT, strlen(errM1FT));
}
else if((msgNumber==1) && (type=='l')){
char errM1LN[] = "ERR|M1LN|";
printf("%s\n", errM1LN);
write(c->fd, errM1LN, strlen(errM1LN));
}
else if((msgNumber==1) && (type=='c')){
char errM1CT[] = "ERR|M1CT|";
printf("%s\n", errM1CT);
write(c->fd, errM1CT, strlen(errM1CT));
}
else if((msgNumber==3) && (type=='f')){
char errM3FT[] = "ERR|M3FT|";
printf("%s\n", errM3FT);
write(c->fd, errM3FT, strlen(errM3FT));
}
else if((msgNumber==3) && (type=='l')){
char errM3LN[] = "ERR|M3LN|";
printf("%s\n", errM3LN);
write(c->fd, errM3LN, strlen(errM3LN));
}
else if((msgNumber==3) && (type=='c')){
char errM3CT[] = "ERR|M3CT|";
printf("%s\n", errM3CT);
write(c->fd, errM3CT, strlen(errM3CT));
}
else if((msgNumber==5) && (type=='f')){
char errM5FT[] = "ERR|M5FT|";
printf("%s\n", errM5FT);
write(c->fd, errM5FT, strlen(errM5FT));
}
else if((msgNumber==5) && (type=='l')){
char errM5LN[] = "ERR|M5LN|";
printf("%s\n", errM5LN);
write(c->fd, errM5LN, strlen(errM5LN));
}
else if((msgNumber==5) && (type=='c')){
char errM5CT[] = "ERR|M5CT|";
printf("%s\n", errM5CT);
write(c->fd, errM5CT, strlen(errM5CT));
}
else{
printf("Nothing sent.Type: %c MsgNumber: %d\n", type, msgNumber);
return;
}

return;
}

/**Function reads socket message from connection c into char buffer (buf) byte by byte
 *checks for errors
 *If an error is found, an appropriate error message is sent and -1 is returned
 *If no errors found, function returns number of bytes read into buffer
 */
int readAndErrorCheck2(struct connection *c, char buf[]){

int bytesRead = 0;
int verticalBarCount = 0;
int i = 0;

//read byte by byte into the buffer 
//until the first vertical bar, or the 5th char, whichever comes first
//as long as all preceding chars match what is allowed
while(verticalBarCount<1){
bytesRead+=read(c->fd, &buf[i], 1);
printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
if(buf[i]=='|'){
verticalBarCount++;
}

//Check for correct format
if((i==0) && (buf[i]!='R' && buf[i]!='E')){
break;
}
if((i==1) && (buf[i]!='E' && buf[i]!='R')){
break;
}
if((i==2) && (buf[i]!='G' && buf[i]!='R')){
break;
}
if(i==5){
break;
}



i++;
}


//Check for correctly formatted message type
if(strncmp(buf, "REG|", 4)==0){
printf("Message is regular\n");

	char lengthBuf[500];
	int lengthBufEnd = 0;
	//Continue reading byte by byte until the second vertical bar is sent
	//Or reached a non numerical character, whichever is first
	while(verticalBarCount<2){
	bytesRead += read(c->fd, &buf[i], 1);
	printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
	if(buf[i]=='|'){
	verticalBarCount++;
	}
	//send formatting error if reached a non digit before second vertical bar
	if((isdigit(buf[i])==0) && (buf[i]!='|')){
	sendErrorMsg(c, 'f');
	return -1;
	}

	if(verticalBarCount<2){
	lengthBuf[lengthBufEnd]=buf[i];
	}

	lengthBufEnd++;
	i++;
	}


	lengthBuf[lengthBufEnd-1]='\0';
	//at this point, buffer will have ex: REG|23|
	//length buffer will have 23
////	printf("Current Buffer: %s\tLength Buffer:%c:%c\n", buf, lengthBuf[0], lengthBuf[lengthBufEnd-2]);
	printf("here\n");
	buf[bufSize-1]='\0';
	printf("Current Buffer: %s\n", buf);
	printf("Length Buf[0]: %c\n", lengthBuf[0]);	
	int length = atoi(lengthBuf);
	int endIndex = length + i + 1;
	printf("Length: %d\n", length);


	//read rest of message char by char
	//until vertical bar or length reached, whichever is first
	//while(i<endIndex){
	while(1){
	if(bytesRead==bufSize){
	bufSize*=2;
	buf = realloc(buf, bufSize);	
	}
	bytesRead+=read(c->fd, &buf[i], 1);
	printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
	if(buf[i]=='|'){
	verticalBarCount++;
	break;
	}
	i++;	
	}

	//Send error msg if incorrect number of vertical bars
	if(verticalBarCount!=3){
	printf("Vertical Bar Count is: %d\n", verticalBarCount);
	sendErrorMsg(c, 'f');
	return -1;
	}
	else if(i!=endIndex-1){
	printf("Msg Length Incorrect. Length: %d i: %i endIndex-1: %d\n", length, i, endIndex-1);
	sendErrorMsg(c, 'l');
	return -1;
	}
	else{
	printf("Bytes Read: %d\n", bytesRead);
	return bytesRead;
	}


	}

else if(strncmp(buf, "ERR|", 4)==0){
//if server has recieved an error message, close connection
return -1;
}
else{
//error message, incorect format
//puts("here");
sendErrorMsg(c, 'f');
return -1;
}








printf("Bytes Read: %d\n", bytesRead);
return bytesRead;
}

void *echo(void *arg)
{
    char host[100]; 
    char port[10]; 
    //char buf[bufSize];

    char *buf = malloc(bufSize);
    struct connection *c = (struct connection *) arg;
    int error;
    msgNumber = 0;

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

    char resp[] = "REG|13|Knock, knock.|";
    char resp2[] = "REG|4|Boo.|";
    char resp3[] = "REG|15|Awe, don't cry.|";
    //char resp4[] = "No answer for this case.";

   char recArg[] = "REG|12|Who's there?|";
   char recArg2[] = "REG|9|Boo, who?|";
   //char recArg3[] = "REG|5|Yuck.|";

    //initial Knock, Knock
    write(c->fd, resp, sizeof(resp));
    msgNumber++;

    int bytesRead = 0;    
    while ((bytesRead = readAndErrorCheck2(c, buf)) > 0) {
        buf[bytesRead] = '\0';
        printf("read %d bytes |%s|\n", bytesRead, buf);
    	
	//Who's there received
	if(strcmp(buf, recArg)==0 && msgNumber==1){
	 write(c->fd, resp2, sizeof(resp2));
	 msgNumber+=2;
	}//Boo, who? received
	else if(strcmp(buf, recArg2)==0 && msgNumber==3){
	 write(c->fd, resp3, sizeof(resp3));
	 msgNumber+=2;
	}//msg of disgust recieved	
	else if((buf[bytesRead-2]=='.' || buf[bytesRead-2]=='!' ||
		       buf[bytesRead-2]=='?') && msgNumber==5){
	 //write(c->fd, resp3, sizeof(resp3));
	break;
	}
	else{//content is incorrect
	sendErrorMsg(c, 'c');
       	break;	
	//write(c->fd, resp4, sizeof(resp4));
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

    //init socket hints
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

int portNum = atoi(argv[1]);
if (portNum==0 || portNum<=5000 || portNum>=65536){
printf("Must provide a valid port number. Must be in exclusive range: 5000-65536\n");
exit(EXIT_FAILURE);
}

    (void) server(argv[1]);
    return EXIT_SUCCESS;
}



