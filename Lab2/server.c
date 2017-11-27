// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#define PORT 8085

/*
 * Constants needed for content and content length extraction
 */
#define TAG_CONTENT_LENGTH_START "Content-Length: "
#define TAG_CONTENT_LENGTH_END "Connection: "
#define TAG_LAST_HEADER_LINE "Connection: keep-alive\r\n\r\n"

void encode(char*, int);
void decode(char*, int);
int handler(void*);

int main(int argc, char const *argv[])
{
    int server_fd, server_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
     
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	pthread_t thread_id;

	while(server_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)){
		if(pthread_create(&thread_id, NULL, *handler, (void*) &server_socket)<0){
			fprintf(stderr,"Thread cration failed");
			return 1;
		}

		pthread_join(thread_id,NULL);

	}
	close(server_socket);
	fprintf(stderr,"out");
}

int handler(void *server_fd) {
	int server_socket = *(int*)server_fd;
	int recv_bytes = 0;
	int peek_size = 82;
    char peek_message[peek_size];
	char *curr_ptr;
	int content_length = 0;
	int header_length = 0;
		/*
		 * Peek at response to get content length and size of header
		 */
		curr_ptr = peek_message;
		do{
			recv_bytes = recv(server_socket, curr_ptr, sizeof(peek_message)-1,MSG_PEEK);
			if(recv_bytes<=0){
				fprintf(stderr,"No message received from client\n");
				return -1;
			}
			curr_ptr+=recv_bytes*sizeof(char);
		}while((curr_ptr-peek_message)/sizeof(char)<peek_size-1);
		peek_message[recv_bytes] = 0;

		/*
		 * Get content length
		 */
		char *content_length_ptr = strstr(peek_message,TAG_CONTENT_LENGTH_START)+strlen(TAG_CONTENT_LENGTH_START);
		
		char *content_length_end = strstr(peek_message,TAG_CONTENT_LENGTH_END) -
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
			recv_bytes = recv(server_socket, curr_ptr, header_length,0);
			if(recv_bytes<=0){
				printf("No message received from client\n");
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
			recv_bytes = recv(server_socket, curr_ptr, content_length,0);
			if(recv_bytes<=0){
				printf("No message received from client\n");
				return -1;
			}
			curr_ptr+=recv_bytes*sizeof(char);
		}while((curr_ptr-content)/sizeof(char)<content_length-1);
		content[content_length] = 0;

		if (tolower(content[0]) == 'd') {
			decode(content,content_length);
		} else if (tolower(content[0]) == 'e') {
			encode(content,content_length);
		} else {
			printf("Invalid Message\n");
		}
		fprintf(stderr,"%s\n",content);
		char msg[content_length+85];
		sprintf(msg,"Content-Length: %d\r\nConnection: keep-alive\r\n\r\n%s\r\n",content_length-2,content+2*sizeof(char));
	    send(server_socket , msg, strlen(msg) , 0 );
	    printf("Message sent\n");

}

void encode(char* content, int content_length) {
	int i;
	
	for (i = 2; i < content_length; i++ ) {
		if (tolower(content[i]) == 'z') {
			content[i] = 'a';
		} else if (tolower(content[i]) < 'z' && tolower(content[i])>='a') {
			content[i] = tolower(content[i]) + 1;
		} else if(content[i]!=' ') {
			content[i] = '?';
		}	
 	}
}

void decode(char* content, int content_length) {
	char *modBuffer = content;
	int i;
	for (i = 2; i < content_length; i++ ) {
		if (content[i] == 'a'){
			content[i] = 'z';
		} else if (tolower(modBuffer[i]) > 'a' && tolower(modBuffer[i])<='z') {
			content[i] = tolower(modBuffer[i]) - 1;
		} else if(content[i]!=' ') {
			content[i] = '?';
		}	
 	}
}
