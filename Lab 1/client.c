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

/*
	[ ] User input - input zip code
	[ ] User input - check number args == 2 && 5 digits, else display help man
	[ ] Group into two functions that main calls - parse and display
	[ ] Rename byte_count
	[ ] Rename str1 and str2
	[ ] Use while loops for every recv
	[ ] Comments
	[ ] Any addition error handling?
	[ ] Can we recover? lols
*/

int main(int argc, char *argv[]){
	if(argc!=2){
		printf("Invalid. Usage: ./%s <us_zipcode>",argv[0]);
		exit(-1);
	}
	int zipcode = atoi(argv[1]);
	if(zipcode<=0 || zipcode>99999){
		printf("The entered zipcode (%s) is invalid.",argv[1]);
		exit(-1);
	}

	char peek_message[800];
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
		return -1;
	}
	
	char *get_command = "GET /api/"API_KEY"/conditions/q/CA/San_Francisco.xml HTTP/1.1\r\nHost: "URL"\r\n\r\n";
	send(client_socket,get_command,strlen(get_command),0);
	
	int byte_count = recv(client_socket, peek_message, sizeof(peek_message)-1,MSG_PEEK);
	peek_message[byte_count] = 0;

	/*
	 * Get content length
	 */
	char *str1 = "Content-Length: ";
	char *next_number_character = strstr(peek_message,str1)+((strlen(str1))*sizeof(char));
	
	char *str2 = "Connection:";
	char *end_number = strstr(peek_message,str2)-(2*sizeof(char)); //Remove two chars since the previous line ends in \r\n
	
	//Create the number by adding each digit, one at a time
	int content_length=0;
	while(next_number_character<end_number){
		content_length = (content_length*10)+(next_number_character[0]-'0');
		next_number_character+=sizeof(char);
	}
	
	/*
	 * Get size of header
	 */
	char *str3 = "<response>";
	char *response_start = strstr(peek_message,str3);
	int header_length = (response_start-peek_message) / sizeof(char);

	/*
	 * Recv header
	 */
	char *header = malloc(sizeof(char) * (header_length+1));
	int byte_count2 = recv(client_socket, header, header_length,0);
	header[byte_count2] = 0;
	free(header);
	
	/*
	 * Recv content
	 */
	char *content = malloc(sizeof(char) * (content_length+1));
	char *curr_content_ptr = content;
	int byte_count3;
	do{
		byte_count3 = recv(client_socket, curr_content_ptr, content_length,0);
		curr_content_ptr+=byte_count3*sizeof(char);
	}while((curr_content_ptr-content)/sizeof(char)<content_length-1);

	content[content_length] = 0;
	printf("-----------------------------------------------\n");
	
	char *time_start_str = "<observation_time>";
	char *time_start = strstr(content,time_start_str)+((strlen(time_start_str))*sizeof(char));
	char *time_end_str = "</observation_time>";
	char *time_end = strstr(content,time_end_str); //Remove two chars since the previous line ends in \r\n
	printf("%.*s\n",(time_end-time_start)/sizeof(char),time_start);

	char *full_start_str = "<full>";
	char *full_start = strstr(content,full_start_str)+((strlen(full_start_str))*sizeof(char));
	char *full_end_str = "</full>";
	char *full_end = strstr(content,full_end_str); //Remove two chars since the previous line ends in \r\n
	printf("%.*s ",(full_end-full_start)/sizeof(char),full_start);

	char *zip_start_str = "<zip>";
	char *zip_start = strstr(content,zip_start_str)+((strlen(zip_start_str))*sizeof(char));
	char *zip_end_str = "</zip>";
	char *zip_end = strstr(content,zip_end_str); //Remove two chars since the previous line ends in \r\n
	printf("%.*s \n",(zip_end-zip_start)/sizeof(char),zip_start);
	
	char *weather_start_str = "<weather>";
	char *weather_start = strstr(content,weather_start_str)+((strlen(weather_start_str))*sizeof(char));
	char *weather_end_str = "</weather>";
	char *weather_end = strstr(content,weather_end_str); //Remove two chars since the previous line ends in \r\n
	printf("> Weather: %.*s\n",(weather_end-weather_start)/sizeof(char),weather_start);

	char *temp_start_str = "<temperature_string>";
	char *temp_start = strstr(content,temp_start_str)+((strlen(temp_start_str))*sizeof(char));
	char *temp_end_str = "</temperature_string>";
	char *temp_end = strstr(content,temp_end_str); //Remove two chars since the previous line ends in \r\n
	printf("> Temperature: %.*s\n",(temp_end-temp_start)/sizeof(char),temp_start);

	char *wind_start_str = "<wind_string>";
	char *wind_start = strstr(content,wind_start_str)+((strlen(wind_start_str))*sizeof(char));
	char *wind_end_str = "</wind_string>";
	char *wind_end = strstr(content,wind_end_str); //Remove two chars since the previous line ends in \r\n
	printf("> Winds: %.*s\n",(wind_end-wind_start)/sizeof(char),wind_start);
	
	printf("-----------------------------------------------\n");

	printf("\n");
	free(content);

	close(client_socket);
	return 0;
}
