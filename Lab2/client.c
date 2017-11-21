/**
  * Kevin Gilboy and Nick Petro
  *	ECE 1150 - Intro to Networks
  *	Lab 1
 
  ***   PLEASE NOTE: All multi-line comments before code sections were
  ***	written to help the developers with organizing the program
  ***	before implementing. They were not meant in any way to insult
  ***	the intelligence of any readers of this program :^)

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 80
#define URL "127.0.0.1"

#define TAG_CONTENT_LENGTH "Content-Length: "
#define TAG_LINE_AFTER_CONTENT_LENGTH "ETag: "
#define TAG_LAST_HEADER_LINE "Connection: keep-alive\r\n\r\n"

int main(int argc, char *argv[]){
	int success;
	/*
	 * Check input args for proper zipcode
	 */
	if(argc!=1){
		printf("Invalid. Usage: %s <text>\n",argv[0]);
		exit(-1);
	}

	sendToServer(argv[1]);

	/*
	 * Get and Display the weather
	 */
	return 1;
}

int sendToServer(char *msg){
	int success = 0;
	int recv_bytes = 0;
	int content_length = 0;
	int header_length = 0;
	int peek_size = 800;
	char peek_message[peek_size];
	char get_command[100];
	struct sockaddr_in server_addr;
	char *curr_ptr;
	
	/*
	 * Open a socket and connect to the addr
	 */
	int client_socket = socket(AF_INET,
		SOCK_STREAM,0);
	if(client_socket<0) {
		printf("Socket error\n");
		return -1;
	}

	memset(&server_addr, '0', sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, URL, &server_addr.sin_addr) <= 0) {
		fprintf(stderr,"Invalid addr\n");
		return -1;
	}

	int connection_status = connect(client_socket, (struct sockaddr *)&server_addr,
		sizeof(server_addr));
	if(connection_status<0){
		fprintf(stderr,"Connection failed\n");
		return -1;
	}
	
	/*
	 * Build a GET request and send it
	 */
	strcpy(get_command,"GET /demo/fullnight/on HTTP/1.1\r\nHost: "URL"\r\n\r\n");
	send(client_socket,get_command,strlen(get_command),0);
	
	/*
	 * Peek at response to get content length and size of header
	 */
	curr_ptr = peek_message;
	do{
		recv_bytes = recv(client_socket, curr_ptr, sizeof(peek_message)-1,MSG_PEEK);
		if(recv_bytes<=0){
			fprintf(stderr,"No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-peek_message)/sizeof(char)<peek_size-1);
	peek_message[recv_bytes] = 0;

	/*
	 * Get content length
	 */
	char *content_length_ptr = strstr(peek_message,TAG_CONTENT_LENGTH)+strlen(TAG_CONTENT_LENGTH);
	
	char *content_length_end = strstr(peek_message,TAG_LINE_AFTER_CONTENT_LENGTH) -
		(2*sizeof(char)); //Rm 2 chars since the previous line ends in \r\n
	
	while(content_length_ptr<content_length_end){
		content_length = (content_length*10) + (content_length_ptr[0]-'0');
		content_length_ptr+=sizeof(char);
	}

	/*
	 * Get size of header
	 */
	char *response_start = strstr(peek_message,TAG_LAST_HEADER_LINE)+strlen(TAG_LAST_HEADER_LINE);
	header_length = (response_start-peek_message) / sizeof(char);

	/*
	 * Recv header and discard it
	 */
	char *header = malloc(sizeof(char) * (header_length+1));
	curr_ptr = header;
	do{
		recv_bytes = recv(client_socket, curr_ptr, header_length,0);
		if(recv_bytes<=0){
			printf("No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-header)/sizeof(char)<header_length-1);
	header[recv_bytes] = 0;
	free(header);
	
	/*
	 * Recv content
	 */
	char *content = malloc(sizeof(char) * (content_length+1));
	curr_ptr = content;
	// We must keep recv'ing if the recv gives less bytes than we ask for
	do{
		recv_bytes = recv(client_socket, curr_ptr, content_length,0);
		if(recv_bytes<=0){
			printf("No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-content)/sizeof(char)<content_length-1);
	content[content_length] = 0;

	fprintf(stderr,"%s",content);

	close(client_socket);

	free(content);
	return success;
}


int printLine(){
	printf("-----------------------------------------------\n");
	return 0;
}
