#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

// the argument we will pass to the connection-handler threads
struct ConnectionThreadArg {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int msgNumber = 0;
int bufBytesUsed = 0;
int bufSize = 32768;

/**Function takes a pointer to a connection struct
 * and a char type: f, l or c indicating message type
 *And writes the appropriate error message to the socket
 *Will return without a write() if connection is null or type is not f,l or c
 */
void sendErrorMsg(struct ConnectionThreadArg *c, char type){

if((c==NULL) || (type!='f' && type!='l' && type!='c')){
return;
}

if((msgNumber==1) && (type=='f')){
char errM1FT[] = "ERR|M1FT|";
printf("Server Sent: %s\n", errM1FT);
write(c->fd, errM1FT, strlen(errM1FT));
}
else if((msgNumber==1) && (type=='l')){
char errM1LN[] = "ERR|M1LN|";
printf("Server Sent: %s\n", errM1LN);
write(c->fd, errM1LN, strlen(errM1LN));
}
else if((msgNumber==1) && (type=='c')){
char errM1CT[] = "ERR|M1CT|";
printf("Server Sent: %s\n", errM1CT);
write(c->fd, errM1CT, strlen(errM1CT));
}
else if((msgNumber==3) && (type=='f')){
char errM3FT[] = "ERR|M3FT|";
printf("Server Sent: %s\n", errM3FT);
write(c->fd, errM3FT, strlen(errM3FT));
}
else if((msgNumber==3) && (type=='l')){
char errM3LN[] = "ERR|M3LN|";
printf("Server Sent: %s\n", errM3LN);
write(c->fd, errM3LN, strlen(errM3LN));
}
else if((msgNumber==3) && (type=='c')){
char errM3CT[] = "ERR|M3CT|";
printf("Server Sent: %s\n", errM3CT);
write(c->fd, errM3CT, strlen(errM3CT));
}
else if((msgNumber==5) && (type=='f')){
char errM5FT[] = "ERR|M5FT|";
printf("Server Sent: %s\n", errM5FT);
write(c->fd, errM5FT, strlen(errM5FT));
}
else if((msgNumber==5) && (type=='l')){
char errM5LN[] = "ERR|M5LN|";
printf("Server Sent: %s\n", errM5LN);
write(c->fd, errM5LN, strlen(errM5LN));
}
else if((msgNumber==5) && (type=='c')){
char errM5CT[] = "ERR|M5CT|";
printf("Server Sent: %s\n", errM5CT);
write(c->fd, errM5CT, strlen(errM5CT));
}
else{
return;
}

return;
}

/**Function reads socket message from ConnectionThreadArg struct c into char buffer (buf) byte by byte
 *checks for errors
 *If an error is found, an appropriate error message is sent and -1 is returned
 *If no errors found, function returns number of bytes read into buffer
 */
int readAndErrorCheck2(struct ConnectionThreadArg *c, char *buf){

int bytesRead = 0;
int verticalBarCount = 0;
int i = 0;

//read byte by byte into the buffer 
//until the first vertical bar, or the 5th char, whichever comes first
//as long as all preceding chars match what is allowed
while(verticalBarCount<1){
bytesRead+=read(c->fd, &buf[i], 1);
//printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
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

	char lengthBuf[9000];
	int lengthBufEnd = 0;
	//Continue reading byte by byte until the second vertical bar is sent
	//Or reached a non numerical character, whichever is first
	while(verticalBarCount<2){
	bytesRead += read(c->fd, &buf[i], 1);
	//printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
	if(buf[i]=='|'){
	verticalBarCount++;
	}
	//send formatting error if reached a non digit before second vertical bar
	if((isdigit(buf[i])==0) && (buf[i]!='|')){
	buf[bytesRead]='\0';
	printf("Server Received: %s", buf);
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
	//printf("Current Buffer: %s\tLength Buffer:%c:%c\n", buf, lengthBuf[0], lengthBuf[lengthBufEnd-2]);
	buf[bufSize-1]='\0';
	//printf("Current Buffer: %s\n", buf);
	//printf("Length Buf[0]: %c\n", lengthBuf[0]);	
	int length = atoi(lengthBuf);
	int endIndex = length + i + 1;
	//printf("Length: %d\n", length);

	//Take care of case where atoi returns zero
	if(buf[i-1]=='|' && buf[i-2]=='|'){
	buf[bytesRead]='\0';
	printf("Server Received: %s\n", buf);
	sendErrorMsg(c, 'f');
	return -1;
	}


	//read rest of message char by char
	//until vertical bar or length reached, whichever is first
	while(i<endIndex){
	bytesRead+=read(c->fd, &buf[i], 1);
	//printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
	if(buf[i]=='|'){
	verticalBarCount++;
	break;
	}
	i++;	
	}

	//Send error msg if incorrect number of vertical bars
	if(verticalBarCount!=3){
	buf[bytesRead]='\0';
	printf("Server Received: %s\n", buf);
	sendErrorMsg(c, 'l');
	return -1;
	}
	else if(i!=endIndex-1){
	buf[bytesRead]='\0';
	printf("Server Received: %s\n", buf);
	sendErrorMsg(c, 'l');
	return -1;
	}
	else{
	buf[bytesRead]='\0';	
	printf("Server Received: %s\n", buf);
	return bytesRead;
	}


	}
//if the server has received an error message, check to see if the error message is correctly formatted
//if so, close the connection
//if not, send appropriate error message and then close the connection
else if(strncmp(buf, "ERR|", 4)==0){
//if server has recieved an error message, close connection
int errMsgLength = 0;

	while(errMsgLength<5){
	bytesRead += read(c->fd, &buf[i], 1);
	//printf("read %d bytes, char: %c\n", bytesRead, buf[i]);
	
	//errMsgBuf[errMsgBufEnd]=buf[i];
	if(buf[i]=='|'){
	verticalBarCount++;
	break;
	}
	
	errMsgLength++;
	i++;
	}

	//If the error message received is not the correct length (9)
	//Or does not include two vertical bars in the correct places
	//send format error, and then close connection
	if(verticalBarCount!=2 || errMsgLength!=4){
	buf[bytesRead]='\0';	
	printf("Server Received: %s\n", buf);
	sendErrorMsg(c, 'f');
	return -1;
	}



buf[bytesRead]='\0';	
printf("Server Received: %s\n", buf);
return bytesRead;
}
else{
//error message, incorect format
buf[bytesRead]='\0';	
printf("Server Received: %s\n", buf);
sendErrorMsg(c, 'f');
return -1;
}



buf[bytesRead]='\0';	
printf("Server Received: %s\n", buf);
return bytesRead;
}


/**Thread Function takes a ConnectionThreadArg pointer as an argument
 * and interacts through the connection parameter according to KKJ Protocol
 */
void *KKJProtocol(void *arg)
{
    char hostName[100]; 
    char portNumber[10];
    char *buf = malloc(bufSize);
    struct ConnectionThreadArg *cta = (struct ConnectionThreadArg *) arg;
    msgNumber = 0;

   // find out the name and port of the remote host
   int error = getnameinfo((struct sockaddr *) &cta->addr, cta->addr_len, hostName, 100, portNumber, 10, NI_NUMERICSERV);
    
   if (error != 0) {
        close(cta->fd);
        return NULL;
    }

    //connected to client
    printf("[%s:%s]New Connection.\n", hostName, portNumber);

    //Hardcoded strings for final content comparison
    char resp[] = "REG|13|Knock, knock.|";
    char resp2[] = "REG|4|Boo.|";
    char resp3[] = "REG|14|Awe don't cry.|";
   char recArg[] = "REG|12|Who's there?|";
   char recArg2[] = "REG|9|Boo, who?|";

    //initial Knock, Knock
    write(cta->fd, resp, sizeof(resp));
    printf("Server Sent: %s\n", resp);
    msgNumber++;

    int bytesRead = 0;    
    while ((bytesRead = readAndErrorCheck2(cta, buf)) > 0) {
        buf[bytesRead] = '\0';
       // printf("read %d bytes |%s|\n", bytesRead, buf);
	
       //If control is here, the message has passed all length and format checks
       //perform the final content checks

	//Who's there received
	if(strcmp(buf, recArg)==0 && msgNumber==1){
	 write(cta->fd, resp2, sizeof(resp2)); 
   	 printf("Server Sent: %s\n", resp2);
	 msgNumber+=2;
	}//Boo, who? received
	else if(strcmp(buf, recArg2)==0 && msgNumber==3){
	 write(cta->fd, resp3, sizeof(resp3));
	 printf("Server Sent: %s\n", resp3);
	 msgNumber+=2;
	}//msg of disgust recieved	
	else if((buf[bytesRead-2]=='.' || buf[bytesRead-2]=='!' ||
		 buf[bytesRead-2]=='?') && msgNumber==5){
	break;
	}
	//if a correctly formatted/numbered error message was received,
	//break and close connection
	else if((strncmp(buf, "ERR|M0FT|", 9)==0) && (msgNumber==1)){
	break;
	}
	else if((strncmp(buf, "ERR|M0LN|", 9)==0) && (msgNumber==1)){
	break;
	}
	else if((strncmp(buf, "ERR|M0CT|", 9)==0) && (msgNumber==1)){
	break;
	}
	else if((strncmp(buf, "ERR|M2FT|", 9)==0) && (msgNumber==3)){
	break;
	}
	else if((strncmp(buf, "ERR|M2LN|", 9)==0) && (msgNumber==3)){
	break;
	}
	else if((strncmp(buf, "ERR|M2CT|", 9)==0) && (msgNumber==3)){
	break;
	}
	else if((strncmp(buf, "ERR|M4FT|", 9)==0) && (msgNumber==5)){
	break;
	}
	else if((strncmp(buf, "ERR|M4LN|", 9)==0) && (msgNumber==5)){
	break;
	}
	else if((strncmp(buf, "ERR|M4CT|", 9)==0) && (msgNumber==5)){
	break;
	}
	else{//content is incorrect, whether it be a REG or an ERR msg
	//Send error message, then break and close connection
	sendErrorMsg(cta, 'c');
       	break;	
	}	
	
    }

   printf("[%s:%s]Connection ended.\n", hostName, portNumber);
   
   //close the connection
    close(cta->fd);
    free(cta);
    return NULL;
}


/**Function takes a string as a port number and creates a listening socket
 * If a client has connected, a new thread is created to handle them
 *
 */
int setupServer(char *portNumber)
{
    //init address info hints
    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    //set address info for port number and listening socket
    struct addrinfo *address_list;
    int error = getaddrinfo(NULL, portNumber, &hint, &address_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    //interate through list of addresses to try to create a listening socket
    struct addrinfo *addressinfo = address_list;
    int socketFD;
    while(addressinfo!=NULL){	
    socketFD = socket(addressinfo->ai_family, addressinfo->ai_socktype, addressinfo->ai_protocol);
        
        if(socketFD == -1){
        addressinfo=addressinfo->ai_next;    
	continue;
        }

        //If socket is successfully created, bind it to the specified port and listen for connections
	if ((bind(socketFD, addressinfo->ai_addr, addressinfo->ai_addrlen) == 0) &&
            (listen(socketFD, 5) == 0)) {
            break;
        }
	else{ //socket bind or listen failed, close socket and try another in next iteration 
        close(socketFD);
	}

        //close(socketFD);
        addressinfo=addressinfo->ai_next;
       }

    //case where no sockets successfully bound/listen
    if (addressinfo == NULL) {
        fprintf(stderr, "Could not bind.\n");
	freeaddrinfo(address_list);
        return -1;
    }

    freeaddrinfo(address_list);

    printf("Server waiting for connection...\n");
    while(1) {
	//create ConnectionThreadArg struct
        struct ConnectionThreadArg *cta = malloc(sizeof(struct ConnectionThreadArg));
        cta->addr_len = sizeof(struct sockaddr_storage);
        
        // block and wait for an incoming connection to accept
	// Once a connection is accepted, get socket for connection
        cta->fd = accept(socketFD, (struct sockaddr *) &cta->addr, &cta->addr_len);
        
        if (cta->fd == -1) {
            continue;
        }

	//create thread to deal with connection
        pthread_t threadID;
	error = pthread_create(&threadID, NULL, KKJProtocol, cta);

	// handle error in thread creation
        if (error != 0) {
	    close(cta->fd);
            free(cta);
            continue;
        }

	//detach newly created thread while main thread waits to accept more connections
        pthread_detach(threadID);
    }

    return 0;
}



int main(int argc, char **argv)
{
if(argc!=2){
return -1;
}

int portNum = atoi(argv[1]);
if (portNum==0 || portNum<=5000 || portNum>=65536){
printf("Must provide a valid port number. Must be in exclusive range: 5000-65536\n");
return -1;
}

(void)setupServer(argv[1]);
return 0;
}



