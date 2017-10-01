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
	char server_message[800];
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
	char *next_number_character = strstr(server_message,str1)+((strlen(str1))*sizeof(char));
	
	char *str2 = "Connection:";
	char *end_number = strstr(server_message,str2)-(2*sizeof(char)); //Remove two chars since the previous line ends in \r\n
	
	//Create the number by adding each digit, one at a time
	int response_length=0;
	while(next_number_character<end_number){
		response_length = (response_length*10)+(next_number_character[0]-'0');
		next_number_character+=sizeof(char);
	}
	printf("%d\n",response_length);
	
	char *str3 = "<response>";
	char *response_start = strstr(server_message,str3);
	
	close(client_socket);
	return 0;
}
