#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT "80"
#define API_KEY "cf497a3ca5237e42"
#define URL "api.wunderground.com"

int main(){
	char server_message[450];
    struct addrinfo hints;
	struct addrinfo *result;
	
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	
	//
	int server_status = getaddrinfo(URL, PORT, &hints, &result);
	int client_socket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	int connection_status = connect(client_socket, result->ai_addr, result->ai_addrlen);

	if(connection_status<0){
		printf("Connection failed\n");
	}
	
	char *header = "GET /api/"API_KEY"/conditions/q/CA/San_Francisco.xml HTTP/1.1\r\nHost: "URL"\r\n\r\n";
	printf("Sent:\n\n%s",header);
	send(client_socket,header,strlen(header),0);
	
	int byte_count = recv(client_socket, server_message, sizeof(server_message)-1,0);
	server_message[byte_count] = 0;
	printf("Recieved:\n\n%s\n",server_message);
	
	char *str1 = "Content-Length: ";
	char *result1 = strstr(server_message, str1);
	int position1 = result1 - str1;
	printf("Position 1: %d\n", position1);
	int substringLength1 = strlen(str1) - position1;
	
	char *str2 = "Connection:";
	char *result2 = strstr(server_message, str2);
	int position2 = result2 - str2;
	printf("Position 2: %d\n", position2);
	int substringLength2 = strlen(str2) - position2;
	
	char hold[6];
	strncpy(hold, result1+((16*sizeof(char))), 5);
	hold[5]='\0';
	printf("%c%c%c%c",hold[0],hold[1],hold[2],hold[3]);
	
	close(client_socket);
	return 0;
}
