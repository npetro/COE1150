// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#define PORT 8080
int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *holdBuffer;
      
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
	while(1) {
	    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
	        perror("accept");
	        exit(EXIT_FAILURE);
	    }
	    valread = read(new_socket , buffer, 1024);
	    printf("%s\n",buffer);
		//call functions here
		strcat(holdBuffer,"Content-Length:13\r\nConnection: keep-alive\r\n");
		strcat(holdBuffer,buffer);
		strcat(holdBuffer, "\r\n\r\n");
	    send(new_socket , holdBuffer , strlen(holdBuffer) , 0 );
	    printf("Message sent\n");
	    //return 0;
		close(new_socket);
		sleep(1);
	}
}

char* encode(char *modBuffer) {
	for (int i = 0; i < strlen(modBuffer); i = i + 1 ) {
		if (tolower(modBuffer[i]) != 'z'){
			modBuffer[i] = tolower(modBuffer[i]) + 1;
		} else if (tolower(modBuffer[i]) == 'z') {
			modBuffer[i] = 'A';
		} else {
			modBuffer[i] = '?';
		}	
 	}
	return modBuffer;
}

char* decode(char *modBuffer) {
	for (int i = 0; i < strlen(modBuffer); i = i + 1 ) {
		if (modBuffer[i] == 'A'){
			modBuffer[i] = 'z';
		} else if (tolower(modBuffer[i]) != 'a') {
			modBuffer[i] = tolower(modBuffer[i]) - 1;
		} else {
			modBuffer[i] = '?';
		}	
 	}
	return modBuffer;
}